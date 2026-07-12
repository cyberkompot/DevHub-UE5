// Copyright (c) Alexandr Pereverzev.

#include "DevMenuTypes.h"

#include "DevCore.h"
#include UE_COMPATIBILITY_INCLUDE_STRUCT_VIEW_PATH

#include "DevActionTypes.h"
#include "DevMenuFactory.h"
#include "DevMenuIterators.h"
#include "DevMenuLogging.h"
#include "DevMenuUtils.h"
#include "Engine/World.h"
#include "UObject/ObjectSaveContext.h"

namespace DevMenus::Core
{
	struct FDynamicEntriesCachesInvalidator final
	{
		FORCEINLINE static void RegisterCache(FDevMenuDynamicEntriesCache* InCache) { Get().Caches.Add(InCache); }
		FORCEINLINE static void UnregisterCache(const FDevMenuDynamicEntriesCache* InCache) { Get().Caches.Remove(InCache); }

	private:
		FDynamicEntriesCachesInvalidator()
		{
			FWorldDelegates::OnWorldBeginTearDown.AddRaw(this, &FDynamicEntriesCachesInvalidator::OnWorldBeginTearDown);
		}
		~FDynamicEntriesCachesInvalidator()
		{
			FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);
		}

		static FDynamicEntriesCachesInvalidator& Get()
		{
			static FDynamicEntriesCachesInvalidator Singleton = FDynamicEntriesCachesInvalidator();
			return Singleton;
		}

		TSet<FDevMenuDynamicEntriesCache*> Caches;

		void ResetCaches()
		{
			const TSet<FDevMenuDynamicEntriesCache*> CachesCopy = Caches; Caches.Reset();
			for (FDevMenuDynamicEntriesCache* Cache : CachesCopy)
			{
				Cache->Reset();
			}
		}

		void OnWorldBeginTearDown(UWorld* InWorld)
		{
			ResetCaches();
		}
	};
}

using namespace DevMenus::Core;


FName IDevMenuEntry::GetEntryName() const
{
	return FName();
}

EDevMenuEntryFlags IDevMenuEntry::GetEntryFlags() const
{
	return EDevMenuEntryFlags::NoFlags;
}

UStruct* IDevMenuEntry::GetEntryType() const
{
	return nullptr;
}

TAttribute<FText> IDevMenuEntry::GetLabel() const
{
	return TAttribute<FText>();
}

TAttribute<FText> IDevMenuEntry::GetToolTip() const
{
	return TAttribute<FText>();
}

TAttribute<FDevInputShortcut> IDevMenuEntry::GetInputShortcut() const
{
	return TAttribute<FDevInputShortcut>();
}

ECheckBoxState IDevMenuEntry::GetCheckState(const UObject* WorldContextObject) const
{
	return ECheckBoxState::Unchecked;
}

bool IDevMenuEntry::IsEnabled(const UObject* WorldContextObject) const
{
	return true;
}

bool IDevMenuEntry::IsVisible(const UObject* WorldContextObject) const
{
	return true;
}

FDevMenuInstancedEntries* IDevMenuEntry::GetSubEntries()
{
	return nullptr;
}

void IDevMenuEntry::QuerySubEntries(const IDevMenuEntriesQuery& InQuery)
{
	/** Nop */
}

void IDevMenuEntry::ExecuteEntry(const UObject* WorldContextObject)
{
	/** Nop */
}

void IDevMenuEntry::SetEntryName(const FName InName)
{
	/** Nop */
}

void IDevMenuEntry::ResolveEntryPath()
{
	/** Nop */
}

void IDevMenuEntry::PopulateMenuBuilder(IDevMenuBuilderContext& InContext)
{
	/** Nop */
}

FDevMenuInstancedEntry IDevMenuEntry::DuplicateEntry() const
{
	const UStruct* EntryType = GetEntryType();
	if (const UScriptStruct* ScriptStruct = Cast<UScriptStruct>(EntryType); ScriptStruct && ScriptStruct->IsChildOf(FDevMenuEntry::StaticStruct()))
	{
		return FDevMenuInstancedEntry(FConstStructView(ScriptStruct, reinterpret_cast<const uint8*>(static_cast<const FDevMenuEntry*>(this))));
	}
	if (const UClass* Class = Cast<UClass>(EntryType); Class && Class->IsChildOf(UDevMenu::StaticClass()))
	{
		UDevMenu* Menu = const_cast<UDevMenu*>(static_cast<const UDevMenu*>(this));
		return FDevMenuInstancedEntry::Make(FDevMenuFactory::CreateProxy(Menu->GetEntryName(), Menu));
	}
	return FDevMenuInstancedEntry();
}


const FDevMenuEntryId FDevMenuEntryId::Empty = FDevMenuEntryId();

FDevMenuEntryId FDevMenuEntryId::NewEntryId()
{
	// A signed 32-bit integer can safely store seconds for up to approximately 68 years.
	// The total number of seconds in 32 years (ignoring leap years) is 1,009,152,000,
	// which is well within the positive range of a signed 32-bit integer.
	constexpr int32 MaxSeconds = 32 * 366 * 24 * 60 * 60;
	constexpr int32 MinId = 0x40000000; // To avoid zero IDs and enforce 10 digits long IDs.
	static int64 LastTimestamp = 0;
	const int64 Timestamp = LastTimestamp = FMath::Max(FDateTime::UtcNow().ToUnixTimestamp(), LastTimestamp + 1); // +1 to ensure unique IDs within the same second.
	const int32 Id = MinId | (Timestamp % MaxSeconds);
	return FDevMenuEntryId(Id);
}


const FDevMenuQueryResult FDevMenuQueryResult::Empty = FDevMenuQueryResult();


void IDevMenuBuilderContext::PopulateMenuBuilder(IDevMenuEntriesIterator& InEntriesIterator)
{
	for (/** Nop */; InEntriesIterator; ++InEntriesIterator)
	{
		if (IDevMenuEntry* Entry = *InEntriesIterator)
		{
			Entry->PopulateMenuBuilder(*this);
		}
	}
}

FCanExecuteAction IDevMenuBuilderContext::CreateGetEntryEnabledStateDelegate(const IDevMenuEntry& InEntry)
{
	const TWeakObjectPtr<const UObject> WorldContextObjectPtr = GetWorldContextObject();
	return FCanExecuteAction::CreateWeakLambda(&GetGeneratedMenu(), [WorldContextObjectPtr, &InEntry]
	{
		return InEntry.IsEnabled(WorldContextObjectPtr.Get());
	});
}

FGetActionCheckState IDevMenuBuilderContext::CreateGetEntryCheckStateDelegate(const IDevMenuEntry& InEntry) const
{
	return GetGeneratedMenu().CreateGetEntryCheckStateDelegate(GetWorldContextObject(), InEntry);
}

FIsActionButtonVisible IDevMenuBuilderContext::CreateGetEntryVisibilityDelegate(const IDevMenuEntry& InEntry) const
{
	return GetGeneratedMenu().CreateGetEntryVisibilityDelegate(GetWorldContextObject(), InEntry);
}

FExecuteAction IDevMenuBuilderContext::CreateExecuteEntryDelegate(IDevMenuEntry& InEntry) const
{
	return GetGeneratedMenu().CreateExecuteEntryDelegate(GetWorldContextObject(), InEntry);
}



FDevMenuDynamicEntriesCache::~FDevMenuDynamicEntriesCache()
{
	Reset();
}

FDevMenuInstancedEntries& FDevMenuDynamicEntriesCache::Get()
{
	UE_CLOG_FUNCTION(!bIsSet, LogDevMenus, Error, TEXT("Dynamic entries cache requested, while no cache is set"));
	return DynamicEntries;
}

void FDevMenuDynamicEntriesCache::Set(const FDevMenuInstancedEntries& InDynamicEntries, const uint32 InValidationHash)
{
	const bool bWasRegistered = (bIsSet && !DynamicEntries.IsEmpty());

	DynamicEntries = InDynamicEntries;
	ValidationHash = InValidationHash;

	const bool bShouldBeRegistered = (!DynamicEntries.IsEmpty());
	if (bShouldBeRegistered != bWasRegistered)
	{
		if (bShouldBeRegistered)
		{
			FDynamicEntriesCachesInvalidator::RegisterCache(this);
		}
		else
		{
			FDynamicEntriesCachesInvalidator::UnregisterCache(this);
		}
	}
}

void FDevMenuDynamicEntriesCache::Set(FDevMenuInstancedEntries&& InDynamicEntries, const uint32 InValidationHash)
{
	const bool bWasRegistered = (bIsSet && !DynamicEntries.IsEmpty());

	bIsSet = true;
	DynamicEntries = Forward<FDevMenuInstancedEntries>(InDynamicEntries);
	ValidationHash = InValidationHash;

	const bool bShouldBeRegistered = (!DynamicEntries.IsEmpty());
	if (bShouldBeRegistered != bWasRegistered)
	{
		if (bShouldBeRegistered)
		{
			FDynamicEntriesCachesInvalidator::RegisterCache(this);
		}
		else
		{
			FDynamicEntriesCachesInvalidator::UnregisterCache(this);
		}
	}
}

void FDevMenuDynamicEntriesCache::Reset()
{
	if (bIsSet)
	{
		if (!DynamicEntries.IsEmpty())
		{
			FDynamicEntriesCachesInvalidator::UnregisterCache(this);
		}
		DynamicEntries.Reset();
		ValidationHash = 0;
		bIsSet = false;
	}
}

IDevMenuEntry* FDevMenuProxy::GetProxyEntry() const
{
	return Menu.Get();
}


TAttribute<FText> FDevMenuItemBase::GetLabel() const
{
	return (Label.IsEmpty()) ? TAttribute<FText>(FDevMenuUtils::EntryPathToDisplayText(EntryName)) : TAttribute<FText>(Label);
}

void FDevMenuExecutionBase::PopulateMenuBuilder(IDevMenuBuilderContext& InContext)
{
	FMenuEntryParams EntryParams;
	EntryParams.ExtensionHook = GetEntryName();
	EntryParams.Type = EMultiBlockType::MenuEntry;
	EntryParams.LabelOverride = GetLabel();
	EntryParams.ToolTipOverride = GetToolTip();
	EntryParams.InputBindingOverride = GetInputShortcut().Get().ToText();
	EntryParams.UserInterfaceActionType = GetUserInterfaceType(InContext.GetWorldContextObject());
	EntryParams.DirectActions.ExecuteAction = InContext.CreateExecuteEntryDelegate(*this);
	EntryParams.DirectActions.CanExecuteAction = InContext.CreateGetEntryEnabledStateDelegate(*this);
	EntryParams.DirectActions.GetActionCheckState = InContext.CreateGetEntryCheckStateDelegate(*this);
	EntryParams.DirectActions.IsActionVisibleDelegate = InContext.CreateGetEntryVisibilityDelegate(*this);
	InContext.GetMenuBuilder().AddMenuEntry(EntryParams);
}

void FDevMenuStaticOuterBase::ResolveEntryPath()
{
	Super::ResolveEntryPath();
	for (FDevMenuInstancedEntry& InstancedEntry : Entries)
	{
		if (FDevMenuEntry* Entry = InstancedEntry.GetMutablePtr<FDevMenuEntry>(); Entry)
		{
			Entry->ResolveEntryPath();
		}
	}
}

void FDevMenuStaticOuterBase::QuerySubEntries(const IDevMenuEntriesQuery& InQuery)
{
	FDevMenuIteratorOverEntries EntriesIterator(Entries);
	InQuery.ExecuteQuery(EntriesIterator);
}


FDevMenuInstancedEntries& FDevMenuDynamicOuterBase::GetDynamicEntries()
{
	if (GetDynamicEntriesCachingPolicy() == EDevMenuDynamicEntriesCachePolicy::CacheAllowed)
	{
		if (!DynamicEntriesCache.IsValid(GetDynamicEntriesValidationHash()))
		{
			DynamicEntriesCache.Set(CreateDynamicEntries());
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Error, TEXT("Dynamic entries requested while caching is disabled: Entry = %s, Type = %s"), *GetEntryName().ToString(), *GetNameSafe(GetEntryType()));
		DynamicEntriesCache.Set(FDevMenuInstancedEntries()); // Fallback to avoid crashing.
	}
	return DynamicEntriesCache.Get();
}

void FDevMenuDynamicOuterBase::QuerySubEntries(const IDevMenuEntriesQuery& InQuery)
{
	if (GetDynamicEntriesCachingPolicy() == EDevMenuDynamicEntriesCachePolicy::CacheAllowed)
	{
		FDevMenuIteratorOverEntries EntriesIterator(GetDynamicEntries());
		InQuery.ExecuteQuery(EntriesIterator);
	}
	else
	{
		FDevMenuIteratorOverDynamicEntries EntriesIterator(CreateDynamicEntries());
		InQuery.ExecuteQuery(EntriesIterator);
	}
}


ECheckBoxState FDevMenuActionButton::GetCheckState(const UObject* WorldContextObject) const
{
	return (Action.IsValid()) ? Action.Get<FDevAction>().GetActionCheckState(WorldContextObject) : ECheckBoxState::Unchecked;
}

bool FDevMenuActionButton::IsVisible(const UObject* WorldContextObject) const
{
	return (Action.IsValid()) ? Action.Get<FDevAction>().GetActionVisibility(WorldContextObject) : true;
}

void FDevMenuActionButton::ExecuteEntry(const UObject* WorldContextObject)
{
	if (Action.IsValid())
	{
		Action.Get<FDevAction>().ExecuteAction(WorldContextObject);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Action is not defined"));
	}
}

EUserInterfaceActionType FDevMenuActionButton::GetUserInterfaceType(const UObject* WorldContextObject) const
{
	return (Action.IsValid()) ? Action.Get<FDevAction>().GetActionUserInterfaceType(WorldContextObject) : EUserInterfaceActionType::Button;
}


const FPrimaryAssetType UDevMenu::PrimaryAssetType(TEXT("DevMenu"));

UDevMenu* UDevMenu::CreateMenu(const FName InMenuPath, UObject* InOuter, const FName InBaseName, void (*OnConstruct)(UDevMenu*& Menu))
{
	const FName UniqueObjectName = MakeUniqueObjectName(InOuter, StaticClass(), InBaseName);
	UDevMenu* NewMenu = NewObject<UDevMenu>(InOuter, UniqueObjectName);
	NewMenu->MenuPath = InMenuPath;
	if (OnConstruct) { OnConstruct(NewMenu); }
	UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Dev Menu created: %s → %s"), *GetNameSafe(NewMenu), *InMenuPath.ToString());
	return NewMenu;
}

void UDevMenu::BeginDestroy()
{
	UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Dev Menu destroyed: %s → %s"), *GetNameSafe(this), *MenuPath.ToString());
	Super::BeginDestroy();
}

void UDevMenu::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	ResolveEntryPath();
}

#if WITH_EDITOR

void UDevMenu::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ResolveEntryPath();
}

#endif // WITH_EDITOR

void UDevMenu::ResolveEntryPath()
{
	if (MenuPath.IsNone())
	{
		MenuPath = FDevMenuEntryId::NewEntryId().ToName();
	}
	for (FDevMenuInstancedEntry& InstancedEntry : Entries)
	{
		if (FDevMenuEntry* Entry = InstancedEntry.GetMutablePtr<FDevMenuEntry>(); Entry)
		{
			Entry->ResolveEntryPath();
		}
	}
}

FPrimaryAssetId UDevMenu::GetPrimaryAssetId() const
{
	// Skip CDO and transient objects.
	if (HasAnyFlags(RF_ClassDefaultObject | RF_Transient))
	{
		return FPrimaryAssetId();
	}

	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

TAttribute<FText> UDevMenu::GetLabel() const
{
	return TAttribute<FText>(FDevMenuUtils::EntryPathToDisplayText(MenuPath));
}

void UDevMenu::PopulateMenuBuilder(IDevMenuBuilderContext& InContext)
{
	FDevMenuIteratorOverEntries EntriesIterator(Entries);
	InContext.PopulateMenuBuilder(EntriesIterator);
}

void UDevMenu::QuerySubEntries(const IDevMenuEntriesQuery& InQuery)
{
	FDevMenuIteratorOverEntries EntriesIterator(Entries);
	InQuery.ExecuteQuery(EntriesIterator);
}
