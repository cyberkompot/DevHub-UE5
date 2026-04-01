// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"
#include "GameFramework/PlayerController.h"

struct FDevInputModeState final : FInputModeDataBase
{
	explicit FDevInputModeState(APlayerController& InPlayerController);

	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<UPlayer> Player;

	// PlayerController.
#if UE_COMPATIBILITY_SUPPORTED_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
	FString CapturedDebugDisplayName;
#endif // UE_COMPATIBILITY_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
#if UE_COMPATIBILITY_SUPPORTED_PLAYER_CONTROLLER_SHOULD_FLUSH_KEYS_WHEN_VIEWPORT_FOCUS_CHANGES
	bool bShouldFlushInputWhenViewportFocusChanges; // TODO: Fix implementation or remove.
#endif // UE_COMPATIBILITY_PLAYER_CONTROLLER_SHOULD_FLUSH_KEYS_WHEN_VIEWPORT_FOCUS_CHANGES
	bool bShowMouseCursor;
	bool bEnableClickEvents;
	bool bEnableTouchEvents;
	bool bEnableMouseOverEvents;
	bool bEnableTouchOverEvents;

	// GameViewportClient.
	bool bIgnoreInput;
	TWeakPtr<SWidget> MouseCaptor;
	EMouseLockMode MouseLockMode;

	// SlateOperations.
	bool bUseHighPrecisionMouse;
	EMouseCaptureMode MouseCaptureMode;
	TWeakPtr<SWidget> FocusRecipient;

	//~ Begin FInputModeDataBase interface.
	virtual void ApplyInputMode(FReply& SlateOperations, UGameViewportClient& GameViewportClient) const override;

#if UE_COMPATIBILITY_SUPPORTED_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
	virtual const FString& GetDebugDisplayName() const override { return CapturedDebugDisplayName; }
#endif // UE_COMPATIBILITY_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME
	//~ End FInputModeDataBase interface.
};

class FDevInputMode final
{
public:
	void SetInputMode(const FInputModeDataBase& InData);
	void RestoreInputMode();

	void SetGameInstance(UGameInstance* InGameInstance);
	void Dispose();

private:
	TOptional<FDevInputModeState> CapturedState;
	UGameInstance* GameInstance = nullptr;

	UGameViewportClient* GetGameViewport() const;
	ULocalPlayer* GetLocalPlayer() const;
	UPlayer* GetPlayer() const;
	APlayerController* GetPlayerController() const;
};
