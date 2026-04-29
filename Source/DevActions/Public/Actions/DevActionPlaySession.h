// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "Framework/DevCorePlaySession.h"
#include "DevActionPlaySession.generated.h"

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Pause Game")
struct DEVACTIONS_API FDevActionPausePlaySession : public FDevActionBase
{
	GENERATED_BODY()

	FDevActionPausePlaySession()
	{
		Label = FText::FromString("Pause Game");
	}

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override { FDevCorePlaySession::PausedGame(WorldContextObject); }
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return !FDevCorePlaySession::IsGamePaused(WorldContextObject); }
	//~ End FDevAction interface.
};

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Resume Game")
struct DEVACTIONS_API FDevActionResumePlaySession : public FDevActionBase
{
	GENERATED_BODY()

	FDevActionResumePlaySession()
	{
		Label = FText::FromString("Resume Game");
	}

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override { FDevCorePlaySession::ResumeGame(WorldContextObject); }
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return FDevCorePlaySession::IsGamePaused(WorldContextObject); }
	//~ End FDevAction interface.
};

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Quit Game")
struct DEVACTIONS_API FDevActionStopPlaySession : public FDevActionBase
{
	GENERATED_BODY()

	FDevActionStopPlaySession()
	{
		Label = FText::FromString("Quit Game");
	}

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override { FDevCorePlaySession::StopGame(WorldContextObject); }
	//~ End FDevAction interface.
};
