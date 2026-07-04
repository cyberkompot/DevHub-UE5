// Copyright (c) Alexandr Pereverzev.

#include "DevActionInternals.h"

#include "DevActions.h"
#include "DevActionConsoleAccessor.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogDevActions)

IConsoleVariable* FDevActionInternals::FindConsoleVariableOrAccessor(const FString& Name)
{
	IConsoleVariable* CVar = FDevActions::FindConsoleVariable(Name);
	return (CVar) ? CVar :FDevActions:: FindConsoleAccessor(Name);
}

IMPLEMENT_MODULE(FDefaultModuleImpl, DevActions)
