// Copyright (c) Alexandr Pereverzev.

#include "DevConsole.h"

#include "DevConsoleLibrary.h"
#include "DevCoreCompatibility.h"
#include "DevConsoleInternals.h"
#include "Engine/Console.h"
#include "GameFramework/PlayerController.h"

UWorld* FDevConsole::FindCurrentPlayWorld()
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

bool FDevConsole::ConsoleCommand(const UObject* WorldContextObject, const FString& Command)
{
	if (Command.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("Console command is not defined"));
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
	UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("Console commands are disabled in this build"));
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	return false;
}

IConsoleCommand* FDevConsole::FindConsoleCommand(const FString& Name)
{
	return FDevConsoleLibrary::Get().FindConsoleCommand(Name);
}

IConsoleVariable* FDevConsole::FindConsoleVariable(const FString& Name)
{
	return FDevConsoleLibrary::Get().FindConsoleVariable(Name);
}
