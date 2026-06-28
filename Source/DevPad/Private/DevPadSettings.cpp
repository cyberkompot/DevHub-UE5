// Copyright (c) Alexandr Pereverzev.

#include "DevPadSettings.h"

#include "DevPadLogging.h"
#include "DevPadTypes.h"

namespace DevPad::Settings
{
	TDevConsoleDynamicConsoleVariable<EDevPadAlignment> PadWidgetAlignmentCVar = TDevConsoleDynamicConsoleVariable<EDevPadAlignment>(TEXT("DevHub.Pad.Settings.Alignment"), TEXT("DevHub Pad HUD widget alignment (0 - TopLeft, 1 - TopRight, 2 - BottomLeft, 3 - BottomRight)"),
		TDevConsoleDynamicConsoleVariable<EDevPadAlignment>::FGetter::CreateLambda([](const UWorld* World)
		{
			return GetDefault<UDevPadSettings>()->PadWidgetAlignment;
		}),
		TDevConsoleDynamicConsoleVariable<EDevPadAlignment>::FSetter::CreateLambda([](const EDevPadAlignment& Value, const UWorld* World)
		{
			GetMutableDefault<UDevPadSettings>()->PadWidgetAlignment = Value;
		}));

	FDevConsoleDynamicFloatConsoleVariable PadWidgetScaleCVar = FDevConsoleDynamicFloatConsoleVariable(TEXT("DevHub.Pad.Settings.Scale"), TEXT("DevHub Pad HUD widget scaling [0.5 - 1.0]"),
		FDevConsoleDynamicFloatConsoleVariable::FGetter::CreateLambda([](UWorld* World)
		{
			return GetDefault<UDevPadSettings>()->PadWidgetScale;
		}),
		FDevConsoleDynamicFloatConsoleVariable::FSetter::CreateLambda([](const float& Value, UWorld* World)
		{
			GetMutableDefault<UDevPadSettings>()->PadWidgetScale = FMath::Clamp(Value, 0.5f, 1.0f);
		}));

	FAutoConsoleCommandWithWorldAndArgs PadWidgetChangeAlignmentCommand(TEXT("DevHub.Pad.Settings.ChangeAlignment"), TEXT("Changes DevHub Pad HUD widget alignment (0 - ToTop, 1 - ToBottom, 2 - ToLeft, 3 - ToRight)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (Args.Num() == 1)
			{
				const FString& EnumName = Args[0];
				const EDevPadAlignment OriginalAlignment = PadWidgetAlignmentCVar->GetEnum<EDevPadAlignment>();
				EDevPadAlignment NewAlignment = OriginalAlignment;
				switch (DevConsole::Implementation::EnumFromString<EDevPadAlignmentChange>(*EnumName))
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
					PadWidgetAlignmentCVar->Set<EDevPadAlignment>(NewAlignment);
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
