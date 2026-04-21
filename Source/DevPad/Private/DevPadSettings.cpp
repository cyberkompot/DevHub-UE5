// Copyright (c) Alexandr Pereverzev.

#include "DevPadSettings.h"

#include "DevPadTypes.h"

UDevPadSettings::UDevPadSettings()
{
	CommonPage = FSoftObjectPath(TEXT("/DevHub/Data/Pad/DP_Common.DP_Common"));
	MainPages =
	{
		TSoftObjectPtr<UDevPadPage>(FSoftObjectPath(TEXT("/DevHub/Data/Pad/DP_Main.DP_Main"))),
	};

	PadWidgetClass = FSoftObjectPath(TEXT("/DevHub/UI/Widgets/WBP_DevPad.WBP_DevPad_C"));
	PageWidgetClasses =
	{
		{ TSoftClassPtr<UDevPadPage>(FSoftObjectPath(TEXT("/Script/DevPad.DevPadControllerPage"))), TSoftClassPtr<UDevPadPageWidget>(FSoftObjectPath(TEXT("/DevHub/UI/Pages/WBP_DevPad_ControllerPage.WBP_DevPad_ControllerPage_C"))) },
	};
 }

TSoftClassPtr<UDevPadPageWidget> UDevPadSettings::GetPageWidgetClass(const TSubclassOf<UDevPadPage> PageClass) const
{
	for (const UClass* CurrentClass = PageClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		const TSoftClassPtr<UDevPadPage> CurrentClassSoft(CurrentClass);
		if (const TSoftClassPtr<UDevPadPageWidget>* PageWidgetClassPtr = PageWidgetClasses.Find(CurrentClassSoft))
		{
			return *PageWidgetClassPtr;
		}
	}
	return TSoftClassPtr<UDevPadPageWidget>();
}
