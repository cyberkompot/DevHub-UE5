// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuRegistryEntries.generated.h"

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct FDevMenuRegistryProxyEntry : public FDevMenuProxy
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End IDevMenuEntry interface.
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct FDevMenuRegistrySubMenuEntry : public FDevMenuSubMenu
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End IDevMenuEntry interface.
};

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevMenuRegistryMenu final : public UDevMenu
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return GetClass(); };
	//~ End IDevMenuEntry interface.
};
