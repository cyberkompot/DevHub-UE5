// Copyright (c) Alexandr Pereverzev.

#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

namespace DevConsole::Extension
{
	FAutoConsoleCommandWithWorld ExtHideAllStatsCommand(TEXT("Ext.Stats.HideAllStats"), TEXT("Hides all stats."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (GEngine)
			{
				FCommonViewportClient* CommonViewportClient = (GStatProcessingViewportClient)? GStatProcessingViewportClient : GEngine->GameViewport;
				if (CommonViewportClient)
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
			}
		}));
}
