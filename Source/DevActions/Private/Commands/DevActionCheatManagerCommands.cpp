// Copyright (c) Alexandr Pereverzev.

#include "DevActionConsoleAccessor.h"
#include "DevActionInternals.h"
#include "GameFramework/WorldSettings.h"

namespace DevConsole::Extension
{
	FDevActionFloatConsoleAccessor CheatManagerSlomoAccessor(TEXT("CheatManager.Slomo"), TEXT("Modify time dilation to affect the apparent passage of time."),
        FDevActionFloatConsoleAccessor::FGetter::CreateLambda([](const UWorld* World)
        {
        	if (World)
        	{
        		if (const AWorldSettings* WorldSettings = World->GetWorldSettings(false, false))
        		{
					return WorldSettings->TimeDilation;
				}
				else
				{
					UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("World settings are missing: World = %s"), *GetNameSafe(World));
				}
        	}
	        else
	        {
				UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("World is missing"));
	        }
        	return 1.0f;
        }),
		FDevActionFloatConsoleAccessor::FSetter::CreateLambda([](const float& Value, const UWorld* World)
		{
			if (World)
			{
				if (AWorldSettings* WorldSettings = World->GetWorldSettings(false, false))
				{
					WorldSettings->SetTimeDilation(Value);
				}
				else
				{
					UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("World settings are missing: World = %s"), *GetNameSafe(World));
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("World is missing"));
			}
		}), ECVF_Cheat);
}
