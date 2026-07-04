// Copyright (c) Alexandr Pereverzev.

#include "DevActions.h"

#include "DevActionConsoleAccessor.h"
#include "DevActionInternals.h"
#include "DevActionTypes.h"
#include "DevCoreCompatibility.h"
#include "Engine/Console.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

void FDevActions::ExecuteAction(const UObject* WorldContextObject, const FDevAction& Action)
{
	Action.ExecuteAction(WorldContextObject);
}

bool FDevActions::ExecuteConsoleCommand(const UObject* WorldContextObject, const FString& Command)
{
	if (Command.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console command is not defined"));
		return false;
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
				return true;
			}
		}

		// 2. Routing via FirstPlayerController.
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->Player)
			{
				PlayerController->ConsoleCommand(Command, true);
				return true;
			}
		}

		// 3. Routing via GEngine with world.
		return GEngine->Exec(World, *Command);
	}
	else
	{
		// Routing via GEngine without world.
		return GEngine->Exec(nullptr, *Command);
	}
#else
	UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console commands are disabled in this build"));
	return false;
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
}

IConsoleAccessor* FDevActions::FindConsoleAccessor(const FString& Name)
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS && !NO_CVARS
	return FDevActionConsoleAccessorManager::Get().FindConsoleAccessor(Name);
#else
	return nullptr;
#endif
}

IConsoleCommand* FDevActions::FindConsoleCommand(const FString& Name)
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* CommandName;
	FNameBuilder CommandNameBuffer; // Must remain valid for at least the lifetime of CommandName.

	if (int32 SpaceIndex; Name.FindChar(TEXT(' '), SpaceIndex))
	{
		CommandNameBuffer.Append(*Name, SpaceIndex);
		CommandName = CommandNameBuffer.ToString();
	}
	else
	{
		CommandName = *Name;
	}

	if (IConsoleObject* CObject = IConsoleManager::Get().FindConsoleObject(CommandName, false))
	{
		return CObject->AsCommand();
	}

	// Fallback search in UConsole::AutoCompleteList for exec-style commands not in IConsoleManager.
	if (GEngine && GEngine->GameViewport)
	{
		if (UConsole* Console = GEngine->GameViewport->ViewportConsole)
		{
			if (!Console->bIsRuntimeAutoCompleteUpToDate || Console->AutoCompleteList.IsEmpty())
			{
				Console->BuildRuntimeAutoCompleteList(true);
			}

			for (const FAutoCompleteCommand& AutoComplete : Console->AutoCompleteList)
			{
				if (AutoComplete.Command.IsEmpty()) { continue; }

				// Extract the command name part from this AutoComplete entry.
				const TCHAR* AutoCommandName;
				FNameBuilder AutoNameBuffer; // Must remain valid for at least the lifetime of AutoCommandName.

				if (int32 AutoSpaceIndex; AutoComplete.Command.FindChar(TEXT(' '), AutoSpaceIndex))
				{
					AutoNameBuffer.Append(*AutoComplete.Command, AutoSpaceIndex);
					AutoCommandName = AutoNameBuffer.ToString();
				}
				else
				{
					AutoCommandName = *AutoComplete.Command;
				}

				// Case-insensitive match against extracted Command name.
				if (FCString::Stricmp(CommandName, AutoCommandName) == 0)
				{
					return IConsoleManager::Get().RegisterConsoleCommand(AutoCommandName, *AutoComplete.Desc);
				}
			}
		}
	}
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	return nullptr;
}

IConsoleVariable* FDevActions::FindConsoleVariable(const FString& Name)
{
#if !NO_CVARS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* VariableName;
	FNameBuilder VariableNameBuffer; // Must remain valid for at least the lifetime of VariableName.

	if (int32 SpaceIndex; Name.FindChar(TEXT(' '), SpaceIndex))
	{
		VariableNameBuffer.Append(*Name, SpaceIndex);
		VariableName = VariableNameBuffer.ToString();
	}
	else
	{
		VariableName = *Name;
	}

	return IConsoleManager::Get().FindConsoleVariable(VariableName, false);
#else
	return nullptr;
#endif // !NO_CVARS
}

UWorld* FDevActions::FindCurrentPlayWorld()
{
	// 1. Game world or PIE if there is no ambiguity.
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			return World;
		}
	}

#if WITH_EDITOR
	if (GEditor)
	{
		auto GetViewportWorld = [](const FViewport* Viewport) -> UWorld*
		{
			if (Viewport)
			{
				if (const FViewportClient* ViewportClient = Viewport->GetClient())
				{
					if (UWorld* World = ViewportClient->GetWorld())
					{
						return World;
					}
				}
			}
			return nullptr;
		};

		// 2. Current game instance viewport is our next best candidate.
		if (UWorld* World = GetViewportWorld(GEditor->GetPIEViewport()))
		{
			return World;
		}

		// 3. If no viewport is specified, use the PIE instance with the smallest index.
		const FWorldContext* BestWorldContext = nullptr;
		for (const FWorldContext& WorldContext : GEditor->GetWorldContexts())
		{
			if ((WorldContext.WorldType == EWorldType::PIE) && (WorldContext.World())
				&& (!BestWorldContext || WorldContext.PIEInstance < BestWorldContext->PIEInstance))
			{
				BestWorldContext = &WorldContext;
			}
		}
		if (BestWorldContext)
		{
			return BestWorldContext->World();
		}
	}
#endif
	return nullptr;
}
