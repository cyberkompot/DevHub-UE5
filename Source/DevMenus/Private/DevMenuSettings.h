// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"
#include "DevMenuTypes.h"
#include "Engine/DeveloperSettings.h"
#include "DevMenuSettings.generated.h"

USTRUCT(NotBlueprintable, NotBlueprintType)
struct FDevMenuShortcuts
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "DevHub|Menu")
	FDevInputShortcut MainMenu = TEXT("Control + ` | L3 + R3");
};

UCLASS(Config = Game, DefaultConfig, Category = "DevHub|Menu", DisplayName = "Dev Menu")
class DEVMENUS_API UDevMenuSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDevMenuSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Dev Menu")
	FDevMenuShortcuts Shortcuts;

	UPROPERTY(Config, EditAnywhere, Category = "Dev Menu", Meta = (BaseStruct = "/Script/DevMenus.DevMenuEntry", ExcludeBaseStruct, DisplayName = "Dev Menu"))
	TArray<FInstancedStruct> MainMenuEntries;

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject interface.
};
