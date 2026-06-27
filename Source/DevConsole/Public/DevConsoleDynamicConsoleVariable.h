// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "HAL/IConsoleManager.h"

class UWorld;

template<typename TVariableType>
class TDevConsoleDynamicConsoleVariable
{
public:
	using FGetter = TDelegate<TVariableType()>;
	using FSetter = TDelegate<void(const TVariableType&)>;

	TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, uint32 Flags = ECVF_Default);
	~TDevConsoleDynamicConsoleVariable();

	TDevConsoleDynamicConsoleVariable(const TDevConsoleDynamicConsoleVariable&) = delete;
	TDevConsoleDynamicConsoleVariable& operator =(const TDevConsoleDynamicConsoleVariable&) = delete;

	TDevConsoleDynamicConsoleVariable(TDevConsoleDynamicConsoleVariable&& Other);
	TDevConsoleDynamicConsoleVariable& operator =(TDevConsoleDynamicConsoleVariable&& Other);

	FORCEINLINE IConsoleVariable* operator->() { return (RegisteredObject) ? RegisteredObject->AsVariable() : nullptr; }
	FORCEINLINE const IConsoleVariable* operator->() const { return (RegisteredObject) ? RegisteredObject->AsVariable() : nullptr; }
	FORCEINLINE IConsoleVariable& operator*() { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }
	FORCEINLINE const IConsoleVariable& operator*() const { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }

	FORCEINLINE bool IsRegistered() const { return RegisteredObject != nullptr; }

private:
	IConsoleObject* RegisteredObject = nullptr;
};

using FDevConsoleDynamicBoolConsoleVariable = TDevConsoleDynamicConsoleVariable<bool>;
using FDevConsoleDynamicIntConsoleVariable = TDevConsoleDynamicConsoleVariable<int32>;
using FDevConsoleDynamicFloatConsoleVariable = TDevConsoleDynamicConsoleVariable<float>;
using FDevConsoleDynamicStringConsoleVariable = TDevConsoleDynamicConsoleVariable<FString>;

template<typename TVariableType>
class TDevConsoleDynamicConsoleVariableWithWorld
{
public:
	using FGetter = TDelegate<TVariableType(UWorld*)>;
	using FSetter = TDelegate<void(UWorld*, const TVariableType&)>;

	TDevConsoleDynamicConsoleVariableWithWorld(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, uint32 Flags = ECVF_Default);
	~TDevConsoleDynamicConsoleVariableWithWorld();

	TDevConsoleDynamicConsoleVariableWithWorld(const TDevConsoleDynamicConsoleVariableWithWorld&) = delete;
	TDevConsoleDynamicConsoleVariableWithWorld& operator=(const TDevConsoleDynamicConsoleVariableWithWorld&) = delete;

	TDevConsoleDynamicConsoleVariableWithWorld(TDevConsoleDynamicConsoleVariableWithWorld&& Other);
	TDevConsoleDynamicConsoleVariableWithWorld& operator=(TDevConsoleDynamicConsoleVariableWithWorld&& Other);

	FORCEINLINE IConsoleVariable* operator->() { return (RegisteredObject) ? RegisteredObject->AsVariable() : nullptr; }
	FORCEINLINE const IConsoleVariable* operator->() const { return (RegisteredObject) ? RegisteredObject->AsVariable() : nullptr; }
	FORCEINLINE IConsoleVariable& operator*() { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }
	FORCEINLINE const IConsoleVariable& operator*() const { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }

	FORCEINLINE bool IsRegistered() const { return RegisteredObject != nullptr; }

private:
	IConsoleObject* RegisteredObject = nullptr;
};

using FDevConsoleDynamicBoolConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<bool>;
using FDevConsoleDynamicIntConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<int32>;
using FDevConsoleDynamicFloatConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<float>;
using FDevConsoleDynamicStringConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<FString>;
