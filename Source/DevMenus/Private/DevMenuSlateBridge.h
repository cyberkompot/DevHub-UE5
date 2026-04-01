// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Engine/GameInstance.h"
#include "Engine/World.h"

struct FDevMenuSlateBridge final
{
	template <typename TSubsystemClass>
	static TSubsystemClass* GetSubsystem(const SWidget* ViewportContextWidget)
	{
		if (const UWorld* World = GetWorld(ViewportContextWidget))
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<TSubsystemClass>();
			}
		}
		return nullptr;
	}

	static UWorld* GetWorld(const SWidget* ViewportContextWidget);
};
