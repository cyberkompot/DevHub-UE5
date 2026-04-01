// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "Templates/SharedPointer.h"
#include "DevMenuManager.generated.h"

class SDevMenuConsoleResponseWidget;
class SDevMenuGameLayerWidget;
class UDevMenuRegistry;
class UDevMenuWidgetBuilder;
class UGameViewportClient;
class FViewport;
class SWidget;

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevMenuManager final : public UObject
#if CPP
	, public FOutputDevice
#endif
{
	GENERATED_BODY()

public:
	void Initialize();
	void Dispose();

	void ShowMenu(const FName InMenuPath);
	void ToggleMenu(const FName InMenuPath);
	void HideMenu(const FName InMenuPath);

	void HideAllMenus();

	FORCEINLINE void SetMenuRegistry(UDevMenuRegistry& InMenuRegistry) { MenuRegistry = &InMenuRegistry; }
	FORCEINLINE void SetMenuWidgetBuilder(UDevMenuWidgetBuilder& InMenuWidgetBuilder) { MenuWidgetBuilder = &InMenuWidgetBuilder; }

	//~ Begin UObject interface.
	virtual void BeginDestroy() override;
	//~ End UObject interface.

	//~ Begin FOutputDevice interface.
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	//~ End FOutputDevice interface.

private:
	struct FDisplayedMenuDetails final
	{
		FDisplayedMenuDetails(const FName InMenuPath, const TSharedRef<SWidget>& InMenuWidget)
			: MenuPath(InMenuPath), MenuWidget(InMenuWidget) {}

		FName MenuPath;
		TSharedRef<SWidget> MenuWidget;

		FORCEINLINE bool operator ==(const FDisplayedMenuDetails& Rhs) const { return (MenuPath == Rhs.MenuPath && MenuWidget == Rhs.MenuWidget); };
	};

	TArray<FDisplayedMenuDetails> DisplayedMenus;
	TSharedPtr<SDevMenuGameLayerWidget> MenuGameLayerWidget;
	TSharedPtr<SDevMenuConsoleResponseWidget, ESPMode::ThreadSafe> MenuConsoleResponseWidget;

	UPROPERTY(Transient)
	UDevMenuRegistry* MenuRegistry;

	UPROPERTY(Transient)
	UDevMenuWidgetBuilder* MenuWidgetBuilder;

	TArray<FName> EntriesToExecuteFromShortcut;

	UGameViewportClient* GetGameViewport() const;

	const FDisplayedMenuDetails* FindMenu(const FName InMenuPath)  const;
	const FDisplayedMenuDetails* FindTopMostMenu()  const;

	void AddMenu(const FDisplayedMenuDetails&& InMenuDetails);
	void PushMenuToTop(const FDisplayedMenuDetails& InMenuDetails);
	void RemoveMenu(const FDisplayedMenuDetails& InMenuDetails);
	void EmptyMenus();

	void CreateWidgets();
	void DestroyWidgets();

	void BindMenuShortcuts(const FName InMenuPath);
	void UnbindMenuShortcuts(const FName InMenuPath) const;
	void ExecuteMenuShortcut(const FName InEntryPath);
	bool ExecuteMenuShortcutDeferred(const float DeltaTime);

	void SetInputMode(const TSharedPtr<SWidget>& InMenuWidget) const;
	void RestoreInputMode() const;

	void CloseConsole() const;
	void RestoreFocus() const;

	void OnConsoleActivationStateChanged(const bool bActive);
	void OnFocusChanging(const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget);
	void OnInputTypeChanged(const EDevInputType InInputType);
	void OnViewportCreated();
	void OnViewportCloseRequested(FViewport* InViewport);
	void OnWorldBeginTearDown(UWorld* InWorld);
};
