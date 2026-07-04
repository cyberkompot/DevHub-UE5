// Copyright (c) Alexandr Pereverzev.

#include "DevActionInternals.h"
#include "DevActionTypes.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

namespace DevConsole::Extension
{
	FAutoConsoleCommandWithWorldAndArgs ExtIncCVarCommand(TEXT("IncVar"), TEXT("Increase Console Variable on give value."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (Args.Num() == 2)
			{
				const FString& CVarName = Args[0];
				const FString& Value = Args[1];
				if (IConsoleVariable* CVar = FDevActionInternals::FindConsoleVariableOrAccessor(CVarName))
				{
					if (CVar->IsVariableString())
					{
						CVar->Set(*(CVar->GetString() + Value));
					}
					else if (Value.IsNumeric())
					{
						if (CVar->IsVariableFloat())
						{
							CVar->Set(CVar->GetFloat() + FCString::Atof(*Value));
						}
						else
						{
							CVar->Set(CVar->GetInt() + FCString::Atoi(*Value));
						}
					}
					else
					{
						UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Value is not a numeric type: CVar = %s, Value = %s"), *CVarName, *Value);
					}
				}
				else
				{
					UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console variable not found: CVar = %s"), *CVarName);
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Invalid arguments count. Expected two arguments: Console Variable name, and value"));
			}
	}));

	FAutoConsoleCommandWithWorldAndArgs ExtDecCVarCommand(TEXT("DecVar"), TEXT("Decrease Console Variable on give value."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (Args.Num() == 2)
			{
				const FString& CVarName = Args[0];
				const FString& Value = Args[1];
				if (IConsoleVariable* CVar = FDevActionInternals::FindConsoleVariableOrAccessor(CVarName))
				{
					if (CVar->IsVariableString())
					{
						CVar->Set(*CVar->GetString().Replace(*Value, TEXT(""), ESearchCase::IgnoreCase));
					}
					else if (Value.IsNumeric())
					{
						if (CVar->IsVariableFloat())
						{
							CVar->Set(CVar->GetFloat() - FCString::Atof(*Value));
						}
						else
						{
							CVar->Set(CVar->GetInt() - FCString::Atoi(*Value));
						}
					}
					else
					{
						UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Value is not a numeric type: CVar = %s, Value = %s"), *CVarName, *Value);
					}
				}
				else
				{
					UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console variable not found: CVar = %s"), *CVarName);
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Invalid arguments count. Expected two arguments: Console Variable name, and value"));
			}
	}));

	FAutoConsoleCommandWithWorldAndArgs ExtToggleCVarCommand(TEXT("ToggleVar"), TEXT("Toggle Console Variable."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (Args.Num() == 1)
			{
				const FString& CVarName = Args[0];
				if (IConsoleVariable* CVar = FDevActionInternals::FindConsoleVariableOrAccessor(CVarName))
				{
					if (!CVar->IsVariableString())
					{
						CVar->Set(CVar->GetBool());
					}
					else
					{
						UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("String console variables are not supported: CVar = %s"), *CVarName);
					}
				}
				else
				{
					UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console variable not found: CVar = %s"), *CVarName);
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Invalid argument count. Expected one argument: Console Variable"));
			}
	}));
}
