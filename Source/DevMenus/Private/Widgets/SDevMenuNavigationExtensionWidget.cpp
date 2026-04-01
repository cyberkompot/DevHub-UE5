// Copyright (c) Alexandr Pereverzev.

#include "SDevMenuNavigationExtensionWidget.h"

#include "DevInputs.h"
#include "DevMenuLogging.h"
#include "DevMenus.h"
#include "DevMenuSlateBridge.h"
#include "Framework/Application/SlateApplication.h"
#include "Types/SlateAttributeMetaData.h"

using namespace DevMenu::Logging;
using namespace DevMenu::Navigation;

SLATE_IMPLEMENT_WIDGET(SDevMenuNavigationExtensionWidgetBase)

void SDevMenuNavigationExtensionWidgetBase::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SDevMenuNavigationExtensionWidgetBase::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath,	const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	Super::OnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);

	// OnFocusChanging may be invoked multiple times with the same parameters.
	if (NewWidgetPath.IsValid() && LastFocusPath.IsValid() && LastFocusPath.GetLastWidget() == NewWidgetPath.GetLastWidget()) { return; }

	LastFocusPath = (NewWidgetPath.IsValid() && NewWidgetPath.ContainsWidget(this))? NewWidgetPath : FWeakWidgetPath();
	bStateChanged = true;
}

FReply SDevMenuNavigationExtensionWidgetBase::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (const FKey Key = InKeyEvent.GetKey(); Key == EKeys::Gamepad_FaceButton_Right)
	{
		UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Extending navigation, emulating Esc key press on Gamepad action"));
		if (const UDevInputs* DevInputs = GetDevInputsSubsystem())
		{
			DevInputs->EmulateKeyPress(EKeys::Escape);
			return FReply::Handled();
		}
	}
	else if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Gamepad_RightShoulder)
	{
		UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Extending navigation, switching to top-level sub-menu"));
		const ENavigationDirection NavigationDirection = (Key == EKeys::Gamepad_LeftShoulder)
			? ENavigationDirection::Previous
			: ENavigationDirection::Next;
		if (const TSharedPtr<SWidget> NextFocusWidget = NavigateToSubMenu(NavigationDirection))
		{
			return FReply::Handled().SetUserFocus(NextFocusWidget->AsShared(), EFocusCause::Navigation);
		}
	}

	return Super::OnKeyDown(MyGeometry, InKeyEvent);
}

FNavigationReply SDevMenuNavigationExtensionWidgetBase::OnNavigation(const FGeometry& MyGeometry, const FNavigationEvent& InNavigationEvent)
{
	if (bIgnoreNavigationEvent) { return Super::OnNavigation(MyGeometry, InNavigationEvent); }
	if (!FDevMenuNavigationExtension::IsExtensionNeeded(InNavigationEvent.GetNavigationGenesis())) { return Super::OnNavigation(MyGeometry, InNavigationEvent); }

	const FNavigationContext Context = GetNavigationContext();
	if (!Context.IsValid()) { return Super::OnNavigation(MyGeometry, InNavigationEvent); }

	UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Navigation: Type = %s, Path = %s"), *EnumToString(InNavigationEvent.GetNavigationType()), *WidgetPathToString(Context.TargetDetails.WidgetPath, this));
	switch (const ENavigationDirection NavigationDirection = GetNavigationDirection(Context, InNavigationEvent.GetNavigationType()))
	{
		case ENavigationDirection::Enter:
			if (Context.GetMenuEntryType() == ENavigationMenuEntryType::MenuButton)
			{
				UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to open sub-menu"));
				if (const TSharedPtr<SWidget> NextFocusWidget = OpenSubMenu(Context))
				{
					UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to menu entry: Path = %s"), *WidgetPathToString(NextFocusWidget, this));
					return FNavigationReply::Custom(FNavigationDelegate::CreateLambda([NextFocusWidget](EUINavigation) { return NextFocusWidget; }));
				}
			}
			else if (Context.SubMenuDetails.IsValid())
			{
				DoOnTick([this, ExpectedWidget = Context.TargetDetails.GetWidget()]()
				{
					// Extend navigation only if the originally focused widget is still focused. That means no sub-menu has been opened and there has been no navigation within the original menu entry.
					if (ExpectedWidget == GetKeyboardFocusedChildWidget(AsShared()))
					{
						UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to next top-leven sub-menu"));
						if (const TSharedPtr<SWidget> NextFocusWidget = NavigateToSubMenu(ENavigationDirection::Next))
						{
							UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to menu entry: Path = %s"), *WidgetPathToString(NextFocusWidget, this));
							SetKeyboardFocus(GetPathToWidget(NextFocusWidget), EFocusCause::Navigation);
						}
					}
				});
			}
			break;

		case ENavigationDirection::Leave:
			if (Context.SubMenuDetails.IsValid())
			{
				DoOnTick([this, ExpectedWidget = Context.TargetDetails.GetWidget()]()
				{
					// Extend navigation only if the originally focused widget is still focused. That means no sub-menu has been closed and there has been no navigation within the original menu entry.
					if (ExpectedWidget == GetKeyboardFocusedChildWidget(AsShared()))
					{
						UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to previous top-leven sub-menu"));
						if (const TSharedPtr<SWidget> NextFocusWidget = NavigateToSubMenu(ENavigationDirection::Previous))
						{
							UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to menu entry: Path = %s"), *WidgetPathToString(NextFocusWidget, this));
							SetKeyboardFocus(GetPathToWidget(NextFocusWidget), EFocusCause::Navigation);
							//return FNavigationReply::Custom(FNavigationDelegate::CreateLambda([NextFocusWidget](EUINavigation) { return NextFocusWidget; }));
						}
					}
				});
			}
			break;

		case ENavigationDirection::Previous:
		case ENavigationDirection::Next:
			UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to wrap around menu entries"));
			if (const TSharedPtr<SWidget> NextFocusWidget = NavigateToMenuEntry(Context, NavigationDirection, FNavigationReply::Wrap()))
			{
				UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote navigation to wrap around menu entries: Path = %s"), *WidgetPathToString(NextFocusWidget, this));
				return FNavigationReply::Custom(FNavigationDelegate::CreateLambda([NextFocusWidget](EUINavigation) { return NextFocusWidget; }));
			}

		case ENavigationDirection::Invalid:
		default:
			break;
	}

	return Super::OnNavigation(MyGeometry, InNavigationEvent);
}

FReply SDevMenuNavigationExtensionWidgetBase::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (const FKey Key = InKeyEvent.GetKey(); Key == EKeys::Escape)
	{
		if (const FNavigationContext Context = GetNavigationContext(); Context.IsValid())
		{
			UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, suppressing Esc key down event, and closing root or top-most sub-menu"));
			FReply Reply = FReply::Handled();
			if (const TSharedPtr<SWidget> NextFocusWidget = CloseRootOrTopMostSubMenu(Context))
			{
				Reply.SetUserFocus(NextFocusWidget->AsShared(), EFocusCause::Navigation);
			}
			return Reply; // We will suppress Esc key even if closing fails.
		}
	}

	return Super::OnPreviewKeyDown(MyGeometry, InKeyEvent);
}

void SDevMenuNavigationExtensionWidgetBase::Tick(const FGeometry& MyGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(MyGeometry, InCurrentTime, InDeltaTime);

	OnTick();

	CheckInputTypeState();

	UpdateFocusedWidget();
	UpdateHoveredWidgets();

	bStateChanged = false;
}

void SDevMenuNavigationExtensionWidgetBase::CheckInputTypeState()
{
	if (const UDevInputs* DevInputs = GetDevInputsSubsystem())
	{
		if (const EDevInputType InputType = DevInputs->GetCurrentInputType(); LastInputType != InputType)
		{
			LastInputType = InputType;
			bStateChanged = true;
		}
	}
}

void SDevMenuNavigationExtensionWidgetBase::UpdateHoveredWidgets()
{
	static TSet<TWeakPtr<SWidget>> PreviouslyHoveredWidgets = TSet<TWeakPtr<SWidget>>();

	if (!bStateChanged) { return; }

	PreviouslyHoveredWidgets = HoveredWidgets;
	HoveredWidgets.Empty();

	if (FDevMenuNavigationExtension::IsExtensionNeeded(LastInputType))
	{
		for (const TWeakPtr<SWidget>& WidgetPtr : LastFocusPath.Widgets)
		{
			HoveredWidgets.Add(WidgetPtr);
			if (!PreviouslyHoveredWidgets.Contains(WidgetPtr))
			{
				if (const TSharedPtr<SWidget> Widget = WidgetPtr.Pin(); Widget && EWidgetTypes::IsKeyboardFocusHoveringEffectWidgetType(Widget->GetType()))
				{
					Widget->OnMouseEnter(Widget->GetCachedGeometry(), FPointerEvent());
				}
			}
		}
	}

	for (const TWeakPtr<SWidget>& WidgetPtr : PreviouslyHoveredWidgets)
	{
		if (!HoveredWidgets.Contains(WidgetPtr))
		{
			if (const TSharedPtr<SWidget> Widget = WidgetPtr.Pin())
			{
				Widget->OnMouseLeave(FPointerEvent());
			}
		}
	}

	PreviouslyHoveredWidgets.Empty();
}

void SDevMenuNavigationExtensionWidgetBase::UpdateFocusedWidget()
{
	if (!bStateChanged) { return; }

	if (FDevMenuNavigationExtension::IsExtensionNeeded(LastInputType))
	{
		if (LastFocusPath.IsValid())
		{
			if (const TSharedPtr<SWidget> FocusWidget = LastFocusPath.GetLastWidget().Pin(); IsKeyboardFocusExtensionNeededForWidget(FocusWidget))
			{
				const FWeakWidgetPath& NewWidgetPath = LastFocusPath;
				const FWeakWidgetPath& PreviousFocusPath = FocusedWidgets;

				const ENavigationDirection AssumedNavigationDirection = (NewWidgetPath.Widgets.Num() >= PreviousFocusPath.Widgets.Num())
						? ENavigationDirection::Next
						: ENavigationDirection::Previous;
				if (AssumedNavigationDirection == ENavigationDirection::Next)
				{
					const FWidgetPath NextFocusPath = GetPathToNextFocusWidget(NewWidgetPath, EUINavigation::Next, FNavigationReply::Stop(), FArrangedWidget(FocusWidget->AsShared(), FocusWidget->GetCachedGeometry()));
					if (NextFocusPath.IsValid())
					{
						UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote focus change from bypassable widget to most relevant widget: Path = %s"), *DevMenu::Logging::WidgetPathToString(NextFocusPath, this));
						SetKeyboardFocus(NextFocusPath, EFocusCause::Navigation);
					}
				}
				else // if (AssumedNavigationDirection == ENavigationDirection::Previous)
				{
					const FWidgetPath NextFocusPath = GetPathToPreviouslyFocusedWidget(PreviousFocusPath, NewWidgetPath);
					if (NextFocusPath.IsValid())
					{
						UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Extending navigation, promote focus change from bypassable widget to most relevant widget: Path = %s"), *DevMenu::Logging::WidgetPathToString(NextFocusPath, this));
						SetKeyboardFocus(NextFocusPath, EFocusCause::Navigation);
					}
				}
			}
		}
	}

	FocusedWidgets = LastFocusPath;	// SetKeyboardFocus triggers OnFocusReceived and updates LastFocusPath to reflect the new state.
}

FNavigationContext SDevMenuNavigationExtensionWidgetBase::GetNavigationContext() const
{
	const TSharedRef<SWidget> SelfWidget = ConstCastSharedRef<SWidget>(AsShared());
	return GetNavigationContext(GetKeyboardFocusedChildWidget(SelfWidget));
}

FNavigationContext SDevMenuNavigationExtensionWidgetBase::GetNavigationContext(const TSharedPtr<SWidget>& InWidget) const
{
	const TSharedRef<SWidget> SelfWidget = ConstCastSharedRef<SWidget>(AsShared());

	FNavigationContext Context = FNavigationContext();
	Context.SelfDetails = FWidgetDetails::CreateFromWidget(SelfWidget);
	Context.TargetDetails = FWidgetDetails::CreateFromWidget(InWidget, SelfWidget);
	Context.SubMenuDetails = FMenuWidgetDetails::CreateFromWidget(Context.TargetDetails.Widget.Widget, SelfWidget);

	return Context;
}

FWidgetPath SDevMenuNavigationExtensionWidgetBase::GetPathToNextFocusWidget(FWeakWidgetPath InFocusSearch, EUINavigation InNavigationType, const FNavigationReply& InNavigationReply, const FArrangedWidget& InRuleWidget) const
{
	if (!InFocusSearch.IsValid()) { return FWidgetPath(); }

	thread_local TArray<TSharedPtr<SWidget>> PreviouslyFoundedFocusWidgets;
	PreviouslyFoundedFocusWidgets.Reserve(8);
	PreviouslyFoundedFocusWidgets.Empty();

	TWeakPtr<SWidget> FocusWidget = InFocusSearch.GetLastWidget();

	FWidgetPath NextFocusPath;
	EUINavigation NavigationType = InNavigationType;
	FNavigationReply NavigationReply = InNavigationReply;
	bool RepeatUntilBoundary = false;

	// Searching for the most relevant widget by traversing the hierarchy,
	// bypassing widgets that are focusable but not interactable.
	bool Repeat;
	do
	{
		Repeat = false;
		if (NextFocusPath = InFocusSearch.ToNextFocusedPath(NavigationType, NavigationReply, InRuleWidget); NextFocusPath.IsValid())
		{
			const TSharedPtr<SWidget> NextFocusWidget = NextFocusPath.GetLastWidget();
			const FName NextFocusWidgetType = NextFocusWidget->GetType();

			if (PreviouslyFoundedFocusWidgets.Contains(NextFocusWidget))
			{
				// When wrapping and reaching the beginning of the boundary, reverse direction and continue searching until the end.
				if (NavigationType == EUINavigation::Previous && NavigationReply.GetBoundaryRule() == EUINavigationRule::Wrap)
				{
					NavigationType = EUINavigation::Next;
					NavigationReply = FNavigationReply::Stop();
					PreviouslyFoundedFocusWidgets.Empty();
					RepeatUntilBoundary = true;
					Repeat = true;
				}
			}
			else
			{
				if (RepeatUntilBoundary || EWidgetTypes::IsKeyboardFocusBypassWidgetType(NextFocusWidgetType))
				{
					PreviouslyFoundedFocusWidgets.Add(NextFocusWidget);
					Repeat = true;
				}
			}

			if (Repeat)
			{
				InFocusSearch = NextFocusPath;
			}
		}
	} while (Repeat);

	PreviouslyFoundedFocusWidgets.Empty();

	// If the search returns the same widget, we treat it as unsuccessful and return an invalid widget path.
	if (NextFocusPath.IsValid() && NextFocusPath.GetLastWidget() == FocusWidget) { return FWidgetPath(); }

	return NextFocusPath;
}

FWidgetPath SDevMenuNavigationExtensionWidgetBase::GetPathToPreviouslyFocusedWidget(const FWeakWidgetPath& InPreviousFocusPath, const FWeakWidgetPath& InNewWidgetPath) const
{
	const TArray<TWeakPtr<SWidget>>& PreviousFocusWidgets = InPreviousFocusPath.Widgets;
	const TArray<TWeakPtr<SWidget>>& NewFocusWidgets = InNewWidgetPath.Widgets;

	if (NewFocusWidgets.IsEmpty()) { return FWidgetPath(); }
	if (PreviousFocusWidgets.IsEmpty()) { return FWidgetPath(); }
	if (PreviousFocusWidgets.Num() < NewFocusWidgets.Num()) { return FWidgetPath(); }

	TSharedPtr<SWidget> LastFoundFocusableWidget;
	TSharedPtr<SWidget> LastFoundWidget;
	int32 i = 0;

	// Verifying that new focused path fully included into previous focused path widgets' hierarchy.
	for (const int32 Num = NewFocusWidgets.Num(); i < Num; ++i)
	{
		if (NewFocusWidgets[i] != PreviousFocusWidgets[i])
		{
			return FWidgetPath();
		}
	}

	// Starting from the end of new focus path, find the deepest child in the previous focus path.
	// Filtering widgets using the same approach as FSlateWindowHelper::FindPathToWidget(),
	// ensuring consistency with how widget paths are resolved within the Slate framework.
	for (const int32 Num = PreviousFocusWidgets.Num(); i < Num; ++i)
	{
		const TSharedPtr<SWidget> Widget = PreviousFocusWidgets[i].Pin();

		if (!Widget) { break; }
		if (!Widget->IsEnabled()) { break; }

		// Update the Widgets visibility before getting the ArrangeChildren.
		FSlateAttributeMetaData::UpdateOnlyVisibilityAttributes(*Widget.Get(), FSlateAttributeMetaData::EInvalidationPermission::DelayInvalidation);
		if (!Widget->GetVisibility().IsVisible()) { break; }

		TSharedPtr<SWidget> WidgetParent = Widget->GetParentWidget();
		if (!WidgetParent) { break; }

		// Even if the parent pointer is valid, and even if the visibility is visible,
		// it's possible a widget shows and hides children without ever removing them
		// this is the case with widgets like SWidgetSwitcher.
		if (!WidgetParent->ValidatePathToChild(Widget.Get())) { break; }

		LastFoundWidget = Widget;
		if (Widget->SupportsKeyboardFocus())
		{
			LastFoundFocusableWidget = Widget;
		}
	}


	FSlateApplication& SlateApp = FSlateApplication::Get();

	// Menu entries widgets hierarchy makes the last focused widget suboptimal,
	// better to continue focus search from the deepest child instead.
	if (LastFoundWidget && LastFoundWidget != LastFoundFocusableWidget)
	{
		FWidgetPath LastFoundPath;
		SlateApp.FindPathToWidget(LastFoundWidget.ToSharedRef(), LastFoundPath);
		if (LastFoundPath.IsValid())
		{
			FArrangedWidget RuleWidget = InNewWidgetPath.ContainsWidget(this)
				? FArrangedWidget(ConstCastSharedRef<SWidget>(AsShared()), GetCachedGeometry())
				: FArrangedWidget::GetNullWidget();
			const FWidgetPath NextFocusPath = GetPathToNextFocusWidget(LastFoundPath, EUINavigation::Next, FNavigationReply::Escape(), RuleWidget);
			if (NextFocusPath.IsValid())
			{
				return NextFocusPath;
			}
		}
	}

	// If the above search fails, the last focused widget becomes our best option.
	if (LastFoundFocusableWidget)
	{
		FWidgetPath LastFoundFocusablePath;
		SlateApp.FindPathToWidget(LastFoundFocusableWidget.ToSharedRef(), LastFoundFocusablePath);
		if (LastFoundFocusablePath.IsValid())
		{
			const FName LastFocusableWidgetType = LastFoundFocusableWidget->GetType();
			if (EWidgetTypes::IsKeyboardFocusBypassWidgetType(LastFocusableWidgetType))
			{
				FArrangedWidget RuleWidget = InNewWidgetPath.ContainsWidget(this)
					? FArrangedWidget(ConstCastSharedRef<SWidget>(AsShared()), GetCachedGeometry())
					: FArrangedWidget::GetNullWidget();
				const FWidgetPath NextFocusPath = GetPathToNextFocusWidget(LastFoundFocusablePath, EUINavigation::Next, FNavigationReply::Escape(), RuleWidget);
				if (NextFocusPath.IsValid())
				{
					return NextFocusPath;
				}
			}
			return LastFoundFocusablePath;
		}
	}

	return FWidgetPath();
}

bool SDevMenuNavigationExtensionWidgetBase::IsKeyboardFocusExtensionNeededForWidget(const TSharedPtr<SWidget>& InWidget) const
{
	if (!InWidget) { return false; }

	return (InWidget.Get() == this) || EWidgetTypes::IsKeyboardFocusBypassWidgetType(InWidget->GetType());
}

bool SDevMenuNavigationExtensionWidgetBase::CloseRootMenu() const
{
	if (const FName MenuPath = GetMenuPath(); !MenuPath.IsNone())
	{
		if (UDevMenus* DevMenus = GetDevMenusSubsystem())
		{
			DevMenus->HideMenu(MenuPath);
			return true;
		}
	}

	UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to close root-menu, menu path is undefined or Dev Menus subsystem is missing"));
	return false;
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::CloseRootOrTopMostSubMenu(const FNavigationContext& InContext) const
{
	if (InContext.SubMenuDetails.IsValid())
	{
		return CloseSubMenu(InContext);
	}
	else
	{
		(void)CloseRootMenu();
		return nullptr;
	}
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::CloseAllSubMenus() const
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Closing all sub-menus"));

	FNavigationContext Context = GetNavigationContext();
	while (Context.SubMenuDetails.IsValid())
	{
		if (const TSharedPtr<SWidget> NextFocusWidget = CloseSubMenu(Context))
		{
			Context = GetNavigationContext(NextFocusWidget);
		}
		else
		{
			return nullptr;
		}
	}

	return Context.TargetDetails.Widget.Widget;
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::OpenSubMenu(const FNavigationContext& InContext) const
{
	if (InContext.GetMenuEntryType() == ENavigationMenuEntryType::MenuButton)
	{
		if (const TSharedPtr<SMenuAnchor> MenuAnchorWidget = InContext.GetMenuAnchorWidget())
		{
			if (MenuAnchorWidget->IsOpen())
			{
				return NavigateToMenuEntry(InContext, ENavigationDirection::Enter, FNavigationReply::Stop());
			}
			else
			{
				MenuAnchorWidget->SetIsOpen(true);
				const FNavigationContext ContextAfterOpening = GetNavigationContext();
				return NavigateToMenuEntry(ContextAfterOpening, ENavigationDirection::Enter, FNavigationReply::Stop());
			}
		}
		else
		{
			UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to open sub-menu, Menu Anchor Widget is missing"));
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to open sub-menu, unexpected Menu Entry Type"));
	}
	return nullptr;
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::CloseSubMenu(const FNavigationContext& InContext) const
{
	if (InContext.SubMenuDetails.IsValid())
	{
		UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Closing sub-menu"));

		FSlateApplication& SlateApp = FSlateApplication::Get();
		SlateApp.DismissMenu(InContext.SubMenuDetails.WidgetMenu);

		const FNavigationContext NewContext = GetNavigationContext();
		const FWidgetPath CurrentFocusPath = (NewContext.TargetDetails.IsValid())
			? NewContext.TargetDetails.WidgetPath
			: NewContext.SelfDetails.WidgetPath;
		const FWidgetPath PreviouslyFocusedPath = GetPathToPreviouslyFocusedWidget(LastFocusPath, CurrentFocusPath);
		const FWidgetPath NextFocusPath = (PreviouslyFocusedPath.IsValid())
			? PreviouslyFocusedPath
			: NewContext.SelfDetails.WidgetPath;
		return (NextFocusPath.IsValid()) ? NextFocusPath.GetLastWidget() : TSharedPtr<SWidget>();
	}

	UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to close sub-menu, sub-menu not found"));
	return nullptr;
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::NavigateToSubMenu(const ENavigationDirection InNavigationDirection) const
{
	const FNavigationContext Context = GetNavigationContext();
	if (!Context.SubMenuDetails.IsValid()) { return nullptr; }

	// This method mirrors standard OS menu behavior:
	// - Navigate to the previous sub-menu (left): sequentially close all open sub-menus, and only then switch to the previous top-level menu.
	// - Navigate to the next sub-menu (right): close all opened sub-menus, then jump to the next top-level sub-menu.
	const TSharedPtr<SWidget> NextFocusWidget = (InNavigationDirection == ENavigationDirection::Previous)
		? CloseSubMenu(Context)
		: CloseAllSubMenus();

	// If the operation fails on any stage, the method returns the most relevant widget.
	if (NextFocusWidget)
	{
		if (const FNavigationContext NextContext = GetNavigationContext(NextFocusWidget); NextContext.IsValid())
		{
			if (!NextContext.SubMenuDetails.IsValid())
			{
				if (const TSharedPtr<SWidget> NextFocusMenuEntryWidget = NavigateToMenuEntry(NextContext, InNavigationDirection, FNavigationReply::Wrap()))
				{
					if (const FNavigationContext MenuEntryContext = GetNavigationContext(NextFocusMenuEntryWidget); MenuEntryContext.GetMenuEntryType() == ENavigationMenuEntryType::MenuButton)
					{
						if (const TSharedPtr<SWidget> NextFocusSubMenuWidget = OpenSubMenu(MenuEntryContext))
						{
							return NextFocusSubMenuWidget;
						}
						else
						{
							UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to navigate to top-level sub-menu, no next sub-menu widget"));
							return MenuEntryContext.TargetDetails.GetWidget();
						}
					}
					else
					{
						UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to navigate to top-level sub-menu, no valid next menu entry navigation context"));
						return NextFocusMenuEntryWidget;
					}
				}
				else
				{
					UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to navigate to top-level sub-menu, no next menu entry widget"));
					return NextContext.TargetDetails.GetWidget();
				}
			}
			else
			{
				return NextContext.TargetDetails.GetWidget();
			}
		}
		else
		{
			UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to navigate to top-level sub-menu, no valid navigation context"));
			return NextFocusWidget;
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to navigate to top-level sub-menu, no focused widget"));
		return nullptr;
	}
}

TSharedPtr<SWidget> SDevMenuNavigationExtensionWidgetBase::NavigateToMenuEntry(const FNavigationContext& InContext, const ENavigationDirection InNavigationDirection, const FNavigationReply& InNavigationReply) const
{
	const EUINavigation NavigationType = (InNavigationDirection == ENavigationDirection::Previous)
		? EUINavigation::Previous
		: EUINavigation::Next;
	const FArrangedWidget RuleWidget = (InContext.GetMenuType() == ENavigationMenuType::HorizontalMenu)
		? InContext.SelfDetails.Widget
		: InContext.SubMenuDetails.Widget;

	const FWidgetPath NextFocusPath = GetPathToNextFocusWidget(InContext.TargetDetails.WidgetPath, NavigationType, InNavigationReply, RuleWidget);
	return (NextFocusPath.IsValid()) ? NextFocusPath.GetLastWidget() : TSharedPtr<SWidget>();
}

void SDevMenuNavigationExtensionWidgetBase::SetKeyboardFocus(const FWidgetPath& InFocusPath, const EFocusCause InCause)
{
	TGuardValue Guard(bIgnoreFocusChangingEvent, true);

	UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Setting focus: Case = %s, Path = %s"), *EnumToString(InCause), *WidgetPathToString(InFocusPath, this));

	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.SetKeyboardFocus(InFocusPath, InCause);
}

UDevInputs* SDevMenuNavigationExtensionWidgetBase::GetDevInputsSubsystem() const
{
	if (!CachedDevInputs.IsValid())
	{
		CachedDevInputs = FDevMenuSlateBridge::GetSubsystem<UDevInputs>(this);
	}
	return CachedDevInputs.Get();
}

UDevMenus* SDevMenuNavigationExtensionWidgetBase::GetDevMenusSubsystem() const
{
	if (!CachedDevMenus.IsValid())
	{
		CachedDevMenus = FDevMenuSlateBridge::GetSubsystem<UDevMenus>(this);
	}
	return CachedDevMenus.Get();
}

void SDevMenuNavigationExtensionWidgetBase::DoOnTick(TFunction<void()> InLambda)
{
	OnTickEvent.AddLambda(MoveTemp(InLambda));
}

void SDevMenuNavigationExtensionWidgetBase::OnTick()
{
	OnTickEvent.Broadcast();
	OnTickEvent.Clear();
}


namespace DevMenu::Navigation
{
	const FLazyName EWidgetTypes::MenuAnchor = TEXT("SMenuAnchor");
	const FLazyName EWidgetTypes::ButtonType = TEXT("SButton");
	const FLazyName EWidgetTypes::SubMenuButtonType = TEXT("SSubMenuButton");

	const TArray<FLazyName> EWidgetTypes::KeyboardFocusBypassWidgetTypes =
	{
		TEXT("SCheckBox"),
		TEXT("SMultiBoxWidget"),
		TEXT("MenuStackInternal::SMenuContentWrapper"),
	};

	const TArray<FLazyName> EWidgetTypes::KeyboardFocusHoveringEffectWidgetsTypes =
	{
		TEXT("SButton"),
		TEXT("SSubMenuButton"),
		TEXT("SMenuEntryButton"),
	};

	FWidgetDetails FWidgetDetails::CreateFromWidget(const TSharedPtr<SWidget>& InWidget)
	{
		FWidgetDetails Details = FWidgetDetails();

		FSlateApplication& SlateApp = FSlateApplication::Get();
		if (InWidget)
		{
			FWidgetPath WidgetPath;
			if (SlateApp.FindPathToWidget(InWidget.ToSharedRef(), WidgetPath); WidgetPath.IsValid())
			{
				Details.Widget = WidgetPath.Widgets.Last();
				Details.WidgetPath = WidgetPath;
			}
		}

		return Details;
	}

	FWidgetDetails FWidgetDetails::CreateFromWidget(const TSharedPtr<SWidget>& InWidget, const TSharedPtr<SWidget>& InParent)
	{
		if (FWidgetDetails Details = CreateFromWidget(InWidget); Details.IsValid() && InParent.IsValid() && Details.WidgetPath.FindArrangedWidget(InParent.ToSharedRef()))
		{
			return Details;
		}
		return FWidgetDetails();
	}


	FMenuWidgetDetails FMenuWidgetDetails::CreateFromWidget(const TSharedPtr<SWidget>& InWidget)
	{
		FMenuWidgetDetails Details = FMenuWidgetDetails();
		if (const FWidgetDetails Proxy = FWidgetDetails::CreateFromWidget(InWidget); Proxy.IsValid())
		{
			FSlateApplication& SlateApp = FSlateApplication::Get();
			if (const TSharedPtr<IMenu> Menu = SlateApp.FindMenuInWidgetPath(Proxy.WidgetPath))
			{
				if (const TSharedPtr<SWidget> MenuWidget = Menu->GetContent())
				{
					if (FWidgetPath MenuWidgetPath; SlateApp.FindPathToWidget(MenuWidget.ToSharedRef(), MenuWidgetPath))
					{
						Details.Widget = MenuWidgetPath.Widgets.Last();
						Details.WidgetPath = MenuWidgetPath;
						Details.WidgetMenu = Menu;
					}
				}
			}
		}
		return Details;
	}

	FMenuWidgetDetails FMenuWidgetDetails::CreateFromWidget(const TSharedPtr<SWidget>& InWidget, const TSharedPtr<SWidget>& InParent)
	{
		if (FMenuWidgetDetails Details = CreateFromWidget(InWidget); Details.IsValid() && InParent.IsValid() && Details.WidgetPath.FindArrangedWidget(InParent.ToSharedRef()))
		{
			return Details;
		}
		return FMenuWidgetDetails();
	}


	TSharedPtr<SMenuAnchor> FNavigationContext::GetMenuAnchorWidget() const
	{
		return StaticCastSharedPtr<SMenuAnchor>(FindNearestWidgetOfType(TargetDetails.Widget.Widget, EWidgetTypes::MenuAnchor));
	}

	ENavigationMenuEntryType FNavigationContext::GetMenuEntryType() const
	{
		if (IsValid())
		{
			const FName WidgetType = TargetDetails.Widget.GetWidgetPtr()->GetType();
			const FName ExpectedWidgetType = (GetMenuType() == ENavigationMenuType::HorizontalMenu) ? EWidgetTypes::ButtonType : EWidgetTypes::SubMenuButtonType;
			if (WidgetType == ExpectedWidgetType)
			{
				return ENavigationMenuEntryType::MenuButton;
			}
		}
		return ENavigationMenuEntryType::Undefined;
	}

	ENavigationMenuType FNavigationContext::GetMenuType() const
	{
		return (IsValid())
			? (SubMenuDetails.IsValid())
				? ENavigationMenuType::VerticalMenu
				: ENavigationMenuType::HorizontalMenu
			: ENavigationMenuType::Undefined;
	}


	TSharedPtr<SWidget> GetKeyboardFocusedChildWidget(const TSharedPtr<SWidget>& InParent)
	{
		const FSlateApplication& SlateApp = FSlateApplication::Get();
		const TSharedPtr<SWidget> FocusedWidget = SlateApp.GetKeyboardFocusedWidget();

		TSharedPtr<SWidget> TempWidget = FocusedWidget;
		while (TempWidget)
		{
			if (TempWidget == InParent) { return FocusedWidget; }
			TempWidget = TempWidget->GetParentWidget();
		}

		return nullptr;
	}

	ENavigationDirection GetNavigationDirection(const FNavigationContext& InContext, const EUINavigation InType)
	{
		if (InContext.GetMenuType() == ENavigationMenuType::HorizontalMenu)
		{
			switch (InType)
			{
			case EUINavigation::Down: return ENavigationDirection::Enter;

			case EUINavigation::Left:
			case EUINavigation::Previous: return ENavigationDirection::Previous;

			case EUINavigation::Right:
			case EUINavigation::Next: return ENavigationDirection::Next;

			case EUINavigation::Up:
			default: return ENavigationDirection::Invalid;
			}
		}
		else // if (InContext.GetMenuType() == ENavigationMenuType::VerticalMenu)
		{
			switch (InType)
			{
			case EUINavigation::Left: return ENavigationDirection::Leave;
			case EUINavigation::Right: return ENavigationDirection::Enter;

			case EUINavigation::Up:
			case EUINavigation::Previous: return ENavigationDirection::Previous;

			case EUINavigation::Down:
			case EUINavigation::Next: return ENavigationDirection::Next;

			default: return ENavigationDirection::Invalid;
			}
		}
	}

	TSharedPtr<SWidget> FindChildWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType)
	{
		if (!InWidget) { return nullptr; }

		FChildren* Children = InWidget->GetChildren();
		if (!Children) { return nullptr; }

		for (int32 i = 0; i < Children->Num(); ++i)
		{
			TSharedRef<SWidget> ChildWidget = Children->GetChildAt(i);
			if (ChildWidget->GetType() == InType)
			{
				return ChildWidget;
			}
			if (TSharedPtr<SWidget> SubChildWidget = FindChildWidgetOfType(ChildWidget, InType))
			{
				return SubChildWidget;
			}
		}

		return nullptr;
	}

	TSharedPtr<SWidget> FindParentWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType)
	{
		if (!InWidget) { return nullptr; }

		TSharedPtr<SWidget> ParentWidget = InWidget->GetParentWidget();
		while (ParentWidget && ParentWidget->GetType() != InType)
		{
			ParentWidget = ParentWidget->GetParentWidget();
		}

		return ParentWidget;
	}

	TSharedPtr<SWidget> FindNearestWidgetOfType(const TSharedPtr<SWidget>& InWidget, const FName InType)
	{
		if (!InWidget) { return nullptr; }

		if (InWidget->GetType() == InType)
		{
			return InWidget;
		}
		if (TSharedPtr<SWidget> ChildWidget = FindChildWidgetOfType(InWidget, InType))
		{
			return ChildWidget;
		}
		if (TSharedPtr<SWidget> ParentWidget = FindParentWidgetOfType(InWidget, InType))
		{
			return ParentWidget;
		}

		return nullptr;
	}
} // namespace DevMenu::Navigation
