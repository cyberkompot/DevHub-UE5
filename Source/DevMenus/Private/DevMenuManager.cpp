// Copyright (c) Alexandr Pereverzev.

#include "DevMenuManager.h"

#include "DevInputs.h"
#include "DevMenuWidgetBuilder.h"
#include "DevMenuLogging.h"
#include "DevMenuQueries.h"
#include "DevMenuRegistry.h"
#include "Components/Viewport.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/IConsoleManager.h"
#include "Widgets/SDevMenuConsoleResponseWidget.h"
#include "Widgets/SDevMenuGameLayerWidget.h"
#include "Widgets/SDevMenuMainMenuWidget.h"

using namespace DevMenu::Logging;

static const FLazyName LogCheatManagerCategoryName = "LogCheatManager";

void UDevMenuManager::Initialize()
{
	UGameViewportClient::OnViewportCreated().AddUObject(this, &UDevMenuManager::OnViewportCreated);
	UConsole::OnConsoleActivationStateChanged.AddUObject(this, &UDevMenuManager::OnConsoleActivationStateChanged);
	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UDevMenuManager::OnWorldBeginTearDown);

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnFocusChanging().AddUObject(this, &UDevMenuManager::OnFocusChanging);
	}

	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->OnInputTypeChanged().AddUObject(this, &ThisClass::OnInputTypeChanged);
	}

	if (GetGameViewport())
	{
		OnViewportCreated();
	}
}

void UDevMenuManager::Dispose()
{
	EmptyMenus();

	UGameViewportClient::OnViewportCreated().RemoveAll(this);
	UConsole::OnConsoleActivationStateChanged.RemoveAll(this);
	FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnFocusChanging().RemoveAll(this);
	}

	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->OnInputTypeChanged().RemoveAll(this);
	}

	if (const UGameViewportClient* GameViewport = GetGameViewport())
	{
		OnViewportCloseRequested(GameViewport->Viewport);
	}
}

void UDevMenuManager::BeginDestroy()
{
	Dispose();
	Super::BeginDestroy();
}

void UDevMenuManager::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
#if !NO_LOGGING
	if (Category == LogConsoleResponse.GetCategoryName()
		|| Category == LogCheatManagerCategoryName)
	{
		if (SDevMenuConsoleResponseWidget* Widget = MenuConsoleResponseWidget.Get())
		{
			Widget->PushConsoleResponse(Verbosity, FText::FromStringView(V));
		}
	}
#endif // !NO_LOGGING
}

void UDevMenuManager::ShowMenu(const FName InMenuPath)
{
	if (const FDisplayedMenuDetails* MenuDetails = FindMenu(InMenuPath))
	{
		PushMenuToTop(*MenuDetails);
	}
	else
	{
		const TSharedRef<SWidget> NewMenuWidget = MenuWidgetBuilder->MakeMainMenuWidget(InMenuPath);
		AddMenu(FDisplayedMenuDetails(InMenuPath, NewMenuWidget));
	}
}

void UDevMenuManager::ToggleMenu(const FName InMenuPath)
{
	if (FindMenu(InMenuPath))
	{
		HideMenu(InMenuPath);
	}
	else
	{
		ShowMenu(InMenuPath);
	}
}

void UDevMenuManager::HideMenu(const FName InMenuPath)
{
	if (const FDisplayedMenuDetails* MenuDetails = FindMenu(InMenuPath))
	{
		RemoveMenu(*MenuDetails);
	}
}

void UDevMenuManager::HideAllMenus()
{
	EmptyMenus();
}

UGameViewportClient* UDevMenuManager::GetGameViewport() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetGameViewport();
	}
	return nullptr;
}

const UDevMenuManager::FDisplayedMenuDetails* UDevMenuManager::FindMenu(const FName InMenuPath) const
{
	for (int32 i = 0, Num = DisplayedMenus.Num(); i < Num; ++i)
	{
		if (const FDisplayedMenuDetails& MenuDetails = DisplayedMenus[i]; InMenuPath == MenuDetails.MenuPath)
		{
			return &MenuDetails;
		}
	}
	return nullptr;
}

const UDevMenuManager::FDisplayedMenuDetails* UDevMenuManager::FindTopMostMenu() const
{
	return (!DisplayedMenus.IsEmpty()) ? &DisplayedMenus.Last() : nullptr;
}

void UDevMenuManager::AddMenu(const FDisplayedMenuDetails&& InMenuDetails)
{
	if (DisplayedMenus.IsEmpty())
	{
		OnMenuShow.Broadcast();
		CloseConsole();
		CreateWidgets();
		GLog->AddOutputDevice(this);
	}

	const TSharedRef<SWidget> MenuWidget = InMenuDetails.MenuWidget;
	MenuGameLayerWidget->AddWidget(InMenuDetails.MenuWidget);
	BindMenuShortcuts(InMenuDetails.MenuPath);
	SetInputMode(MenuWidget);

	DisplayedMenus.Emplace(InMenuDetails);
}

void UDevMenuManager::PushMenuToTop(const FDisplayedMenuDetails& InMenuDetails)
{
	DisplayedMenus.Swap(DisplayedMenus.Find(InMenuDetails), DisplayedMenus.Num() - 1);
}

void UDevMenuManager::RemoveMenu(const FDisplayedMenuDetails& InMenuDetails)
{
	const TSharedRef<SWidget> MenuWidget = InMenuDetails.MenuWidget;
	MenuGameLayerWidget->RemoveWidget(MenuWidget);
	UnbindMenuShortcuts(InMenuDetails.MenuPath);

	DisplayedMenus.RemoveSingle(InMenuDetails);

	if (DisplayedMenus.IsEmpty())
	{
		GLog->RemoveOutputDevice(this);
		DestroyWidgets();
		RestoreInputMode();
		OnMenuHide.Broadcast();
	}
}

void UDevMenuManager::EmptyMenus()
{
	while (!DisplayedMenus.IsEmpty())
	{
		RemoveMenu(DisplayedMenus.Last());
	}
}

void UDevMenuManager::CreateWidgets()
{
	const UWorld* World = GetWorld();
	const EWorldType::Type WorldType = (World) ? World->WorldType.GetValue() : EWorldType::Type::None;

	MenuConsoleResponseWidget = SNew(SDevMenuConsoleResponseWidget);

	MenuGameLayerWidget = SNew(SDevMenuGameLayerWidget)
		.WorldType(WorldType);
	MenuGameLayerWidget
		->AddWidget(MenuConsoleResponseWidget->AsShared());

	if (UGameViewportClient* GameViewport = GetGameViewport())
	{
#if UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
		GameViewport->AddGameLayerWidget(MenuGameLayerWidget->AsShared(), INDEX_NONE);
#else
		GameViewport->AddViewportWidgetContent(MenuGameLayerWidget->AsShared(), INDEX_NONE);
#endif // UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
	}
}

void UDevMenuManager::DestroyWidgets()
{
	if (UGameViewportClient* GameViewport = GetGameViewport())
	{
#if UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
		GameViewport->RemoveGameLayerWidget(MenuGameLayerWidget->AsShared());
#else
		GameViewport->RemoveViewportWidgetContent(MenuGameLayerWidget->AsShared());
#endif // UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
	}

	MenuConsoleResponseWidget = nullptr;
	MenuGameLayerWidget = nullptr;
}

void UDevMenuManager::BindMenuShortcuts(const FName InMenuPath)
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		const FDevInputOwner InputOwner(InMenuPath);
		MenuRegistry->QueryEntries(FDevMenuGetEntriesByPathQuery(InMenuPath.ToString(), EDevMenuQueryDepth::AllEntries, FDevMenuQueryDelegate::CreateLambda([this, DevInputs, &InputOwner](const FDevMenuQueryResult& QueryResult)
		{
			if (const FDevInputShortcut& InputShortcut = QueryResult.Entry->GetInputShortcut(this).Get(); !InputShortcut.IsNone())
			{
				FDevInputShortcutDelegateBinding& Binding = DevInputs->BindShortcut(InputShortcut);
				Binding.Owner = InputOwner;
				Binding.Delegate.BindDelegate(this, &ThisClass::ExecuteMenuShortcut, FName(QueryResult.Path));
			}
			return EDevMenuQueryExecution::Continue;
		})));
	}
}

void UDevMenuManager::UnbindMenuShortcuts(const FName InMenuPath) const
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		const FDevInputOwner InputOwner(InMenuPath);
		DevInputs->ClearBindingsForOwner(InputOwner);
	}
}

void UDevMenuManager::ExecuteMenuShortcut(const FName InEntryPath)
{
	EntriesToExecuteFromShortcut.AddUnique(InEntryPath);

	// Immediate execution when multiple entries use the same shortcut may lead to undesired behavior.
	// Therefore, all entries intended for execution are collected first,
	// then evaluated against their visibility, and only after that executed.
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::ExecuteMenuShortcutDeferred));
}

bool UDevMenuManager::ExecuteMenuShortcutDeferred(const float DeltaTime)
{
	// Entry execution may have side effects.
	// Therefore, we first need to query all relevant entries, and only after that execute them.
	TArray<IDevMenuEntry*, TInlineAllocator<8>> EntriesToExecute; // TODO: Change to thread-local.
	for (const FName& EntryPath : EntriesToExecuteFromShortcut)
	{
		// We query entries through the Menu Registry because displayed menus are generated on-demand,
		// and there's no guarantee that the required entry already present in the generated menu.
		MenuRegistry->QueryEntries(FDevMenuGetEntriesByPathQuery(EntryPath.ToString(), EDevMenuQueryDepth::TopLevelEntries, FDevMenuQueryDelegate::CreateLambda([this, &EntriesToExecute](const FDevMenuQueryResult& QueryResult)
		{
			if (IDevMenuEntry* Entry = QueryResult.Entry; Entry->IsVisible(this))
			{
				EntriesToExecute.Add(Entry);
			}
			return EDevMenuQueryExecution::Continue;
		})));
	}
	EntriesToExecuteFromShortcut.Reset();

	for (IDevMenuEntry* Entry : EntriesToExecute)
	{
		Entry->ExecuteEntry(this);
	}

	return false;
}

void UDevMenuManager::SetInputMode(const TSharedPtr<SWidget>& InMenuWidget) const
{
	if (InMenuWidget)
	{
		if (const UDevInputs* DevInputs = UDevInputs::Get(this))
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(InMenuWidget);
			DevInputs->SetInputMode(InputMode);
		}
	}
}

void UDevMenuManager::RestoreInputMode() const
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->RestoreInputMode();
	}
}

void UDevMenuManager::CloseConsole() const
{
	if (const UGameViewportClient* GameViewport = GetGameViewport())
	{
		if (const TObjectPtr<UConsole>& Console = GameViewport->ViewportConsole; Console && Console->ConsoleActive())
		{
			Console->FakeGotoState(NAME_None);
		}
	}
}

void UDevMenuManager::RestoreFocus() const
{
	if (const FDisplayedMenuDetails* MenuDetails = FindTopMostMenu())
	{
		//UE_LOG_FUNCTION(LogDevMenus, Log, TEXT("Setting focus to top-most menu (Widget: %s)"), *WidgetToString(MenuWidget));
		const TSharedRef<SWidget> MenuWidget = MenuDetails->MenuWidget;
		FSlateApplication::Get().SetKeyboardFocus(MenuDetails->MenuWidget, EFocusCause::Navigation);
	}
}

void UDevMenuManager::OnConsoleActivationStateChanged(const bool bActive)
{
	if (bActive)
	{
		EmptyMenus();
	}
}

void UDevMenuManager::OnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
{
	if (DisplayedMenus.IsEmpty()) { return; }
	if (NewFocusedWidget) { return; }
	if (!FDevMenuNavigationExtension::IsExtensionNeeded(FocusEvent.GetUser())) { return; }
	if (const UDevInputs* DevInputs = UDevInputs::Get(this); DevInputs && !FDevMenuNavigationExtension::IsExtensionNeeded(DevInputs->GetCurrentInputType())) { return; }

	RestoreFocus();
}

void UDevMenuManager::OnInputTypeChanged(const EDevInputType InInputType)
{
	if (DisplayedMenus.IsEmpty()) { return; }
	if (!FDevMenuNavigationExtension::IsExtensionNeeded(InInputType)) { return; }
	if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetKeyboardFocusedWidget()) { return; }

	RestoreFocus();
}

void UDevMenuManager::OnViewportCreated()
{
	UGameViewportClient* GameViewport = GetGameViewport();
	if (!GameViewport) { return; }
	if (GameViewport->OnCloseRequested().IsBoundToObject(this)) { return; }

	GameViewport->OnCloseRequested().AddUObject(this, &UDevMenuManager::OnViewportCloseRequested);

	if (MenuGameLayerWidget && !MenuGameLayerWidget->IsParentValid())
	{
#if UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
		GameViewport->AddGameLayerWidget(MenuGameLayerWidget->AsShared(), INDEX_NONE);
#else
		GameViewport->AddViewportWidgetContent(MenuGameLayerWidget->AsShared(), INDEX_NONE);
#endif // UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
	}
}

void UDevMenuManager::OnViewportCloseRequested(FViewport* InViewport)
{
	UGameViewportClient* GameViewport = GetGameViewport();
	if (!GameViewport) { return; }
	if (InViewport->GetClient() != GameViewport) { return; }

	GameViewport->OnCloseRequested().RemoveAll(this);

	if (MenuGameLayerWidget && MenuGameLayerWidget->IsParentValid())
	{
#if UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
		GameViewport->RemoveGameLayerWidget(MenuGameLayerWidget->AsShared());
#else
		GameViewport->RemoveViewportWidgetContent(MenuGameLayerWidget->AsShared());
#endif // UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET
	}
}

void UDevMenuManager::OnWorldBeginTearDown(UWorld* InWorld)
{
	// Switching world breaking input capture. For now, we'll fix it by closing menus.
	EmptyMenus();
}
