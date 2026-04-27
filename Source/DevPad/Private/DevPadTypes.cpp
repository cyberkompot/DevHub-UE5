// Copyright (c) Alexandr Pereverzev.

#include "DevPadTypes.h"

#include "DevPadSettings.h"

const TArray<UDevPadPage*> UDevPadStack::Empty {};

TSoftClassPtr<UDevPadInfoWidget> UDevPadPage::GetInfoWidgetClass(const UObject* WorldContextObject) const
{
	return GetDefault<UDevPadSettings>()->GetInfoWidgetClass(GetClass());
}

TSoftClassPtr<UDevPadPageWidget> UDevPadPage::GetPageWidgetClass(const UObject* WorldContextObject) const
{
	return GetDefault<UDevPadSettings>()->GetPageWidgetClass(GetClass());
}

TAttribute<FText> UDevPadPage::GetPageTitle() const
{
	const FName PageName = GetPageName();
	return (!PageName.IsNone())
		? FText::FromString(FName::NameToDisplayString(PageName.ToString(), false))
		: TAttribute<FText>();
}
