// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevConsole.h"
#include "HAL/IConsoleManager.h"
#include "UObject/Class.h"

class UWorld;

#define UE_API DEVCONSOLE_API

struct IDevConsoleVariableExt : IConsoleVariable
{
	using IConsoleVariable::Set;
	using IConsoleVariable::SetWithCurrentPriority;

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	TEnumType GetEnum() const
	{
		return static_cast<TEnumType>(GetInt());
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	void Set(TEnumType InValue, EConsoleVariableFlags SetBy = ECVF_SetByCode)
	{
		Set(static_cast<int32>(InValue), SetBy);
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	void SetWithCurrentPriority(TEnumType InValue)
	{
		SetWithCurrentPriority(static_cast<int32>(InValue));
	}
};

struct UE_API FDevConsoleVariableBase : FNoncopyable
{
	FDevConsoleVariableBase(IDevConsoleVariableExt* InRegisteredObject);
	~FDevConsoleVariableBase();

	FDevConsoleVariableBase(FDevConsoleVariableBase&& Other);
	FDevConsoleVariableBase& operator =(FDevConsoleVariableBase&& Other);

	FORCEINLINE IDevConsoleVariableExt* operator->() { return RegisteredObject; }
	FORCEINLINE const IDevConsoleVariableExt* operator->() const { return RegisteredObject; }

	FORCEINLINE IDevConsoleVariableExt& operator*() { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject; }
	FORCEINLINE const IDevConsoleVariableExt& operator*() const { checkf(RegisteredObject, TEXT("Dereferencing unregistered Console Variable.")); return *RegisteredObject; }

	FORCEINLINE bool IsRegistered() const { return RegisteredObject != nullptr; }

protected:
	static IDevConsoleVariableExt* AddConsoleObject(const TCHAR* Name, IDevConsoleVariableExt* Obj);

private:
	IDevConsoleVariableExt* RegisteredObject = nullptr;
};

/** Console variable for an enum type. */
template<typename TEnumType>
struct TDevConsoleEnumConsoleVariable : FDevConsoleVariableBase
{
	TDevConsoleEnumConsoleVariable(const TCHAR* Name, const TCHAR* Help, const TEnumType& DefaultValue, const uint32 Flags = ECVF_Default);
};

/** Console variable for an enum type. */
template<typename TEnumType>
struct TDevConsoleEnumConsoleVariableRef : FDevConsoleVariableBase
{
	TDevConsoleEnumConsoleVariableRef(const TCHAR* Name, const TCHAR* Help, TEnumType& RefValue, const uint32 Flags = ECVF_Default);
};

/** Console variable with getter and setter. */
template<typename TVariableType>
class TDevConsoleDynamicConsoleVariable : public FDevConsoleVariableBase
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(TVariableType, FGetter, UWorld*);
	DECLARE_DELEGATE_TwoParams(FSetter, const TVariableType&, UWorld*);

	TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags = ECVF_Default);
};

using FDevConsoleDynamicBoolConsoleVariable = TDevConsoleDynamicConsoleVariable<bool>;
using FDevConsoleDynamicIntConsoleVariable = TDevConsoleDynamicConsoleVariable<int32>;
using FDevConsoleDynamicFloatConsoleVariable = TDevConsoleDynamicConsoleVariable<float>;
using FDevConsoleDynamicStringConsoleVariable = TDevConsoleDynamicConsoleVariable<FString>;

namespace DevConsole::Implementation
{
	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	static TEnumType EnumFromString(const TCHAR* InValue)
	{
		const int64 ByName = StaticEnum<TEnumType>()->GetValueByNameString(FString(InValue), EGetByNameFlags::None);
		return (ByName != INDEX_NONE) ? static_cast<TEnumType>(ByName) : static_cast<TEnumType>(FCString::Atoi(InValue));
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	static FString EnumToString(TEnumType Value)
	{
		return StaticEnum<TEnumType>()->GetNameStringByValue(static_cast<int64>(Value));
	}

	struct FDevConsoleVariableImplBase : IDevConsoleVariableExt
	{
		FDevConsoleVariableImplBase(const TCHAR* InHelp, const uint32 InFlags)
			: Help(InHelp), Flags(static_cast<EConsoleVariableFlags>(InFlags)) {}

		virtual const TCHAR* GetHelp() const override { return *Help; }
		virtual void SetHelp(const TCHAR* InHelp) override { Help = InHelp; }
		virtual EConsoleVariableFlags GetFlags() const override { return Flags; }
		virtual void SetFlags(const EConsoleVariableFlags InFlags) override { Flags = InFlags; }
		virtual IConsoleVariable* AsVariable() override { return this; }
		virtual void SetOnChangedCallback(const FConsoleVariableDelegate& Callback) override { OnChangedSingle = Callback; }
		virtual FConsoleVariableMulticastDelegate& OnChangedDelegate() override { return OnChangedMulticast; }

	protected:
		void NotifyChanged(const EConsoleVariableFlags SetBy)
		{
			SetFlags(static_cast<EConsoleVariableFlags>((GetFlags() & ~ECVF_SetByMask) | SetBy));
			OnChangedSingle.ExecuteIfBound(this);
			OnChangedMulticast.Broadcast(this);
		}

	private:
		virtual void Release() override { delete this; }

		FString Help;
		EConsoleVariableFlags Flags;
		FConsoleVariableDelegate OnChangedSingle;
		FConsoleVariableMulticastDelegate OnChangedMulticast;
	};

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	struct TDevConsoleEnumConsoleVariableRefImpl : FDevConsoleVariableImplBase
	{
		TDevConsoleEnumConsoleVariableRefImpl(const TCHAR* InHelp, const uint32 InFlags, TEnumType& InRefValue)
			: FDevConsoleVariableImplBase(InHelp, InFlags), RefValue(InRefValue) {}

		virtual bool IsVariableBool() const override { return false; }
		virtual bool IsVariableInt() const override { return true; }
		virtual bool IsVariableFloat() const override { return false; }
		virtual bool IsVariableString() const override { return false; }

		virtual bool GetBool() const override { return GetInt() != 0; }
		virtual int32 GetInt() const override { return static_cast<int32>(RefValue); }
		virtual float GetFloat() const override { return static_cast<float>(static_cast<int32>(RefValue)); }
		virtual FString GetString() const override { return EnumToString(RefValue); }

		virtual void Set(const TCHAR* InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode) override
		{
			RefValue = EnumFromString<TEnumType>(InValue);
			NotifyChanged(SetBy);
		}

	private:
		TEnumType& RefValue;
	};

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	struct TDevConsoleEnumConsoleVariableImpl final : TDevConsoleEnumConsoleVariableRefImpl<TEnumType>
	{
		TDevConsoleEnumConsoleVariableImpl(const TCHAR* InHelp, const uint32 InFlags, const TEnumType& InDefaultValue)
			: TDevConsoleEnumConsoleVariableRefImpl<TEnumType>(InHelp, InFlags, InitValue(Value, InDefaultValue)) {}

	private:
		TEnumType Value;

		static TEnumType& InitValue(TEnumType& ReferenceValue, const TEnumType& InDefaultValue)
		{
			ReferenceValue = InDefaultValue;
			return ReferenceValue;
		}
	};

	template<typename TVariableType, typename = typename TEnableIf<
		TIsSame<TVariableType, bool>::Value || TIsSame<TVariableType, int32>::Value ||
		TIsSame<TVariableType, float>::Value || TIsSame<TVariableType, FString>::Value || TIsEnum<TVariableType>::Value>::Type>
	struct TDevConsoleDynamicConsoleVariableImpl final : FDevConsoleVariableImplBase
	{
		using FGetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FGetter;
		using FSetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FSetter;

		TDevConsoleDynamicConsoleVariableImpl(const TCHAR* InHelp, const uint32 InFlags, const FGetter& InGetter, const FSetter& InSetter)
			: FDevConsoleVariableImplBase(InHelp, InFlags), Getter(InGetter), Setter(InSetter) {}

		virtual bool IsVariableBool() const override { return static_cast<bool>(TIsSame<TVariableType, bool>::Value); }
		virtual bool IsVariableInt() const override { return static_cast<bool>(TIsSame<TVariableType, int32>::Value || TIsEnum<TVariableType>::Value); }
		virtual bool IsVariableFloat() const override { return static_cast<bool>(TIsSame<TVariableType, float>::Value); }
		virtual bool IsVariableString() const override { return static_cast<bool>(TIsSame<TVariableType, FString>::Value); }

		virtual bool GetBool() const override
		{
			if (!Getter.IsBound()) { return false; }
			UWorld* World = FDevConsole::FindCurrentPlayWorld();
			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(World); }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return (Getter.Execute(World) != 0); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return (Getter.Execute(World) != 0.0f); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return (FCString::Atoi(*Getter.Execute(World)) != 0); }
			if constexpr (TIsEnum<TVariableType>::Value) { return (static_cast<int32>(Getter.Execute(World)) != 0); }
			return false;
		}

		virtual int32 GetInt() const override
		{
			if (!Getter.IsBound()) { return 0; }
			UWorld* World = FDevConsole::FindCurrentPlayWorld();
			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(World) ? 1 : 0; }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return Getter.Execute(World); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return static_cast<int32>(Getter.Execute(World)); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atoi(*Getter.Execute(World)); }
			if constexpr (TIsEnum<TVariableType>::Value) { return static_cast<int32>(Getter.Execute(World)); }
			return 0;
		}

		virtual float GetFloat() const override
		{
			if (!Getter.IsBound()) { return 0.0f; }
			UWorld* World = FDevConsole::FindCurrentPlayWorld();
			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(World) ? 1.0f : 0.0f; }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return static_cast<float>(Getter.Execute(World)); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return Getter.Execute(World); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atof(*Getter.Execute(World)); }
			if constexpr (TIsEnum<TVariableType>::Value) { return static_cast<float>(static_cast<int32>(Getter.Execute(World))); }
			return 0.0f;
		}

		virtual FString GetString() const override
		{
			if (!Getter.IsBound()) { return FString(); }
			UWorld* World = FDevConsole::FindCurrentPlayWorld();
			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(World) ? TEXT("1") : TEXT("0"); }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return FString::FromInt(Getter.Execute(World)); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return FString::Printf(TEXT("%g"), Getter.Execute(World)); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return Getter.Execute(World); }
			if constexpr (TIsEnum<TVariableType>::Value) { return EnumToString(Getter.Execute(World)); }
			return FString();
		}

		virtual void Set(const TCHAR* InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode) override
		{
			if (!Setter.IsBound()) { return; }
			UWorld* World = FDevConsole::FindCurrentPlayWorld();
			if constexpr (TIsSame<TVariableType, bool>::Value) { Setter.Execute(FCString::Atoi(InValue) != 0, World); }
			if constexpr (TIsSame<TVariableType, int32>::Value) { Setter.Execute(FCString::Atoi(InValue), World); }
			if constexpr (TIsSame<TVariableType, float>::Value) { Setter.Execute(FCString::Atof(InValue), World); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { Setter.Execute(FString(InValue), World); }
			if constexpr (TIsEnum<TVariableType>::Value) { Setter.Execute(EnumFromString<TVariableType>(InValue), World); }
			NotifyChanged(SetBy);
		}

	private:
		FGetter Getter;
		FSetter Setter;
	};
}

template<typename TEnumType>
TDevConsoleEnumConsoleVariable<TEnumType>::TDevConsoleEnumConsoleVariable(const TCHAR* Name, const TCHAR* Help, const TEnumType& DefaultValue, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new DevConsole::Implementation::TDevConsoleEnumConsoleVariableImpl<TEnumType>(Help, Flags, DefaultValue))) {}

template<typename TEnumType>
TDevConsoleEnumConsoleVariableRef<TEnumType>::TDevConsoleEnumConsoleVariableRef(const TCHAR* Name, const TCHAR* Help, TEnumType& RefValue, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new DevConsole::Implementation::TDevConsoleEnumConsoleVariableRefImpl<TEnumType>(Help, Flags, RefValue))) {}

template<typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>::TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new DevConsole::Implementation::TDevConsoleDynamicConsoleVariableImpl<TVariableType>(Help, Flags, Forward<FGetter>(Getter), Forward<FSetter>(Setter)))) {}

#undef UE_API
