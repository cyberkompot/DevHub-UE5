// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleTypes.h"
#include "DevConsoleInternals.h"
#include "GameFramework/WorldSettings.h"

namespace DevConsole::Extension
{
	FDevConsoleDynamicFloatConsoleVariable ExtSlomoCVar(TEXT("CheatManager.Slomo"), TEXT("Modify time dilation to affect the apparent passage of time."),
        FDevConsoleDynamicFloatConsoleVariable::FGetter::CreateLambda([](const UWorld* World)
        {
        	if (World)
        	{
        		if (const AWorldSettings* WorldSettings = World->GetWorldSettings(false, false))
        		{
					return WorldSettings->TimeDilation;
				}
				else
				{
					UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("World setting are missing: World = %s"), *GetNameSafe(World));
				}
        	}
	        else
	        {
				UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("World is missing"));
	        }
        	return 1.0f;
        }),
		FDevConsoleDynamicFloatConsoleVariable::FSetter::CreateLambda([](const float& Value, const UWorld* World)
		{
			if (World)
			{
				if (AWorldSettings* WorldSettings = World->GetWorldSettings(false, false))
				{
					WorldSettings->SetTimeDilation(Value);
				}
				else
				{
					UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("World setting are missing: World = %s"), *GetNameSafe(World));
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevConsole, Warning, TEXT("World is missing"));
			}
		}), ECVF_Cheat);
}
