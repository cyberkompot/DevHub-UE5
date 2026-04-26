// Copyright (c) Alexandr Pereverzev.

#include "DevPadSettings.h"

#include "DevPadTypes.h"

namespace
{
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
