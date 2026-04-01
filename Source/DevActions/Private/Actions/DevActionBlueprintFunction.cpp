// Copyright (c) Alexandr Pereverzev.

#include "Actions/DevActionBlueprintFunction.h"

#include "DevActionLogging.h"
#include "Engine/Blueprint.h"

void FDevActionBlueprintFunction::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (Blueprint.IsNull())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Blueprint is not defined"));
		return;
	}

	if (!FunctionName.IsValid())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Function name is not defined or invalid"));
		return;
	}

	const UBlueprint* BlueprintInstance = Blueprint.LoadSynchronous();
	if (!BlueprintInstance)
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Blueprint asset is missing"));
		return;
	}

	const TSubclassOf<UObject> GeneratedClass = BlueprintInstance->GeneratedClass;
	if (!GeneratedClass)
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Blueprint generated class is missing"));
		return;
	}

	UFunction* Function = GeneratedClass->FindFunctionByName(FunctionName);
	if (!Function)
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Functions '%s' was not found in the Blueprint '%s' of the class '%s'"), *FunctionName.ToString(), *Blueprint->GetFullName(), *GeneratedClass->GetName());
		return;
	}

	const bool bIsSupportedSignature = (Function->HasAllFunctionFlags(FUNC_Public) && !Function->GetReturnProperty() && Function->NumParms == 0);
	if (!bIsSupportedSignature)
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Functions '%s' in the Blueprint '%s' of the class '%s' has an unsupported signature. The function must be public, have no parameters, and no return value"), *Function->GetPathName(), *Blueprint->GetFullName(), *GeneratedClass->GetName());
	}

	GeneratedClass->ProcessEvent(Function, nullptr);
}
