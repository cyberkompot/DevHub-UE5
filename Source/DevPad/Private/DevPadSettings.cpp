// Copyright (c) Alexandr Pereverzev.

#include "DevPadSettings.h"

#include "DevActionConsoleAccessor.h"
#include "DevPadLogging.h"
#include "DevPadTypes.h"

namespace DevPad::Settings
{
	TDevActionConsoleAccessor<EDevPadGamepadPlatform> PadWidgetGamepadAccessor = TDevActionConsoleAccessor<EDevPadGamepadPlatform>(TEXT("DevHub.Pad.Settings.Gamepad"), TEXT("DevHub Pad HUD widget gamepad fo PC platforms (0 - AutoDetected, 1 - XBOX, 2 - PlayStation, 3 - Steam)"),
		TDevActionConsoleAccessor<EDevPadGamepadPlatform>::FGetter::CreateLambda([](UWorld* World)
		{
			return GetDefault<UDevPadSavableSettings>()->PadWidgetGamepad;
		}),
		TDevActionConsoleAccessor<EDevPadGamepadPlatform>::FSetter::CreateLambda([](const EDevPadGamepadPlatform& Value, UWorld* World)
		{
			if (UDevPadSavableSettings* SavableSettings = GetMutableDefault<UDevPadSavableSettings>())
			{
				SavableSettings->PadWidgetGamepad = Value;
				SavableSettings->SaveAndNotify();
			}
		}));

	TDevActionConsoleAccessor<EDevPadAlignment> PadWidgetAlignmentAccessor = TDevActionConsoleAccessor<EDevPadAlignment>(TEXT("DevHub.Pad.Settings.Alignment"), TEXT("DevHub Pad HUD widget alignment (0 - TopLeft, 1 - TopRight, 2 - BottomLeft, 3 - BottomRight)"),
		TDevActionConsoleAccessor<EDevPadAlignment>::FGetter::CreateLambda([](const UWorld* World)
		{
			return GetDefault<UDevPadSavableSettings>()->PadWidgetAlignment;
		}),
		TDevActionConsoleAccessor<EDevPadAlignment>::FSetter::CreateLambda([](const EDevPadAlignment& Value, const UWorld* World)
		{
			if (UDevPadSavableSettings* SavableSettings = GetMutableDefault<UDevPadSavableSettings>())
			{
				SavableSettings->PadWidgetAlignment = Value;
				SavableSettings->SaveAndNotify();
			}
		}));

	FDevActionFloatConsoleAccessor PadWidgetScaleAccessor = FDevActionFloatConsoleAccessor(TEXT("DevHub.Pad.Settings.Scale"), TEXT("DevHub Pad HUD widget scaling [0.5 - 1.0]"),
		FDevActionFloatConsoleAccessor::FGetter::CreateLambda([](UWorld* World)
		{
			return GetDefault<UDevPadSavableSettings>()->PadWidgetScale;
		}),
		FDevActionFloatConsoleAccessor::FSetter::CreateLambda([](const float& Value, UWorld* World)
		{
			if (UDevPadSavableSettings* SavableSettings = GetMutableDefault<UDevPadSavableSettings>())
			{
				SavableSettings->PadWidgetScale = FMath::Clamp(Value, 1.f, 2.f);
				SavableSettings->SaveAndNotify();
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs PadWidgetChangeAlignmentCommand(TEXT("DevHub.Pad.Settings.ChangeAlignment"), TEXT("Changes DevHub Pad HUD widget alignment (0 - ToTop, 1 - ToBottom, 2 - ToLeft, 3 - ToRight)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (Args.Num() == 1)
			{
				const FString& EnumName = Args[0];
				const EDevPadAlignment OriginalAlignment = PadWidgetAlignmentAccessor->GetEnum<EDevPadAlignment>();
				EDevPadAlignment NewAlignment = OriginalAlignment;
				switch (FDevCoreEnums::EnumFromString<EDevPadAlignmentChange>(EnumName))
				{
					case EDevPadAlignmentChange::ToTop:
						NewAlignment = (OriginalAlignment == EDevPadAlignment::TopLeft || OriginalAlignment == EDevPadAlignment::BottomLeft) ? EDevPadAlignment::TopLeft : EDevPadAlignment::TopRight;
						break;
					case EDevPadAlignmentChange::ToBottom:
						NewAlignment = (OriginalAlignment == EDevPadAlignment::TopLeft || OriginalAlignment == EDevPadAlignment::BottomLeft) ? EDevPadAlignment::BottomLeft : EDevPadAlignment::BottomRight;
						break;
					case EDevPadAlignmentChange::ToLeft:
						NewAlignment = (OriginalAlignment == EDevPadAlignment::TopLeft || OriginalAlignment == EDevPadAlignment::TopRight) ? EDevPadAlignment::TopLeft : EDevPadAlignment::BottomLeft;
						break;
					case EDevPadAlignmentChange::ToRight:
						NewAlignment = (OriginalAlignment == EDevPadAlignment::TopLeft || OriginalAlignment == EDevPadAlignment::TopRight) ? EDevPadAlignment::TopRight : EDevPadAlignment::BottomRight;
						break;
					default:
						UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("Invalid enum value: %s"), *EnumName);
						break;
				}
				if (NewAlignment != OriginalAlignment)
				{
					PadWidgetAlignmentAccessor->SetEnum<EDevPadAlignment>(NewAlignment);
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("Invalid argument count. Expected one argument: Alignment Change"));
			}
	}));

	template <class TContentWidgetType = UDevPadContentWidget>
	TSoftClassPtr<TContentWidgetType> GetWidgetClass(const TMap<TSoftClassPtr<UDevPadPage>, TSoftClassPtr<TContentWidgetType>>& WidgetClasses, const TSubclassOf<UDevPadPage> PageClass)
	{
		for (const UClass* CurrentClass = PageClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
		{
			const TSoftClassPtr<UDevPadPage> CurrentClassSoft(CurrentClass);
			if (const TSoftClassPtr<TContentWidgetType>* WidgetClassPtr = WidgetClasses.Find(CurrentClassSoft))
			{
				return *WidgetClassPtr;
			}
		}

		// Fallback to an empty set of class items.
		if (const TSoftClassPtr<TContentWidgetType>* WidgetClassPtr = WidgetClasses.Find(TSoftClassPtr<UDevPadPage>()))
		{
			return *WidgetClassPtr;
		}

		return TSoftClassPtr<TContentWidgetType>();
	}
}

using namespace DevPad::Settings;

UDevPadSettings::UDevPadSettings()
{
	CommonPage = FSoftObjectPath(TEXT("/DevHub/Data/Pad/DP_Common.DP_Common"));
	MainPages =
	{
		TSoftObjectPtr<UDevPadPage>(FSoftObjectPath(TEXT("/DevHub/Data/Pad/Main/DP_Main.DP_Main"))),
		TSoftObjectPtr<UDevPadPage>(FSoftObjectPath(TEXT("/DevHub/Data/Pad/Settings/DP_Settings.DP_Settings"))),
	};

	PadWidgetClass = FSoftObjectPath(TEXT("/DevHub/UI/Widgets/WBP_DevPad.WBP_DevPad_C"));
	InfoWidgetClasses =
	{
		{ TSoftClassPtr<UDevPadPage>(FSoftObjectPath(TEXT("/Script/DevPad.DevPadPage"))), TSoftClassPtr<UDevPadInfoWidget>(FSoftObjectPath(TEXT("/DevHub/UI/Widgets/WBP_DevPad_CommonInfo.WBP_DevPad_CommonInfo_C"))) },
	};
	PageWidgetClasses =
	{
		{ TSoftClassPtr<UDevPadPage>(FSoftObjectPath(TEXT("/Script/DevPad.DevPadControllerPage"))), TSoftClassPtr<UDevPadPageWidget>(FSoftObjectPath(TEXT("/DevHub/UI/Pages/WBP_DevPad_ControllerPage.WBP_DevPad_ControllerPage_C"))) },
	};
 }

TSoftClassPtr<UDevPadInfoWidget> UDevPadSettings::GetInfoWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const
{
	return GetWidgetClass<UDevPadInfoWidget>(InfoWidgetClasses, PageClass);
}

TSoftClassPtr<UDevPadPageWidget> UDevPadSettings::GetPageWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const
{
	return GetWidgetClass<UDevPadPageWidget>(PageWidgetClasses, PageClass);
}

void UDevPadSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (UDevPadSavableSettings* SavableSettings = GetMutableDefault<UDevPadSavableSettings>())
	{
		SavableSettings->Reset();
		SavableSettings->SaveAndNotify();
	}
}

UDevPadSavableSettings::UDevPadSavableSettings()
{
	Reset();
}

UDevPadSavableSettings::FOnSettingsChanged& UDevPadSavableSettings::OnSettingsChanged()
{
	static FOnSettingsChanged OnSettingsChangedDelegate = FOnSettingsChanged();
	return OnSettingsChangedDelegate;
}

void UDevPadSavableSettings::Reset()
{
	if (const UDevPadSettings* Settings = GetDefault<UDevPadSettings>())
	{
		PadWidgetGamepad = Settings->PadWidgetGamepad;
		PadWidgetAlignment = Settings->PadWidgetAlignment;
		PadWidgetScale = Settings->PadWidgetScale;
	}
}

void UDevPadSavableSettings::SaveAndNotify()
{
	SaveConfig();
	OnSettingsChanged().Broadcast(this);
}

void UDevPadSavableSettings::PostInitProperties()
{
	Super::PostInitProperties();
	LoadConfig();
}
