// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h" // Included for log macros.
#include "UObject/ObjectMacros.h"

class IConsoleVariable;

DECLARE_LOG_CATEGORY_EXTERN(LogDevActions, Warning, All);

struct FDevActionInternals final
{
	static IConsoleVariable* FindConsoleVariableOrAccessor(const FString& Name);
};
