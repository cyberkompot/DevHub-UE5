// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevPadSettings.generated.h"

class UDevPadPage;
class UDevPadPageWidget;
class UDevPadPanelWidget;

UCLASS(Config = Game, DefaultConfig, Category = "DevHub|Pad", DisplayName = "DevPad")
class UDevPadSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDevPadSettings();

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Pages")
	TSoftObjectPtr<UDevPadPage> CommonPage;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Pages")
	TArray<TSoftObjectPtr<UDevPadPage>> MainPages;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Widgets")
	TSoftClassPtr<UDevPadPanelWidget> PadWidgetClass;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Widgets")
	TMap<TSoftClassPtr<UDevPadPage>, TSoftClassPtr<UDevPadPageWidget>> PageWidgetClasses;

	TSoftClassPtr<UDevPadPageWidget> GetPageWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const;
};
