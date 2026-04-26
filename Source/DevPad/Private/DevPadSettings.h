// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"
#include "DevPadTypes.h"
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

	UPROPERTY(Config, EditAnywhere, Category = "DevPad", DisplayName = "Shortcut")
	FDevInputShortcut PadShortcut = "Num 5 | L3 + R3";

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Pages", meta = (DisplayThumbnail = false))
	TSoftObjectPtr<UDevPadPage> CommonPage = nullptr;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Pages", meta = (DisplayThumbnail = false))
	TArray<TSoftObjectPtr<UDevPadPage>> MainPages;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Widgets")
	TSoftClassPtr<UDevPadPanelWidget> PadWidgetClass = nullptr;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Widgets")
	TMap<TSoftClassPtr<UDevPadPage>, TSoftClassPtr<UDevPadInfoWidget>> InfoWidgetClasses;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Widgets")
	TMap<TSoftClassPtr<UDevPadPage>, TSoftClassPtr<UDevPadPageWidget>> PageWidgetClasses;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Display", DisplayName = "Pad Alignment")
	EDevPadAlignment PadWidgetAlignment = EDevPadAlignment::BottomRight;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Display", DisplayName = "Pad Scale", meta = (ClampMin = 0.5f, ClampMax = 1.0f, Units = "Percent"))
	float PadWidgetScale = 1.f;

	TSoftClassPtr<UDevPadInfoWidget> GetInfoWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const;
	TSoftClassPtr<UDevPadPageWidget> GetPageWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const;
};
