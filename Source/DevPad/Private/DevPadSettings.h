// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionConsoleAccessor.h"
#include "DevInputTypes.h"
#include "DevPadTypes.h"
#include "DevPadSettings.generated.h"

class UDevPadPage;
class UDevPadPageWidget;
class UDevPadPanelWidget;

namespace DevPad::Settings
{
	extern TDevActionConsoleAccessor<EDevPadAlignment> PadWidgetAlignmentAccessor;
	extern FDevActionFloatConsoleAccessor PadWidgetScaleAccessor;
}

UENUM(BlueprintType, Category = "DevHub|Pad")
enum struct EDevPadAlignmentChange : uint8
{
	ToTop = 0,
	ToBottom = 1,
	ToLeft = 2,
	ToRight = 3,
};

UENUM(BlueprintType, Category = "DevHub|Pad")
enum struct EDevPadGamepadPlatform : uint8
{
	AutoDetect = 0 UMETA(DisplayName = "Auto-detect"),
	Xbox = 1 UMETA(DisplayName = "XBOX"),
	PlayStation = 2 UMETA(DisplayName = "PlayStation"),
	Steam = 3 UMETA(DisplayName = "Steam"),
};

UCLASS(Config = Game, DefaultConfig, Category = "DevHub|Pad", DisplayName = "Dev Pad")
class UDevPadSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDevPadSettings();

	static const FLazyName AutoDetectedGamepad;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, const UDevPadSettings*)

	UPROPERTY(Config, EditAnywhere, Category = "DevPad", DisplayName = "Shortcut")
	FDevInputShortcut PadShortcut = "Num 5 | Special Right + D-pad Down";

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

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Display", DisplayName = "Pad Gamepad")
	EDevPadGamepadPlatform PadWidgetGamepad = EDevPadGamepadPlatform::AutoDetect;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Display", DisplayName = "Pad Alignment")
	EDevPadAlignment PadWidgetAlignment = EDevPadAlignment::BottomRight;

	UPROPERTY(Config, EditDefaultsOnly, Category = "DevPad|Display", DisplayName = "Pad Scale", meta = (ClampMin = 1.f, ClampMax = 2.f))
	float PadWidgetScale = 1.f;

	TSoftClassPtr<UDevPadInfoWidget> GetInfoWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const;
	TSoftClassPtr<UDevPadPageWidget> GetPageWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const;

	void NotifySettingsChanged() const { OnSettingsChanged().Broadcast(this); };

	static FOnSettingsChanged& OnSettingsChanged() { return OnSettingsChangedDelegate; }

private:
	static FOnSettingsChanged OnSettingsChangedDelegate;
};
