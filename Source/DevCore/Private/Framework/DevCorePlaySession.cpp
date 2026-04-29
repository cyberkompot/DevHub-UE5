// Copyright (c) Alexandr Pereverzev.

#include "Framework/DevCorePlaySession.h"

#include "DevCoreInternal.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Kismet2/DebuggerCommands.h"
#endif // WITH_EDITOR

bool FDevCorePlaySession::IsGamePaused(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	return FPlayWorldCommandCallbacks::HasPlayWorldAndPaused();
#else
	return UGameplayStatics::IsGamePaused(WorldContextObject);
#endif // WITH_EDITOR
}

void FDevCorePlaySession::PausedGame(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	if (FPlayWorldCommandCallbacks::HasPlayWorldAndRunning())
	{
		FPlayWorldCommandCallbacks::PausePlaySession_Clicked();
	}
	else
	{
		UE_LOG_FUNCTION(LogDevCore, Log, TEXT("No play session or play session already paused"));
	}
#else
	if (!UGameplayStatics::IsGamePaused(WorldContextObject))
	{
		if (UGameplayStatics::SetGamePaused(WorldContextObject, true))
		{
			UE_LOG_FUNCTION(LogDevCore, Log, TEXT("Play session paused"));
		}
		else
		{
			UE_LOG_FUNCTION(LogDevCore, Warning, TEXT("Failed to pause play session"));
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevCore, Log, TEXT("Play session already paused"));
	}
#endif // WITH_EDITOR
}

void FDevCorePlaySession::ResumeGame(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	if (FPlayWorldCommandCallbacks::HasPlayWorldAndPaused())
	{
		FPlayWorldCommandCallbacks::ResumePlaySession_Clicked();
	}
	else
	{
		UE_LOG_FUNCTION(LogDevCore, Log, TEXT("No play session or play session is not paused"));
	}
#else
	if (UGameplayStatics::IsGamePaused(WorldContextObject))
	{
		if (UGameplayStatics::SetGamePaused(WorldContextObject, false))
		{
			UE_LOG_FUNCTION(LogDevCore, Log, TEXT("Play session resumed"));
		}
		else
		{
			UE_LOG_FUNCTION(LogDevCore, Warning, TEXT("Failed to resumed play session"));
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevCore, Log, TEXT("Play session is not paused"));
	}
#endif // WITH_EDITOR
}

void FDevCorePlaySession::StopGame(const UObject* WorldContextObject)
{
	UKismetSystemLibrary::QuitGame(WorldContextObject, nullptr, EQuitPreference::Quit, false);
}
