// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "HAL/IConsoleManager.h"

struct FDevConsoleLibrary final
{
	static FDevConsoleLibrary& Get();

	IConsoleCommand* FindConsoleCommand(const FString& Name);
	IConsoleVariable* FindConsoleVariable(const FString& Name);

	void Reset();

private:
	TMap<FName, TUniquePtr<IConsoleObject>> ProxyCommandCache;
};
