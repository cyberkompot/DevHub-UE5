// Copyright (c) Alexandr Pereverzev.

#pragma once

class UObject;

struct FDevConsole final
{
	DEVCONSOLE_API static bool ConsoleCommand(const UObject* WorldContextObject, const FString& Command);

	DEVCONSOLE_API static IConsoleCommand* FindConsoleCommand(const FString& Name);
	DEVCONSOLE_API static IConsoleVariable* FindConsoleVariable(const FString& Name);

	DEVCONSOLE_API static UWorld* FindCurrentPlayWorld();
};