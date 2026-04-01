// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuStatCommandButtons.generated.h"

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Stat Command Group"))
struct DEVMENUS_API FDevMenuStatCommandGroup : public FDevMenuDynamicOuterWithLayoutBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "Layout"))
	TArray<FString> StatNames = { "FPS", "UNIT", "Memory", "RHI", "SceneRendering" };

	//~ Begin FDevMenuDynamicOuterBase interface.
	virtual FDevMenuInstancedEntries CreateDynamicEntries() const override;
	//~ End FDevMenuDynamicOuterBase interface.

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuDynamicOuterWithLayoutBase;
};
