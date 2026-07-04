// Copyright (c) Alexandr Pereverzev.

#include "DevActionConsoleAccessor.h"

FDevActionConsoleAccessorManager& FDevActionConsoleAccessorManager::Get()
{
	static FDevActionConsoleAccessorManager Instance = FDevActionConsoleAccessorManager();
	return Instance;
}

IConsoleAccessor* FDevActionConsoleAccessorManager::FindConsoleAccessor(const FString& InName) const
{
	IConsoleAccessor* const* AccessorPtr = Accessors.Find(InName);
	return (AccessorPtr) ? *AccessorPtr: nullptr;
}

void FDevActionConsoleAccessorManager::RegisterConsoleAccessor(const FString& InName, IConsoleAccessor* InAccessor)
{
	if (InAccessor)
	{
		Accessors.Emplace(InName, InAccessor);
	}
}

void FDevActionConsoleAccessorManager::UnregisterConsoleAccessor(const FString& InName)
{
	Accessors.Remove(InName);
}

#define UE_API DEVACTIONS_API
template class UE_API TDevActionConsoleAccessor<bool>;
template class UE_API TDevActionConsoleAccessor<int32>;
template class UE_API TDevActionConsoleAccessor<float>;
template class UE_API TDevActionConsoleAccessor<FString>;
#undef UE_API
