// Copyright (c) Alexandr Pereverzev.

#include "DevPadLogging.h"

#include "DevActionTypes.h"
#include "DevPadTypes.h"
#include "Pages/DevPadControllerPage.h"

DEFINE_LOG_CATEGORY(LogDevPad)

FString DevPad::Logging::ControllerPageActionViewToLog(const UObject* WorldContextObject, const FDevPadControllerPageConstActionView& InActionView)
{
	if (!InActionView.IsValid()) { return TEXT("<null>"); }
	FString ActionLabel = InActionView.GetActionPtr()->GetActionLabel(WorldContextObject).Get().ToString();
	if (ActionLabel.IsEmpty()) { ActionLabel = TEXT("None"); }
	return FString::Printf(TEXT("%s.%s (%s)"),
		*PageToLog(InActionView.GetPagePtr()),
		*ActionLabel,
		*GetNameSafe(InActionView.GetScriptStruct()));
};

FString DevPad::Logging::PageToLog(const UDevPadPage* InPage)
{
	if (!InPage) { return TEXT("<null>"); }
	return (!InPage->GetPageName().IsNone()) ? InPage->GetPageName().ToString() : TEXT("None");
}

FString DevPad::Logging::SoftObjectPathToPackageLog(const FSoftObjectPath& InSoftObjectPath)
{
	if (InSoftObjectPath.IsNull()) { return TEXT("<null>"); }
	return InSoftObjectPath.GetLongPackageName();
};
