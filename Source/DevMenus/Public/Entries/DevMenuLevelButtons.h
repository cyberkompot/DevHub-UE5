// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuLevelButtons.generated.h"

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Load Level Group"))
struct DEVMENUS_API FDevMenuLoadLevelGroup : public FDevMenuDynamicOuterWithLayoutBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "Layout"))
	TArray<FString> IncludedPaths;

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "IncludedPaths"))
	TArray<FString> ExcludePaths = { "/Engine", "/Game/StarterContent" };

	//~ Begin FDevMenuDynamicOuterBase interface.
	virtual FDevMenuInstancedEntries CreateDynamicEntries() const override;
	//~ End FDevMenuDynamicOuterBase interface.

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuDynamicOuterWithLayoutBase;

	bool IsPackageUnderDirectories(const FString& InPackagePath, const TArray<FString>& InDirectories, const bool InResultForEmptyDirectories) const;
};
