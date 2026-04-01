// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "DevActionBlueprintFunction.generated.h"

/**
 * Executes the specified function from Blueprint.
 */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Blueprint Function")
struct DEVACTIONS_API FDevActionBlueprintFunction : public FDevAction
{
	GENERATED_BODY()

	/** The Blueprint in which the function with the specified name is to be executed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action", Meta = (DisplayThumbnail = false))
	TSoftObjectPtr<UBlueprint> Blueprint;

	/** The name of the function to execute. The function must be public, have no parameters, and no return value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	FName FunctionName;

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	//~ End FDevAction interface.
};
