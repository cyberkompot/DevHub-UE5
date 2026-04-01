// Copyright (c) Alexandr Pereverzev.

#include "DevInputMode.h"

#include "DevCore.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SViewport.h"

FDevInputModeState::FDevInputModeState(APlayerController& InPlayerController)
	: PlayerController(&InPlayerController),
	  Player(InPlayerController.Player),
#if UE_COMPATIBILITY_SUPPORTED_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
	  CapturedDebugDisplayName(InPlayerController.GetCurrentInputModeDebugString()),
#endif // UE_COMPATIBILITY_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
#if UE_COMPATIBILITY_SUPPORTED_PLAYER_CONTROLLER_SHOULD_FLUSH_KEYS_WHEN_VIEWPORT_FOCUS_CHANGES
	  bShouldFlushInputWhenViewportFocusChanges(InPlayerController.ShouldFlushKeysWhenViewportFocusChanges()),
#endif // UE_COMPATIBILITY_PLAYER_CONTROLLER_SHOULD_FLUSH_KEYS_WHEN_VIEWPORT_FOCUS_CHANGES
	  bShowMouseCursor(InPlayerController.bShowMouseCursor),
	  bEnableClickEvents(InPlayerController.bEnableClickEvents),
	  bEnableTouchEvents(InPlayerController.bEnableTouchEvents),
	  bEnableMouseOverEvents(InPlayerController.bEnableMouseOverEvents),
	  bEnableTouchOverEvents(InPlayerController.bEnableTouchOverEvents)
{
	// GameViewportClient.
	if (const UWorld* World = InPlayerController.GetWorld())
	{
		if (UGameViewportClient* GameViewportClient = World->GetGameViewport())
		{
			bIgnoreInput = GameViewportClient->IgnoreInput();
#if UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_GET_MOUSE_LOCK_MODE
			MouseLockMode = GameViewportClient->GetMouseLockMode();
#else
			MouseLockMode = (GameViewportClient->LockDuringCapture())
				? (GameViewportClient->ShouldAlwaysLockMouse())
					? (GameViewportClient->IsExclusiveFullscreenViewport())
						? EMouseLockMode::LockInFullscreen
						: EMouseLockMode::LockAlways
					: EMouseLockMode::LockOnCapture
				: EMouseLockMode::DoNotLock;
#endif
			MouseCaptureMode = GameViewportClient->GetMouseCaptureMode();
		}
	}

	// SlateOperations.
	if (FSlateApplication::IsInitialized())
	{
		bUseHighPrecisionMouse = FSlateApplication::Get().IsUsingHighPrecisionMouseMovment();
	}
	if (const ULocalPlayer* LocalPlayer = InPlayerController.GetLocalPlayer())
	{
		if (const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
		{
			MouseCaptor = SlateUser->GetCursorCaptor();
			FocusRecipient = SlateUser->GetFocusedWidget();
		}
	}
}

void FDevInputModeState::ApplyInputMode(FReply& SlateOperations, UGameViewportClient& GameViewportClient) const
{
	// PlayerController.
	if (APlayerController* Controller = PlayerController.Get())
	{
		Controller->bShowMouseCursor = bShowMouseCursor;
		Controller->bEnableClickEvents = bEnableClickEvents;
		Controller->bEnableTouchEvents = bEnableTouchEvents;
		Controller->bEnableMouseOverEvents = bEnableMouseOverEvents;
		Controller->bEnableTouchOverEvents = bEnableTouchOverEvents;
	}

	// GameViewportClient.
	{
		GameViewportClient.SetMouseLockMode(MouseLockMode);
		GameViewportClient.SetIgnoreInput(bIgnoreInput);
		GameViewportClient.SetMouseCaptureMode(MouseCaptureMode);
	}

	// SlateOperations.
	if (const TSharedPtr<SWidget> MouseCaptorWidget = MouseCaptor.Pin())
	{
		if (bUseHighPrecisionMouse)
		{
			SlateOperations.UseHighPrecisionMouseMovement(MouseCaptorWidget.ToSharedRef());
		}
		else
		{
			SlateOperations.CaptureMouse(MouseCaptorWidget.ToSharedRef());
		}
		SlateOperations.LockMouseToWidget(MouseCaptorWidget.ToSharedRef());
	}
	else
	{
		SlateOperations.ReleaseMouseCapture();
		SlateOperations.ReleaseMouseLock();
	}
	if (const TSharedPtr<SWidget> FocusRecipientWidget = FocusRecipient.Pin())
	{
		SlateOperations.SetUserFocus(FocusRecipientWidget.ToSharedRef(), EFocusCause::SetDirectly);
	}
	else
	{
		SlateOperations.ClearUserFocus();
	}
}

void FDevInputMode::SetInputMode(const FInputModeDataBase& InData)
{
	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController) { return; }

	if (CapturedState && CapturedState->PlayerController != PlayerController)
	{
		// Input mode was previously set for another PlayerController – restoring it and applying the input mode to new one.
		RestoreInputMode();
	}

	if (!CapturedState)
	{
		CapturedState.Emplace(*PlayerController);
	}

	PlayerController->SetInputMode(InData);
}

void FDevInputMode::RestoreInputMode()
{
	if (!CapturedState) { return; }

	// Always restore input for the original PlayerController, even if it has changed in the meantime.
	if (APlayerController* PlayerController = CapturedState->PlayerController.Get(); PlayerController)
	{
		if (PlayerController->Player)
		{
			PlayerController->SetInputMode(*CapturedState);
		}
		else if (const TObjectPtr<UPlayer> Player = CapturedState->Player.Get())
		{
			// There may be multiple PlayerControllers, for example when ADebugCameraController is active (see UCheatManager::ToggleDebugCamera()).
			PlayerController->Player = Player;
			ON_SCOPE_EXIT { if (PlayerController->Player == Player) { PlayerController->Player = nullptr; } };
			PlayerController->SetInputMode(*CapturedState);
		}
	}
	else
	{
		// Fallback to applying the input mode directly if the PlayerController has already expired.
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UGameViewportClient* GameViewportClient = GetGameViewport())
			{
				CapturedState->ApplyInputMode(LocalPlayer->GetSlateOperations(), *GameViewportClient);
			}
		}
	}

	CapturedState.Reset();
}

void FDevInputMode::SetGameInstance(UGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;
}

void FDevInputMode::Dispose()
{
	RestoreInputMode();
	GameInstance = nullptr;
}

UGameViewportClient* FDevInputMode::GetGameViewport() const
{
	if (!GameInstance) { return nullptr; }

	const UWorld* World = GameInstance->GetWorld();
	if (!World) { return nullptr; }

	return  World->GetGameViewport();
}

ULocalPlayer* FDevInputMode::GetLocalPlayer() const
{
	return Cast<ULocalPlayer>(GetPlayer());
}

UPlayer* FDevInputMode::GetPlayer() const
{
	APlayerController* PlayerController = GetPlayerController();
	return (PlayerController) ? PlayerController->Player : nullptr;
}

APlayerController* FDevInputMode::GetPlayerController() const
{
	if (!GameInstance) { return nullptr; }

	const UWorld* World = GameInstance->GetWorld();
	if (!World) { return nullptr; }

	return UGameplayStatics::GetPlayerController(World, 0);
}
