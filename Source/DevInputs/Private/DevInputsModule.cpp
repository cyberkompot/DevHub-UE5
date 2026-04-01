// Copyright (c) Alexandr Pereverzev.

#include "DevInputsModule.h"

#include "DevInputTypes.h"

IMPLEMENT_MODULE(FDevInputsModule, DevInputs)

void FDevInputsModule::StartupModule()
{
	FModuleManager::Get().LoadModuleChecked(TEXT("InputCore"));

	EDevInputTokens::Initialise();
}
