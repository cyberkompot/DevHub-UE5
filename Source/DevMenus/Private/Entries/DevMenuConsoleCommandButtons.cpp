// Copyright (c) Alexandr Pereverzev.

#include "Entries/DevMenuConsoleCommandButtons.h"

#include "DevMenuFactory.h"
#include "DevMenuLogging.h"
#include "DevMenuUtils.h"
#include "Actions/DevActionConsoleCommand.h"

FDevMenuInstancedEntries FDevMenuConsoleCommandGroup::CreateDynamicEntries() const
{
	FDevMenuInstancedEntries DynamicEntries;

	if (Command.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Command is not defined"));
		return DynamicEntries;
	}

#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	DynamicEntries.Reserve(CommandArgumentsInfos.Num());

	for (const FDevMenuConsoleCommandArgumentInfo& CommandArgumentsInfo : CommandArgumentsInfos)
	{
		const FName DynamicEntryName = (!CommandArgumentsInfo.Argument.IsEmpty()) ? FName(CommandArgumentsInfo.Argument) : EDevMenuPaths::Undefined;
		DynamicEntries.Emplace(FDevMenuFactory::CreateActionButton<FDevActionConsoleCommand>(
			DynamicEntryName,
			FormatEntryLabel(CommandArgumentsInfo),
			FDevActionConsoleCommand(Command, CommandArgumentsInfo.Argument)));
	}
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS

	return DynamicEntries;
}

FText FDevMenuConsoleCommandGroup::FormatEntryLabel(const FDevMenuConsoleCommandArgumentInfo& CVarValueInfo) const
{
	const FText& Fmt = (CVarValueInfo.LabelOverride.IsEmpty()) ? LabelFormat : CVarValueInfo.LabelOverride;
	return FText::FormatNamed(Fmt,
		TEXT("Command"), FText::FromString(Command),
		TEXT("Argument"), FText::FromString(CVarValueInfo.Argument));
}
