// Copyright (c) Alexandr Pereverzev.

#include "Actions/DevActionConsoleCommand.h"

#include "DevActionLogging.h"
#include "DevConsole.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"

IConsoleCommand* FDevActionConsoleCommandView::GetCCommand() const
{
	return FDevConsole::FindConsoleCommand(Command);
}

bool FDevActionConsoleCommandView::IsActionEnabled(const UObject* WorldContextObject) const
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS

	const IConsoleCommand* CCommand = GetCCommand();
	if (!CCommand) { return false; }

#if UE_COMPATIBILITY_SUPPORTED_CONSOLE_VARIABLE_IS_ENABLED
	return CCommand->IsEnabled();
#else
	return true;
#endif // UE_COMPATIBILITY_SUPPORTED_CONSOLE_VARIABLE_IS_ENABLED

#else
	return false;
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
}

bool FDevActionConsoleCommandView::IsActionVisible(const UObject* WorldContextObject) const
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS

	const IConsoleCommand* CCommand = GetCCommand();
	if (!CCommand) { return DevAction::Setting::GShowInvalidActions; }

	return true;

#else
	return false;
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
}

FStringView FDevActionConsoleCommandView::GetActionToolTip() const
{
#if !NO_CVARS

	const IConsoleCommand* CCommand = GetCCommand();
	return (CCommand) ? FStringView(CCommand->GetHelp()) : FStringView();

#else
	return FStringView();
#endif // !NO_CVARS
}

void FDevActionConsoleCommandView::ExecuteAction(const UObject* WorldContextObject) const
{
	FDevConsole::ConsoleCommand(WorldContextObject, Command);
}
