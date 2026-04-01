// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "HAL/IConsoleManager.h"

struct FDevActionConsoleLibrary final
{
	static FDevActionConsoleLibrary& Get();

	IConsoleCommand* FindConsoleCommand(const FString& Name);
	IConsoleVariable* FindConsoleVariable(const FString& Name);

private:
	TArray<TUniquePtr<IConsoleObject>> DummyConsoleObjects;
	TMap<FName, IConsoleObject*> ConsoleObjectsCache;
};
