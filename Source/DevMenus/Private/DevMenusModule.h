// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Modules/ModuleManager.h"

class FDevMenusModule final : public IModuleInterface
{
public:
	static FDevMenusModule* Get();

	DECLARE_EVENT(FDevMenusModule, FPrimaryAssetTypeReadyEvent);
	FPrimaryAssetTypeReadyEvent OnPrimaryAssetTypeReady;

	FORCEINLINE bool IsPrimaryAssetTypeReady() const { return bPrimaryAssetTypeReady; }

	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface
private:
	bool bPrimaryAssetTypeReady = false;

	void BroadcastPrimaryAssetTypeReady();

	void OnAssetManagerReady();
	void OnAssetRegistryReady();
};
