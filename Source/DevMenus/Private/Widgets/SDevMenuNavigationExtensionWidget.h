// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Templates/SharedPointer.h"

class SMenuAnchor;
class UDevMenus;
class UDevInputs;

namespace DevMenu::Navigation
{
	struct FNavigationContext;
	enum class ENavigationDirection : uint8;
}

using namespace DevMenu::Navigation;

struct FDevMenuNavigationExtension final
{
	FORCEINLINE static bool IsExtensionNeeded(const ENavigationGenesis InGenesis) { return (InGenesis == ENavigationGenesis::Keyboard || InGenesis == ENavigationGenesis::Controller); }
	FORCEINLINE static bool IsExtensionNeeded(const EDevInputType InInputType) { return (InInputType == EDevInputType::Keyboard || InInputType == EDevInputType::Gamepad); }
	FORCEINLINE static bool IsExtensionNeeded(const uint32 InUser) { return (FSlateApplication::IsInitialized() && InUser == FSlateApplication::Get().GetUserIndexForKeyboard()); }
};

class SDevMenuNavigationExtensionWidgetBase : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SDevMenuNavigationExtensionWidgetBase, SCompoundWidget)

public:
	virtual FName GetMenuPath() const PURE_VIRTUAL(GetMenuPath, return NAME_None;)

	//~ Begin SWidget interface.
	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FNavigationReply OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent) override;
	virtual FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual void Tick(const FGeometry& MyGeometry, const double InCurrentTime, const float InDeltaTime) override;
	//~ End SWidget interface.

private:
	FWeakWidgetPath FocusedWidgets;
	TSet<TWeakPtr<SWidget>> HoveredWidgets;

	FWeakWidgetPath LastFocusPath;
	EDevInputType LastInputType = EDevInputType::Undefined;

	bool bIgnoreFocusChangingEvent = false;
	bool bIgnoreNavigationEvent = false;
	bool bStateChanged = false;

	void CheckInputTypeState();
	void UpdateHoveredWidgets();
	void UpdateFocusedWidget();

private:
	FNavigationContext GetNavigationContext() const;
	FNavigationContext GetNavigationContext(const TSharedPtr<SWidget>& InWidget) const;

	FWidgetPath GetPathToNextFocusWidget(FWeakWidgetPath InFocusSearch, EUINavigation InNavigationType, const FNavigationReply& InNavigationReply, const FArrangedWidget& InRuleWidget) const;
	FWidgetPath GetPathToPreviouslyFocusedWidget(const FWeakWidgetPath& InPreviousFocusPath, const FWeakWidgetPath& InNewWidgetPath) const;

	bool IsKeyboardFocusExtensionNeededForWidget(const TSharedPtr<SWidget>& InWidget) const;

private:
	bool CloseRootMenu() const;
	TSharedPtr<SWidget> CloseRootOrTopMostSubMenu(const FNavigationContext& InContext) const;
	TSharedPtr<SWidget> CloseAllSubMenus() const;

	TSharedPtr<SWidget> OpenSubMenu(const FNavigationContext& InContext) const;
	TSharedPtr<SWidget> CloseSubMenu(const FNavigationContext& InContext) const;

	TSharedPtr<SWidget> NavigateToSubMenu(const ENavigationDirection InNavigationDirection) const;
	TSharedPtr<SWidget> NavigateToMenuEntry(const FNavigationContext& InContext, const ENavigationDirection InNavigationDirection, const FNavigationReply& InNavigationReply) const;

	void SetKeyboardFocus(const FWidgetPath& InFocusPath, const EFocusCause InCause);

private:
	mutable TWeakObjectPtr<UDevInputs> CachedDevInputs;
	mutable TWeakObjectPtr<UDevMenus> CachedDevMenus;

	UDevInputs* GetDevInputsSubsystem() const;
	UDevMenus* GetDevMenusSubsystem() const;

private:
	void DoOnTick(TFunction<void()> InLambda);

	DECLARE_MULTICAST_DELEGATE(FOnTickEvent);
	FOnTickEvent OnTickEvent;
	void OnTick();
};

namespace DevMenu::Navigation
{
	struct EWidgetTypes
	{
		static const FLazyName MenuAnchor;
		static const FLazyName ButtonType;
		static const FLazyName SubMenuButtonType;

		static const TArray<FLazyName> KeyboardFocusBypassWidgetTypes;
		static const TArray<FLazyName> KeyboardFocusHoveringEffectWidgetsTypes;

		FORCEINLINE static bool IsKeyboardFocusBypassWidgetType(const FName InWidgetType) { return KeyboardFocusBypassWidgetTypes.Contains(InWidgetType); }
		FORCEINLINE static bool IsKeyboardFocusHoveringEffectWidgetType(const FName InWidgetType) { return KeyboardFocusHoveringEffectWidgetsTypes.Contains(InWidgetType); }
	};

	enum class ENavigationMenuType : uint8
	{
		Undefined = 0,
		HorizontalMenu, // Main menu.
		VerticalMenu, // Pull-down, context, or sub-menu menu.
	};

	enum class ENavigationMenuEntryType : uint8
	{
		Undefined = 0,
		MenuButton, // Pull-down or sub-menu button.
	};

	enum class ENavigationDirection : uint8
	{
		Invalid = 0,

		/** Four entering and leaving menus. */
		Enter,
		Leave,

		/** Four looping over menu entries. */
		Next,
		Previous,
	};

	struct FWidgetDetails
	{
		FArrangedWidget Widget = FArrangedWidget::GetNullWidget();
		FWidgetPath WidgetPath;

		static FWidgetDetails CreateFromWidget(const TSharedPtr<SWidget>& InWidget);
		static FWidgetDetails CreateFromWidget(const TSharedPtr<SWidget>& InWidget, const TSharedPtr<SWidget>& InParent);

		FORCEINLINE TSharedPtr<SWidget> GetWidget() const { return Widget.Widget; }

		FORCEINLINE bool IsValid() const { return Widget.GetWidgetPtr() && WidgetPath.IsValid(); }
	};

	struct FMenuWidgetDetails : FWidgetDetails
	{
		TSharedPtr<IMenu> WidgetMenu;

		static FMenuWidgetDetails CreateFromWidget(const TSharedPtr<SWidget>& InWidget);
		static FMenuWidgetDetails CreateFromWidget(const TSharedPtr<SWidget>& InWidget, const TSharedPtr<SWidget>& InParent);

		FORCEINLINE bool IsValid() const { return WidgetMenu && FWidgetDetails::IsValid(); }
	};

	struct FNavigationContext
	{
		FWidgetDetails SelfDetails;
		FWidgetDetails TargetDetails;
		FMenuWidgetDetails SubMenuDetails;

		TSharedPtr<SMenuAnchor> GetMenuAnchorWidget() const;
		ENavigationMenuEntryType GetMenuEntryType() const;
		ENavigationMenuType GetMenuType() const;

		FORCEINLINE bool IsValid() const { return SelfDetails.IsValid() && TargetDetails.IsValid(); } // SubMenuDetails may be undefined if there is no opened sub-menu.
	};

	static TSharedPtr<SWidget> GetKeyboardFocusedChildWidget(const TSharedPtr<SWidget>& InParent);
	static ENavigationDirection GetNavigationDirection(const FNavigationContext& InContext, EUINavigation InType);
	static FWidgetPath GetPathToWidget(const TSharedPtr<SWidget>& InWidget) { return FWidgetDetails::CreateFromWidget(InWidget).WidgetPath; }

	static TSharedPtr<SWidget> FindChildWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType);
	static TSharedPtr<SWidget> FindParentWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType);
	static TSharedPtr<SWidget> FindNearestWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType);
} // namespace DevMenu::Navigation
