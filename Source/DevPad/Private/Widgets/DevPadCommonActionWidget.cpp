// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadCommonActionWidget.h"

#include "CommonInputSubsystem.h"
#include "DevCoreCompatibility.h"
#include "DevInputs.h"
#include "DevPadSettings.h"

#include UE_COMPATIBILITY_INCLUDE_COMMON_UI_TYPES_PATH

FSlateBrush UDevPadCommonActionWidget::GetIcon() const
{
	if (const FCommonInputActionDataBase* InputActionData = GetInputActionData())
	{
		ECommonInputType CurrentInputType = ECommonInputType::MouseAndKeyboard;
		FName CurrentGamepadName = FCommonInputDefaults::GamepadGeneric;

		if (const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
		{
			CurrentInputType = InputSubsystem->GetCurrentInputType();
			CurrentGamepadName = InputSubsystem->GetCurrentGamepadName();
		}

		if (CurrentInputType == ECommonInputType::Gamepad)
		{
#if PLATFORM_DESKTOP
			// Gamepad type override from settings on desktop platforms.
			if (const UDevPadSavableSettings* SavableSettings = GetDefault<UDevPadSavableSettings>())
			{
				switch (SavableSettings->PadWidgetGamepad)
				{
				case EDevPadGamepadPlatform::Xbox:
					CurrentGamepadName = EDevInputGamepadNames::Xbox;
					break;
				case EDevPadGamepadPlatform::PlayStation:
					CurrentGamepadName = EDevInputGamepadNames::PS5;
					break;
				case EDevPadGamepadPlatform::Steam:
					CurrentGamepadName = EDevInputGamepadNames::Steam;
					break;
				case EDevPadGamepadPlatform::AutoDetect:
				default:
					break;
				}
			}
#endif

			// Refine the gamepad name using our implementation, which requires no setup.
			if (CurrentGamepadName == FCommonInputDefaults::GamepadGeneric || !InputActionData->HasGamepadInputOverride(CurrentGamepadName))
			{
				if (const UDevInputs* Inputs =  UDevInputs::Get(GetWorld()))
				{
					CurrentGamepadName = Inputs->GetCurrentGamepadName();
				}
			}
		}

		const FCommonInputTypeInfo& InputTypeInfo = InputActionData->GetInputTypeInfo(CurrentInputType, CurrentGamepadName);
		if (InputTypeInfo.OverrideBrush.DrawAs != ESlateBrushDrawType::NoDrawType)
		{
			return InputTypeInfo.OverrideBrush;
		}
	}
	return Super::GetIcon();
}

#if PLATFORM_DESKTOP

void UDevPadCommonActionWidget::PostInitProperties()
{
	Super::PostInitProperties();

	if (!SavableSettingsChangedHandle.IsValid())
	{
		SavableSettingsChangedHandle = UDevPadSavableSettings::OnSettingsChanged().AddUObject(this, &ThisClass::OnSettingsChanged);
	}
}

void UDevPadCommonActionWidget::BeginDestroy()
{
	if (SavableSettingsChangedHandle.IsValid())
	{
		UDevPadSavableSettings::OnSettingsChanged().Remove(SavableSettingsChangedHandle);
		SavableSettingsChangedHandle.Reset();
	}

	Super::BeginDestroy();
}

void UDevPadCommonActionWidget::OnSettingsChanged(const UDevPadSavableSettings* SavableSettings)
{
	if (LastPadWidgetGamepad != SavableSettings->PadWidgetGamepad)
	{
		LastPadWidgetGamepad = SavableSettings->PadWidgetGamepad;
		if (IsConstructed())
		{
			UpdateActionWidget();
		}
	}
}

#endif // PLATFORM_DESKTOP
