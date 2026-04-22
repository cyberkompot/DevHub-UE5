// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "DevCoreGameInstanceSubsystem.generated.h"

UCLASS(Abstract, NotBlueprintable, Meta = (Hidden))
class DEVCORE_API UDevCoreGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

protected:
	template <class TSubsystemClass = UDevCoreGameInstanceSubsystem>
	static TSubsystemClass* Get(const UObject* WorldContextObject)
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<TSubsystemClass>();
			}
		}
		return nullptr;
	}
};
