// Copyright (c) Alexandr Pereverzev.

#include "Actions/DevActionConsoleCommand.h"

#include "DevActionConsoleLibrary.h"
#include "DevActionLogging.h"
#include "Engine/Console.h"
#include "GameFramework/PlayerController.h"

IConsoleCommand* FDevActionConsoleCommandView::GetCCommand() const
{
	return FDevActionConsoleLibrary::Get().FindConsoleCommand(Command);
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
	if (Command.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console command is not defined"));
		return;
	}

#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// 1. Routing through the ViewportConsole.
		if (const UGameViewportClient* GameViewport = World->GetGameViewport())
		{
			if (const TObjectPtr<UConsole>& Console = GameViewport->ViewportConsole)
			{
				Console->ConsoleCommand(Command);
				return;
			}
		}

		// 2. Routing via FirstPlayerController.
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->Player)
			{
				PlayerController->ConsoleCommand(Command, true);
				return;
			}
		}

		// 3. Routing via GEngine with world.
		GEngine->Exec(World, *Command);
		return;
	}

	// 4. Routing via GEngine without world.
	GEngine->Exec(nullptr, *Command);

#else
	UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console commands are disabled in this build"));
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
}
