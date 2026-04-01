// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "DevActionPlaySession.generated.h"

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Pause Game (Pause Play Session)")
struct DEVACTIONS_API FDevActionPausePlaySession : public FDevAction
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override;
	//~ End FDevAction interface.
};

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Resume Game (Resume Play Session)")
struct DEVACTIONS_API FDevActionResumePlaySession : public FDevAction
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override;
	//~ End FDevAction interface.
};

USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Quit Game (Quit Play Session)")
struct DEVACTIONS_API FDevActionStopPlaySession : public FDevAction
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	//~ End FDevAction interface.
};
