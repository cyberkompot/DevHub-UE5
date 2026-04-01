// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h" // Included for log macros.
#include "Templates/SharedPointer.h"
#include "UObject/ReflectedTypeAccessors.h"

struct IDevMenuEntry;
struct FDevMenuEntry;
struct FInstancedStruct;
class FString;
class SWidget;
class FWeakWidgetPath;
class FWidgetPath;

DEVMENUS_API DECLARE_LOG_CATEGORY_EXTERN(LogDevMenus, VeryVerbose, All);

namespace DevMenu::Logging
{
	FString EntryToString(const IDevMenuEntry* InEntry);
	FString EntryToString(const FInstancedStruct& InEntry);

	template<typename EnumType>
	FString EnumToString(EnumType InEnumValue) { return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue)); }

	FString UserToString(const int32 InUser);

	FString WidgetToString(const SWidget* InWidget);
	FString WidgetToString(const TSharedPtr<SWidget>& InWidget);
	FString WidgetToString(const TWeakPtr<SWidget>& InWidget);

	FString WidgetPathToString(const SWidget* InWidget, const SWidget* InRelativeToWidget = nullptr);
	FString WidgetPathToString(const TSharedPtr<SWidget>& InWidget, const SWidget* InRelativeToWidget = nullptr);
	FString WidgetPathToString(const TWeakPtr<SWidget>& InWidget, const SWidget* InRelativeToWidget = nullptr);

	FString WidgetPathToString(const FWidgetPath& InWidgetPath, const SWidget* InRelativeToWidget = nullptr);
	FString WidgetPathToString(const FWeakWidgetPath& InWidgetPath, const SWidget* InRelativeToWidget = nullptr);
} // namespace DevMenu::Logging
