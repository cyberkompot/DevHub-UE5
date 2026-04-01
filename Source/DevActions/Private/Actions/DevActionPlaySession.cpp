// Copyright (c) Alexandr Pereverzev.

#include "Actions/DevActionPlaySession.h"

#include "Engine/Engine.h"
#include "DevActionLogging.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Kismet2/DebuggerCommands.h"
#endif // WITH_EDITOR

void FDevActionPausePlaySession::OnExecuteAction(const UObject* WorldContextObject) const
{
#if WITH_EDITOR
	if (FPlayWorldCommandCallbacks::HasPlayWorldAndRunning())
	{
		FPlayWorldCommandCallbacks::PausePlaySession_Clicked();
	}
	else
	{
		UE_LOG_FUNCTION(LogDevActions, Log, TEXT("No play session or play session already paused"));
	}
#else
	if (!UGameplayStatics::IsGamePaused(WorldContextObject))
	{
		if (UGameplayStatics::SetGamePaused(WorldContextObject, true))
		{
			UE_LOG_FUNCTION(LogDevActions, Log, TEXT("Play session paused"));
		}
		else
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Failed to pause play session"));
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevActions, Log, TEXT("Play session already paused"));
	}
#endif // WITH_EDITOR
}

bool FDevActionPausePlaySession::OnGetActionVisibility(const UObject* WorldContextObject) const
{
#if WITH_EDITOR
	return FPlayWorldCommandCallbacks::HasPlayWorldAndRunning();
#else
	return (!UGameplayStatics::IsGamePaused(WorldContextObject));
#endif // WITH_EDITOR
}


void FDevActionResumePlaySession::OnExecuteAction(const UObject* WorldContextObject) const
{
#if WITH_EDITOR
	if (FPlayWorldCommandCallbacks::HasPlayWorldAndPaused())
	{
		FPlayWorldCommandCallbacks::ResumePlaySession_Clicked();
	}
	else
	{
		UE_LOG_FUNCTION(LogDevActions, Log, TEXT("No play session or play session is not paused"));
	}
#else
	if (UGameplayStatics::IsGamePaused(WorldContextObject))
	{
		if (UGameplayStatics::SetGamePaused(WorldContextObject, false))
		{
			UE_LOG_FUNCTION(LogDevActions, Log, TEXT("Play session resumed"));
		}
		else
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Failed to resumed play session"));
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevActions, Log, TEXT("Play session is not paused"));
	}
#endif // WITH_EDITOR
}

bool FDevActionResumePlaySession::OnGetActionVisibility(const UObject* WorldContextObject) const
{
#if WITH_EDITOR
	return FPlayWorldCommandCallbacks::HasPlayWorldAndPaused();
#else
	return UGameplayStatics::IsGamePaused(WorldContextObject);
#endif // WITH_EDITOR
}


void FDevActionStopPlaySession::OnExecuteAction(const UObject* WorldContextObject) const
{
	UKismetSystemLibrary::QuitGame(WorldContextObject, nullptr, EQuitPreference::Quit, false);
}
