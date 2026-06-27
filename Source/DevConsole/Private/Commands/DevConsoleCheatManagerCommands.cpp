// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleTypes.h"
#include "DevConsoleInternals.h"
#include "GameFramework/WorldSettings.h"

namespace DevConsole::Extension
{
	FDevConsoleDynamicFloatConsoleVariableWithWorld ExtSlomoCVar(TEXT("Ext.Slomo"), TEXT("Modify time dilation to affect the apparent passage of time."),
        FDevConsoleDynamicFloatConsoleVariableWithWorld::FGetter::CreateLambda([](UWorld* World)
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
		FDevConsoleDynamicFloatConsoleVariableWithWorld::FSetter::CreateLambda([](UWorld* World, const float& Value)
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
		}));
}
