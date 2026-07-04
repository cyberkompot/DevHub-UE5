// Copyright (c) Alexandr Pereverzev.

#pragma once

struct FDevAction;
struct IConsoleAccessor;
struct IConsoleCommand;
class IConsoleVariable;
class UObject;

#define UE_API DEVACTIONS_API

struct FDevActions final
{
	UE_API static void ExecuteAction(const UObject* WorldContextObject, const FDevAction& Action);
	UE_API static bool ExecuteConsoleCommand(const UObject* WorldContextObject, const FString& Command);

	UE_API static IConsoleAccessor* FindConsoleAccessor(const FString& Name);
	UE_API static IConsoleCommand* FindConsoleCommand(const FString& Name);
	UE_API static IConsoleVariable* FindConsoleVariable(const FString& Name);

	UE_API static UWorld* FindCurrentPlayWorld();
};

#undef UE_API
