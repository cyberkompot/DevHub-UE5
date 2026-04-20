// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.generated.h"

UCLASS(Abstract, MinimalAPI, Blueprintable, BlueprintType, Category = "DevHub|Core")
class UDevCoreResettable : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Core")
	DEVCORE_API virtual void Reset() { /** Nop. */ }

	//~ Begin UObject interface.
	virtual void BeginDestroy() override
	{
		Reset();
		Super::BeginDestroy();
	}
	//~ End UObject interface.
};
