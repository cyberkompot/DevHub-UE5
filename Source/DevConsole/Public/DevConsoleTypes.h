// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "HAL/IConsoleManager.h"

class UWorld;

#define UE_API DEVCONSOLE_API

struct FDevConsoleVariableBase : FNoncopyable
{
	FDevConsoleVariableBase(IConsoleObject* InRegisteredObject);
	~FDevConsoleVariableBase();

	FDevConsoleVariableBase(FDevConsoleVariableBase&& Other);
	FDevConsoleVariableBase& operator =(FDevConsoleVariableBase&& Other);

	FORCEINLINE IConsoleVariable* operator->() { return RegisteredObject ? RegisteredObject->AsVariable() : nullptr; }
	FORCEINLINE const IConsoleVariable* operator->() const { return RegisteredObject ? RegisteredObject->AsVariable() : nullptr; }

	FORCEINLINE IConsoleVariable& operator*() { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }
	FORCEINLINE const IConsoleVariable& operator*() const { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject->AsVariable(); }

	FORCEINLINE bool IsRegistered() const { return RegisteredObject != nullptr; }

private:
	IConsoleObject* RegisteredObject = nullptr;
};

/** Console variable for an enum type. */
template<typename TEnumType>
struct UE_API TDevConsoleEnumConsoleVariable : FDevConsoleVariableBase
{
	TDevConsoleEnumConsoleVariable(const TCHAR* Name, const TCHAR* Help, const TEnumType& DefaultValue, const uint32 Flags = ECVF_Default);
};

/** Console variable for an enum type. */
template<typename TEnumType>
struct UE_API TDevConsoleEnumConsoleVariableRef : FDevConsoleVariableBase
{
	TDevConsoleEnumConsoleVariableRef(const TCHAR* Name, const TCHAR* Help, TEnumType& RefValue, const uint32 Flags = ECVF_Default);
};

/** Console variable with getter and setter. */
template<typename TVariableType>
class UE_API TDevConsoleDynamicConsoleVariable : public FDevConsoleVariableBase
{
public:
	DECLARE_DELEGATE_RetVal(TVariableType, FGetter);
	DECLARE_DELEGATE_OneParam(FSetter, const TVariableType&);

	TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags = ECVF_Default);
};

/** Console variable with getter, setter, and access to the world. */
template<typename TVariableType>
class UE_API TDevConsoleDynamicConsoleVariableWithWorld : public FDevConsoleVariableBase
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(TVariableType, FGetter, UWorld*);
	DECLARE_DELEGATE_TwoParams(FSetter, UWorld*, const TVariableType&);

	TDevConsoleDynamicConsoleVariableWithWorld(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags = ECVF_Default);
};

using FDevConsoleDynamicBoolConsoleVariable = TDevConsoleDynamicConsoleVariable<bool>;
using FDevConsoleDynamicIntConsoleVariable = TDevConsoleDynamicConsoleVariable<int32>;
using FDevConsoleDynamicFloatConsoleVariable = TDevConsoleDynamicConsoleVariable<float>;
using FDevConsoleDynamicStringConsoleVariable = TDevConsoleDynamicConsoleVariable<FString>;

using FDevConsoleDynamicBoolConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<bool>;
using FDevConsoleDynamicIntConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<int32>;
using FDevConsoleDynamicFloatConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<float>;
using FDevConsoleDynamicStringConsoleVariableWithWorld = TDevConsoleDynamicConsoleVariableWithWorld<FString>;

#undef UE_API
