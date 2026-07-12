// Copyright (c) Alexandr Pereverzev.

#include "DevMenuGenerator.h"

#include "DevMenuLayoutEntries.h"
#include "DevMenuLogging.h"
#include "DevMenuQueries.h"
#include "DevMenuRegistry.h"

using namespace DevMenu::Logging;

const FName UDevMenuGenerator::GeneratedMenuName("GeneratedMenuName");

UDevMenu* UDevMenuGenerator::GenerateMenu(const FName InMenuPath)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Generating menu: Path = %s"), *InMenuPath.ToString());

	UDevMenu* GeneratedMenu = UDevMenu::CreateMenu(InMenuPath, this, GeneratedMenuName);
	GeneratedMenu->MenuType = EDevMenuType::MenuBar;
	PopulateMenuEntries(*GeneratedMenu);

	return GeneratedMenu;
}

void UDevMenuGenerator::PopulateMenuEntries(UDevMenu& InGeneratedMenu) const
{
	TFunction<void(const FDevMenuQueryResult&, const FDevMenuLayoutFactory::FLayoutContext&)> MainDelegate;
	MainDelegate = [this, &InGeneratedMenu, &MainDelegate](const FDevMenuQueryResult& InQueryResult, const FDevMenuLayoutFactory::FLayoutContext& InLayoutContext)
	{
		if (IDevMenuEntry& Entry = *InQueryResult.Entry; Entry.HasAnyEntryFlags(FDevMenuLayoutFactory::LayoutFlags))
		{
			// Beginning layout entry.
			FDevMenuInstancedEntry GeneratedBeginningEntry;
			if (GeneratedBeginningEntry = FDevMenuLayoutFactory::CreateInstancedLayoutBeginEntryIfNeeded(Entry, InLayoutContext); GeneratedBeginningEntry.IsValid())
			{
				UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Entry = %s"), *EntryToString(GeneratedBeginningEntry));
				InGeneratedMenu.GetSubEntries()->Emplace(MoveTemp(GeneratedBeginningEntry));
			}

			// Nested entries.
			if (Entry.HasAnyEntryFlags(EDevMenuEntryFlags::Section | EDevMenuEntryFlags::Embedded))
			{
				IDevMenuEntry& NestedOuter = (GeneratedBeginningEntry.IsValid())
					? GeneratedBeginningEntry.GetMutable<FDevMenuEntry>()
					: InLayoutContext.Outer;
				const FName& NestedEmbeddedPath = (GeneratedBeginningEntry.IsValid())
					? NAME_None
					: Entry.GetEntryName();
				const FDevMenuLayoutFactory::FLayoutContext NestedLayoutContext(InLayoutContext.Menu, NestedOuter, NestedEmbeddedPath);
				const FDevMenuGetEntriesByPathQuery SubEntriesQuery(InQueryResult.Path, EDevMenuQueryDepth::FirstLevelSubEntries, FDevMenuQueryDelegate::CreateLambda([&MainDelegate](const FDevMenuQueryResult& InNestedQueryResult, const FDevMenuLayoutFactory::FLayoutContext& InNestedLayoutContext)
				{
					MainDelegate(InNestedQueryResult, InNestedLayoutContext);
					return EDevMenuQueryExecution::Continue;
				}, NestedLayoutContext));
				MenuRegistry->QueryEntries(SubEntriesQuery);
			}

			// Ending layout entry.
			if (FDevMenuInstancedEntry GeneratedEndingEntry = FDevMenuLayoutFactory::CreateInstancedLayoutEndEntryIfNeeded(Entry, InLayoutContext); GeneratedEndingEntry.IsValid())
			{
				UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Entry = %s"), *EntryToString(GeneratedEndingEntry));
				InGeneratedMenu.GetSubEntries()->Emplace(MoveTemp(GeneratedEndingEntry));
			}
		}
		else
		{
			// Non layout entry.
			FDevMenuInstancedEntry GeneratedEntry = FDevMenuLayoutFactory::DuplicateLayoutEntry(Entry, InLayoutContext);
			UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Entry = %s"), *EntryToString(GeneratedEntry));
			InGeneratedMenu.GetSubEntries()->Emplace(MoveTemp(GeneratedEntry));
		}
	};

	thread_local TSet<FName> SectionsAndSubMenus = TSet<FName>();
	ON_SCOPE_EXIT { SectionsAndSubMenus.Reset(); };

	const FDevMenuLayoutFactory::FLayoutContext LayoutContext(InGeneratedMenu);
	const FDevMenuGetEntriesByPathQuery EntriesQuery(InGeneratedMenu.GetEntryName(), EDevMenuQueryDepth::FirstLevelSubEntries, FDevMenuQueryDelegate::CreateLambda([&MainDelegate](const FDevMenuQueryResult& InQueryResult, const FDevMenuLayoutFactory::FLayoutContext& InLayoutContext)
	{
		bool bIsSectionOrSubMenuAlreadyPresent = false;
		if (InQueryResult.Entry->HasAnyEntryFlags(EDevMenuEntryFlags::Section | EDevMenuEntryFlags::SubMenu))
		{
			SectionsAndSubMenus.Emplace(InQueryResult.Entry->GetEntryName(), &bIsSectionOrSubMenuAlreadyPresent);
		}
		if (!bIsSectionOrSubMenuAlreadyPresent)
		{
			MainDelegate(InQueryResult, InLayoutContext);
		}
		return EDevMenuQueryExecution::Continue;
	}, LayoutContext));
	MenuRegistry->QueryEntries(EntriesQuery);
}
