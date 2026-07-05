// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "CommonActionWidget.h"
#include "DevPadSettings.h"
#include "DevPadCommonActionWidget.generated.h"

/** Displays a platform-specific icon. Functions correctly even when UCommonInputPlatformSettings is not configured. */
UCLASS(BlueprintType, Blueprintable)
class UDevPadCommonActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:
	//~ Begin UCommonActionWidget Interface.
	virtual FSlateBrush GetIcon() const override;
	//~ End UCommonActionWidget Interface.

#if PLATFORM_DESKTOP
	//~ Begin UObject Interface.
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;
	//~ End UObject Interface.

private:
	EDevPadGamepadPlatform LastPadWidgetGamepad = EDevPadGamepadPlatform::AutoDetect;
	FDelegateHandle SettingsChangedHandle;

	void OnSettingsChanged(const UDevPadSettings* Settings);
#endif // PLATFORM_DESKTOP
};