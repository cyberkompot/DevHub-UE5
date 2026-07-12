// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreTypeTraits.h"
#include "DevMenuTypes.h"
#include "Templates/Decay.h"
#include "Templates/EnableIf.h"
#include "Templates/UnrealTypeTraits.h"

/** Dev Menu entries factory. Implemented as a namespace to allow usage via `using namespace FDevMenuFactory;`. */
namespace FDevMenuFactory
{
	template<typename TEntryType,
			 typename = typename TEnableIf<
			 	TIsDerivedFrom<typename TDecay<TEntryType>::Type, FDevMenuEntry>::Value>>
	static TEntryType CreateEntry(const TFunction<void(TEntryType&)>& OnConstruct = nullptr)
	{
		using T = typename TDecay<TEntryType>::Type;
		T Entry{};
		if (OnConstruct) { OnConstruct(Entry); }
		return Forward<T>(Entry);
	}

	template<typename TEntryType,
			 typename = typename TEnableIf<
			 	TIsDerivedFrom<typename TDecay<TEntryType>::Type, FDevMenuEntry>::Value &&
				TIsMemberPointer<decltype(&TEntryType::EntryName)>::Value>::Type>
	static TEntryType CreateEntry(const FName InEntryName, const TFunction<void(TEntryType&)>& OnConstruct = nullptr)
	{
		TEntryType Entry{};
		Entry.EntryName = InEntryName;
		if (OnConstruct) { OnConstruct(Entry); }
		return Entry;
	}

	template<typename TEntryType,
			 typename = typename TEnableIf<
			 	TIsDerivedFrom<typename TDecay<TEntryType>::Type, FDevMenuEntry>::Value &&
				TIsMemberPointer<decltype(&TEntryType::EntryName)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::Label)>::Value>::Type>
	static TEntryType CreateEntry(const FName InEntryName, const FText& InLabel, const TFunction<void(TEntryType&)>& OnConstruct = nullptr)
	{
		TEntryType Entry{};
		Entry.EntryName = InEntryName;
		Entry.Label = InLabel;
		if (OnConstruct) { OnConstruct(Entry); }
		return Entry;
	}

	template<typename TEntryType,
			 typename = typename TEnableIf<
			 	TIsDerivedFrom<typename TDecay<TEntryType>::Type, FDevMenuEntry>::Value &&
				TIsMemberPointer<decltype(&TEntryType::EntryName)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::Label)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::ToolTip)>::Value>::Type>
	static TEntryType CreateEntry(const FName InEntryName, const FText& InLabel, const FText& InToolTip, const TFunction<void(TEntryType&)>& OnConstruct = nullptr)
	{
		TEntryType Entry{};
		Entry.EntryName = InEntryName;
		Entry.Label = InLabel;
		Entry.ToolTip = InToolTip;
		if (OnConstruct) { OnConstruct(Entry); }
		return Entry;
	}

	template<typename TEntryType,
			 typename = typename TEnableIf<
			 	TIsDerivedFrom<typename TDecay<TEntryType>::Type, FDevMenuEntry>::Value &&
				TIsMemberPointer<decltype(&TEntryType::EntryName)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::Label)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::ToolTip)>::Value &&
				TIsMemberPointer<decltype(&TEntryType::InputShortcut)>::Value>::Type>
	static TEntryType CreateEntry(const FName InEntryName, const FText& InLabel, const FText& InToolTip, FDevInputShortcut InInputShortcut, const TFunction<void(TEntryType&)>& OnConstruct = nullptr)
	{
		TEntryType Entry{};
		Entry.EntryName = InEntryName;
		Entry.Label = InLabel;
		Entry.ToolTip = InToolTip;
		Entry.InputShortcut = InInputShortcut;
		if (OnConstruct) { OnConstruct(Entry); }
		return Entry;
	}


	FORCEINLINE static FDevMenuSeparator CreateSeparator()
	{
		return FDevMenuSeparator();
	};

	FORCEINLINE static FDevMenuSection CreateSection(const FName InEntryName, const FText& InLabel = FText::GetEmpty(), const FText& InToolTip = FText::GetEmpty())
	{
		return CreateEntry<FDevMenuSection>(InEntryName, InLabel, InToolTip);;
	};

	FORCEINLINE static FDevMenuSubMenu CreateSubMenu(const FName InEntryName, const FText& InLabel = FText::GetEmpty(), const FText& InToolTip = FText::GetEmpty())
	{
		return CreateEntry<FDevMenuSubMenu>(InEntryName, InLabel, InToolTip);
	};

	FORCEINLINE static FDevMenuProxy CreateProxy(const FName InEntryName, UDevMenu* InMenu)
	{
		FDevMenuProxy Entry = CreateEntry<FDevMenuProxy>();
		Entry.EntryName = InEntryName;
		Entry.Menu = InMenu;
		return Entry;
	};

	template<typename TActionType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TActionType>::Type, FDevAction>::Value>::Type>
	static FDevMenuActionButton CreateActionButton(const FName InEntryName, const FText& InLabel, const FText& InToolTip, const FDevInputShortcut& InInputShortcut, const TActionType& Action)
	{
		FDevMenuActionButton Entry = CreateEntry<FDevMenuActionButton>(InEntryName, InLabel, InToolTip, InInputShortcut);
		Entry.Action = FDevActionInstancedAction::Make<TActionType>(Action);
		return Entry;
	};

	template<typename TActionType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TActionType>::Type, FDevAction>::Value>::Type>
	static FDevMenuActionButton CreateActionButton(const FName InEntryName, const FText& InLabel, const FText& InToolTip, const TActionType& Action)
	{
		return CreateActionButton<TActionType>(InEntryName, InLabel, InToolTip, FDevInputShortcut::GetEmpty(), Action);
	};

	template<typename TActionType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TActionType>::Type, FDevAction>::Value>::Type>
	static FDevMenuActionButton CreateActionButton(const FName InEntryName, const FText& InLabel, const TActionType& Action)
	{
		return CreateActionButton<TActionType>(InEntryName, InLabel, FText::GetEmpty(), FDevInputShortcut::GetEmpty(), Action);
	};

	template<typename TActionType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TActionType>::Type, FDevAction>::Value>::Type>
	static FDevMenuActionButton CreateActionButton(const FName InEntryName, const TActionType& Action)
	{
		return CreateActionButton<TActionType>(InEntryName, FText::GetEmpty(), FText::GetEmpty(), FDevInputShortcut::GetEmpty(), Action);
	};


	FORCEINLINE static UDevMenu* CreateMenu(const FName InMenuPath, UObject* InOuter, const FName InBaseName = NAME_None, const TFunction<void(UDevMenu*&)>& OnConstruct = nullptr)
	{
		UDevMenu* Menu = UDevMenu::CreateMenu(InMenuPath, InOuter, InBaseName, nullptr);
		if (OnConstruct) { OnConstruct(Menu); }
		return Menu;
	}

};
