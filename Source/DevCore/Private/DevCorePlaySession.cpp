// Copyright (c) Alexandr Pereverzev.

#include "DevCorePlaySession.h"

#if WITH_EDITOR
#include "Kismet2/DebuggerCommands.h"
#else
#include "Kismet/GameplayStatics.h"
#endif // WITH_EDITOR

bool FDevCorePlaySession::IsGamePaused(const UObject* WorldContextObject)
{
#if WITH_EDITOR
	return FPlayWorldCommandCallbacks::HasPlayWorldAndPaused();
#else
	return UGameplayStatics::IsGamePaused(WorldContextObject);
#endif // WITH_EDITOR
}
