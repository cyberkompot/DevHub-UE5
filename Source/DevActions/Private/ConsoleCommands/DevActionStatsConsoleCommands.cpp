// Copyright (c) Alexandr Pereverzev.

#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

namespace DevConsole::Ext::Stats
{
	static FAutoConsoleCommandWithWorld HideAllStatsCommand(TEXT("DevHub.Console.Ext.Stats.HideAllStats"), TEXT("Hides all stats."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (FCommonViewportClient* CommonViewportClient = (GStatProcessingViewportClient)? GStatProcessingViewportClient : GEngine->GameViewport)
			{
				if (const TArray<FString>* EnabledStats = CommonViewportClient->GetEnabledStats())
				{
					while (!EnabledStats->IsEmpty())
					{
						const FString& CommandName = EnabledStats->Last();
						GEngine->ExecEngineStat(World, CommonViewportClient, *CommandName);
					}
				}
			}
	}));
}
