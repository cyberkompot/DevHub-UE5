// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;

class FDevEditorModule final : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface

private:
	TSharedPtr<FAssetTypeActions_Base> DevMenuAssetTypeActions;

	bool IsIntegrationAvailable() const { return GIsEditor && !IsRunningCommandlet(); }
};
