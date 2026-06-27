// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleInternals.h"
#include "DevConsoleLibrary.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogDevConsole);

class FDevConsoleModule final : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface.
	virtual void ShutdownModule() override
	{
		FDevConsoleLibrary::Get().Reset();
	}
	//~ End IModuleInterface Interface.
};

IMPLEMENT_MODULE(FDevConsoleModule, DevConsole)
