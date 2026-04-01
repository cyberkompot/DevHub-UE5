// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuGenerator.generated.h"

class UDevMenuRegistry;

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevMenuGenerator final : public UObject
{
	GENERATED_BODY()

public:
	UDevMenu* GenerateMenu(const FName InMenuPath);

	FORCEINLINE void SetMenuRegistry(UDevMenuRegistry& InMenuRegistry) { MenuRegistry = &InMenuRegistry; }

private:
	static const FName GeneratedMenuName;

	UPROPERTY(Transient)
	UDevMenuRegistry* MenuRegistry;

	void PopulateMenuEntries(UDevMenu& InGeneratedMenu) const;
};
