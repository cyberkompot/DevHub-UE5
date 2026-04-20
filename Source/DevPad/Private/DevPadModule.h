// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Modules/ModuleManager.h"

class FDevPadModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	//~ End IModuleInterface Interface
};
