// Copyright (c) Alexandr Pereverzev.

#include "DevMenuLogging.h"

#include "DevMenuTypes.h"
#include "InstancedStruct.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"

DEFINE_LOG_CATEGORY(LogDevMenus);

FString DevMenu::Logging::EntryToString(const IDevMenuEntry* InEntry)
{
	if (!InEntry) { return TEXT("<null>"); }

	return (InEntry->GetEntryName().IsNone())
		? FString::Printf(TEXT("%s()"), *GetNameSafe(InEntry->GetEntryType()))
		: FString::Printf(TEXT("%s(Name = %s)"), *GetNameSafe(InEntry->GetEntryType()), *InEntry->GetEntryName().ToString());
}

FString DevMenu::Logging::EntryToString(const FInstancedStruct& InEntry)
{
	return EntryToString((InEntry.IsValid()) ? &InEntry.Get<FDevMenuEntry>() : nullptr);
}

FString DevMenu::Logging::UserToString(const int32 InUser)
{
	const FSlateApplication& SlateApp = FSlateApplication::Get();
	if (InUser == SlateApp.GetUserIndexForKeyboard()) { return FString::Printf(TEXT("Keyboard:%lu"), InUser); }
	if (InUser == SlateApp.GetUserIndexForMouse()) { return FString::Printf(TEXT("Mouse:%lu"), InUser); }
	return FString::Printf(TEXT("User:%lu"), InUser);
}

FString DevMenu::Logging::WidgetToString(const SWidget* InWidget)
{
	if (!InWidget) { return TEXT("<null>"); }
	TStringBuilder<64> SB;
	InWidget->GetType().AppendString(SB);
#if UE_SLATE_WITH_WIDGET_UNIQUE_IDENTIFIER
	SB.Appendf(TEXT(":%llu"), InWidget->GetId());
#endif
	return SB.ToString();
}

FString DevMenu::Logging::WidgetToString(const TSharedPtr<SWidget>& InWidget)
{
	return WidgetToString(InWidget.Get());
}

FString DevMenu::Logging::WidgetToString(const TWeakPtr<SWidget>& InWidget)
{
	return (InWidget.IsValid()) ? WidgetToString(InWidget.Pin()) : TEXT("<expired>");
}

FString DevMenu::Logging::WidgetPathToString(const SWidget* InWidget, const SWidget* InRelativeToWidget)
{
	if (!InWidget) { return TEXT("<null>"); }
	if (!FSlateApplication::IsInitialized()) { return TEXT("<uninitialized>"); }

	FWidgetPath WidgetPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(InWidget->AsShared(), WidgetPath);

	return WidgetPathToString(WidgetPath, InRelativeToWidget);
}

FString DevMenu::Logging::WidgetPathToString(const TSharedPtr<SWidget>& InWidget, const SWidget* InRelativeToWidget)
{
	return WidgetPathToString(InWidget.Get(), InRelativeToWidget);
}

FString DevMenu::Logging::WidgetPathToString(const TWeakPtr<SWidget>& InWidget, const SWidget* InRelativeToWidget)
{
	const TSharedPtr<SWidget>& Widget = (InWidget.IsValid()) ? InWidget.Pin() : nullptr;
	return (Widget.IsValid()) ? WidgetPathToString(Widget, InRelativeToWidget) : TEXT("<expired>");
}

FString DevMenu::Logging::WidgetPathToString(const FWidgetPath& InWidgetPath, const SWidget* InRelativeToWidget)
{
	if (!InWidgetPath.IsValid()) { return TEXT("<empty>"); }

	const SWidget* RelativeToWidget = InRelativeToWidget;
	bool bRelativeWidgetFoundOrMissing = (!RelativeToWidget) || (!InWidgetPath.ContainsWidget(RelativeToWidget));

	TStringBuilder<1024> SB;
	if (!bRelativeWidgetFoundOrMissing) { SB.Append("."); }
	for (const FArrangedWidget& ArrangedWidget : InWidgetPath.Widgets.GetInternalArray())
	{
		const SWidget* Widget = ArrangedWidget.GetWidgetPtr();
		bRelativeWidgetFoundOrMissing |= (Widget == RelativeToWidget);
		if (bRelativeWidgetFoundOrMissing)
		{
			if (SB.Len()) { SB.Append(" "); }
			SB.Append("/ ");
			Widget->GetType().AppendString(SB);
#if UE_SLATE_WITH_WIDGET_UNIQUE_IDENTIFIER
			SB.Appendf(TEXT(":%llu"), Widget->GetId());
#endif
			if (Widget->SupportsKeyboardFocus()) { SB.Append(" [F]"); }
		}
	}
	return SB.ToString();
}

FString DevMenu::Logging::WidgetPathToString(const FWeakWidgetPath& InWidgetPath, const SWidget* InRelativeToWidget)
{
	if (!InWidgetPath.IsValid()) { return TEXT("<empty>"); }

	const SWidget* RelativeToWidget = InRelativeToWidget;
	bool bRelativeWidgetFoundOrMissing = (!RelativeToWidget) || (!InWidgetPath.ContainsWidget(RelativeToWidget));

	TStringBuilder<1024> SB;
	if (!bRelativeWidgetFoundOrMissing) { SB.Append("."); }
	for (const TWeakPtr<SWidget>& WeakWidget : InWidgetPath.Widgets)
	{
		if (const SWidget* Widget = WeakWidget.Pin().Get())
		{
			bRelativeWidgetFoundOrMissing |= (Widget == RelativeToWidget);
			if (bRelativeWidgetFoundOrMissing)
			{
				if (SB.Len()) { SB.Append(" "); }
				SB.Append("/ ");
				Widget->GetType().AppendString(SB);
#if UE_SLATE_WITH_WIDGET_UNIQUE_IDENTIFIER
				SB.Appendf(TEXT(":%llu"), Widget->GetId());
#endif
				if (Widget->SupportsKeyboardFocus()) { SB.Append(" [F]"); }
			}
		}
		else
		{
			if (SB.Len()) { SB.Append(" "); }
			SB.Append("/ <expired> ...");
			break;
		}
	}
	return SB.ToString();
}
