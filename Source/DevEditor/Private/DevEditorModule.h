// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "ClassBagCustomization.h"
#include "Modules/ModuleManager.h"

class FAssetTypeActions_Base;

class FDevEditorModule final : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface.
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface.

private:
	FClassBagCustomizations ClassBagCustomizations;
	TSharedPtr<FAssetTypeActions_Base> DevMenuAssetTypeActions = nullptr;
	TSharedPtr<FAssetTypeActions_Base> DevPadPageAssetTypeActions = nullptr;

	bool IsIntegrationAvailable() const { return GIsEditor && !IsRunningCommandlet(); }
};
