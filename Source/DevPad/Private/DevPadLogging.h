// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreLogging.h" // Included for log macros.

struct FDevPadControllerPageActionView;
class UDevPadPage;

DEVPAD_API DECLARE_LOG_CATEGORY_EXTERN(LogDevPad, VeryVerbose, All);

namespace DevPad::Logging
{
	FString ControllerPageActionViewToLog(const UObject* WorldContextObject, const FDevPadControllerPageActionView& InActionView);
	FString PageToLog(const UDevPadPage* InPage);
	FString SoftObjectPathToPackageLog(const FSoftObjectPath& InSoftObjectPath);

	template <class TObjectType = UObject>
	FString SoftObjectPtrToPackageLog(const TSoftObjectPtr<TObjectType>& InSoftObjectPtr) { return SoftObjectPathToPackageLog(InSoftObjectPtr.ToSoftObjectPath()); }
} // namespace DevPad::Logging

using namespace DevCore::Logging;
using namespace DevPad::Logging;
