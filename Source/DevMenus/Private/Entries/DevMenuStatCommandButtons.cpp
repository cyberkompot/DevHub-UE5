// Copyright (c) Alexandr Pereverzev.

#include "Entries/DevMenuStatCommandButtons.h"

#include "DevMenuFactory.h"
#include "DevMenuLogging.h"
#include "DevMenuUtils.h"
#include "Actions/DevActionConsoleCommand.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Stats/StatsData.h"

FDevMenuInstancedEntries FDevMenuStatCommandGroup::CreateDynamicEntries() const
{
	FDevMenuInstancedEntries DynamicEntries;

#if STATS
	DynamicEntries.Reserve(StatNames.Num());

	// For reference see UEngine::IsEngineStat(), UConsole::BuildRuntimeAutoCompleteList(), and UE::LevelEditor::CreateShowStatsSubmenu().
	const TSet<FName>& StatGroupNames = FStatGroupGameThreadNotifier::Get().StatGroupNames;
	for (const FString& StatName : StatNames)
	{
		if (GEngine->IsEngineStat(StatName)
			|| StatGroupNames.Contains(FName("STAT_" + StatName))
			|| StatGroupNames.Contains(FName("STATGROUP_" + StatName)))
			// Add search in UConsole::AutoCompleteList, since UEngine::IsEngineStat() performs a case-sensitive check.
		{
			DynamicEntries.Emplace(FDevMenuFactory::CreateActionButton<FDevActionConsoleCommand>(
				FName(StatName),
				FDevMenuUtils::EntryNameToDisplayText(StatName),
				FDevActionConsoleCommand(TEXT("Stat"), StatName)));
		}
		else
		{
			UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Stat not found: Entity = %s, Stat = %s"), *GetEntryName().ToString(), *StatName);
		}
	}
#endif // STATS

	return DynamicEntries;
}
