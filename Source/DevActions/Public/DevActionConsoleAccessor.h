// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActions.h"
#include "DevCoreCompatibility.h"
#include "DevCoreTypeTraits.h"
#include "Framework/DevCoreEnums.h"
#include "HAL/IConsoleManager.h"
#include "Misc/OutputDevice.h"
#include "UObject/Class.h"

class UWorld;

#define UE_API DEVACTIONS_API

struct UE_API IConsoleAccessor : IConsoleVariable
{
	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	TEnumType GetEnum() const
	{
		return static_cast<TEnumType>(GetInt());
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	void SetEnum(TEnumType InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode)
	{
		Set(static_cast<int32>(InValue), SetBy);
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	void SetEnumWithCurrentPriority(TEnumType InValue)
	{
		SetWithCurrentPriority(static_cast<int32>(InValue));
	}
};

struct UE_API FDevActionConsoleAccessorManager final
{
	static FDevActionConsoleAccessorManager& Get();

	IConsoleAccessor* FindConsoleAccessor(const FString& InName) const;

	void RegisterConsoleAccessor(const FString& InName, IConsoleAccessor* InAccessor);
	void UnregisterConsoleAccessor(const FString& InName);

private:
	TMap<FString, IConsoleAccessor*> Accessors;
};

/** Console accessor with getter and setter, exposed to the console as a command. Supports bool, int32, float, FString and enum types. */
template<typename TVariableType, typename = typename TEnableIf<
	TIsSame<TVariableType, bool>::Value || TIsSame<TVariableType, int32>::Value || TIsSame<TVariableType, float>::Value ||
	TIsSame<TVariableType, FName>::Value || TIsSame<TVariableType, FString>::Value || TIsEnum<TVariableType>::Value>::Type>
class TDevActionConsoleAccessor : public IConsoleAccessor, FNoncopyable
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(TVariableType, FGetter, UWorld*);
	DECLARE_DELEGATE_TwoParams(FSetter, const TVariableType&, UWorld*);

	TDevActionConsoleAccessor(const TCHAR* InName, const TCHAR* InHelp, FGetter&& InGetter, FSetter&& InSetter, const uint32 InFlags = ECVF_Default)
		: Getter(Forward<FGetter>(InGetter)), Setter(Forward<FSetter>(InSetter)), Name(InName), Help(InHelp), Flags(InFlags)
	{
		FDevActionConsoleAccessorManager::Get().RegisterConsoleAccessor(Name, this);
		Command = IConsoleManager::Get().RegisterConsoleCommand(InName, InHelp, FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda(
			[this](const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar)
			{
				if (Args.Num() == 0)
				{
					FNameBuilder Builder;
					Builder.Append(Name).Append(TEXT(" = ")).Append(GetString()).AppendChar(TEXT('\0'));
					Ar.Log(Builder.GetData());
				}
				else if (Args.Num() == 1)
				{
					SetWorld(World);
					ON_SCOPE_EXIT { SetWorld(nullptr); };
#if UE_COMPATIBILITY_CONSOLE_VARIABLE_RESOLVED_CONTEXT
					Set(*Args[0], FResolvedContext());
#else
					Set(*Args[0], ECVF_SetByConsole);
#endif
				}
				else
				{
					Ar.Log(ELogVerbosity::Type::Error, TEXT("Invalid arguments count. Expected one argument: Console Variable Value"));
				}
			}),	InFlags);
	}

	virtual ~TDevActionConsoleAccessor() override
	{
		TDevActionConsoleAccessor::Release();
	}

	FORCEINLINE bool IsRegistered() const { return Command != nullptr; }

	FORCEINLINE IConsoleAccessor* operator->() { return this; }
	FORCEINLINE const IConsoleAccessor* operator->() const { return this; }

	FORCEINLINE IConsoleAccessor& operator*() { return *this; }
	FORCEINLINE const IConsoleAccessor& operator*() const { return *this; }

	//~ Begin IConsoleVariable interface.
#if UE_COMPATIBILITY_CONSOLE_VARIABLE_RESOLVED_CONTEXT
	virtual void Set(const TCHAR* InValue, const FResolvedContext& InSetContext) override
#else
	virtual void Set(const TCHAR* InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode) override
#endif
	{
		if (!Setter.IsBound()) { return; }
		if constexpr (TIsSame<TVariableType, bool>::Value) { Setter.Execute(FCString::Atoi(InValue) != 0, GetWorld()); }
		else if constexpr (TIsSame<TVariableType, int32>::Value) { Setter.Execute(FCString::Atoi(InValue), GetWorld()); }
		else if constexpr (TIsSame<TVariableType, float>::Value) { Setter.Execute(FCString::Atof(InValue), GetWorld()); }
		else if constexpr (TIsSame<TVariableType, FName>::Value) { Setter.Execute(FName(InValue), GetWorld()); }
		else if constexpr (TIsSame<TVariableType, FString>::Value) { Setter.Execute(FString(InValue), GetWorld()); }
		else if constexpr (TIsEnum<TVariableType>::Value) { Setter.Execute(FDevCoreEnums::EnumFromString<TVariableType>(InValue), GetWorld()); }
#if UE_COMPATIBILITY_CONSOLE_VARIABLE_RESOLVED_CONTEXT
		NotifyChanged(static_cast<EConsoleVariableFlags>(InSetContext.Flags));
#else
		NotifyChanged(SetBy);
#endif
	}

	virtual bool GetBool() const override
	{
		if (!Getter.IsBound()) { return false; }
		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(GetWorld()); }
		else if constexpr (TIsSame<TVariableType, int32>::Value) { return (Getter.Execute(GetWorld()) != 0); }
		else if constexpr (TIsSame<TVariableType, float>::Value) { return (Getter.Execute(GetWorld()) != 0.0f); }
		else if constexpr (TIsSame<TVariableType, FName>::Value) { return (FCString::Atoi(*Getter.Execute(GetWorld()).ToString()) != 0); }
		else if constexpr (TIsSame<TVariableType, FString>::Value) { return (FCString::Atoi(*Getter.Execute(GetWorld())) != 0); }
		else if constexpr (TIsEnum<TVariableType>::Value) { return (static_cast<int32>(Getter.Execute(GetWorld())) != 0); }
		else { return false; }
	}

	virtual int32 GetInt() const override
	{
		if (!Getter.IsBound()) { return 0; }
		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(GetWorld()) ? 1 : 0; }
		else if constexpr (TIsSame<TVariableType, int32>::Value) { return Getter.Execute(GetWorld()); }
		else if constexpr (TIsSame<TVariableType, float>::Value) { return static_cast<int32>(Getter.Execute(GetWorld())); }
		else if constexpr (TIsSame<TVariableType, FName>::Value) { return FCString::Atoi(*Getter.Execute(GetWorld()).ToString()); }
		else if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atoi(*Getter.Execute(GetWorld())); }
		else if constexpr (TIsEnum<TVariableType>::Value) { return static_cast<int32>(Getter.Execute(GetWorld())); }
		else { return 0; }
	}

	virtual float GetFloat() const override
	{
		if (!Getter.IsBound()) { return 0.0f; }
		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(GetWorld()) ? 1.0f : 0.0f; }
		else if constexpr (TIsSame<TVariableType, int32>::Value) { return static_cast<float>(Getter.Execute(GetWorld())); }
		else if constexpr (TIsSame<TVariableType, float>::Value) { return Getter.Execute(GetWorld()); }
		else if constexpr (TIsSame<TVariableType, FName>::Value) { return FCString::Atof(*Getter.Execute(GetWorld()).ToString()); }
		else if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atof(*Getter.Execute(GetWorld())); }
		else if constexpr (TIsEnum<TVariableType>::Value) { return static_cast<float>(static_cast<int32>(Getter.Execute(GetWorld()))); }
		else { return 0.0f; }
	}

	virtual FString GetString() const override
	{
		if (!Getter.IsBound()) { return FString(); }
		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(GetWorld()) ? TEXT("1") : TEXT("0"); }
		else if constexpr (TIsSame<TVariableType, int32>::Value) { return FString::FromInt(Getter.Execute(GetWorld())); }
		else if constexpr (TIsSame<TVariableType, float>::Value) { return FString::Printf(TEXT("%g"), Getter.Execute(GetWorld())); }
		else if constexpr (TIsSame<TVariableType, FName>::Value) { return Getter.Execute(GetWorld()).ToString(); }
		else if constexpr (TIsSame<TVariableType, FString>::Value) { return Getter.Execute(GetWorld()); }
		else if constexpr (TIsEnum<TVariableType>::Value) { return FDevCoreEnums::EnumToString(Getter.Execute(GetWorld())); }
		else { return FString(); }
	}

	virtual void SetOnChangedCallback(const FConsoleVariableDelegate& Callback) override { OnChangedSingle = Callback; }
	virtual FConsoleVariableMulticastDelegate& OnChangedDelegate() override { return OnChangedMulticast; }

#if UE_COMPATIBILITY_CONSOLE_VARIABLE_RESOLVED_CONTEXT
	virtual FResolvedContext ResolveContext(const FSetContext& Context) override { return FResolvedContext { Context.Flags, Context.Tag }; }
	virtual void Unset(EConsoleVariableFlags SetBy, FName Tag = NAME_None) override { /* Nop */ }
	virtual FString GetDefaultValue() override { return FString(); }
	virtual void LogHistory(FOutputDevice& Ar) override { /* Nop */ }
	virtual SIZE_T GetHistorySize() override { return 0; }
#if ALLOW_OTHER_PLATFORM_CONFIG
	virtual TSharedPtr<IConsoleVariable> GetPlatformValueVariable(FName PlatformName, const FString& DeviceProfileName = FString()) override { return nullptr; }
	virtual bool HasPlatformValueVariable(FName PlatformName, const FString& DeviceProfileName = FString()) override { return false; }
#endif // ALLOW_OTHER_PLATFORM_CONFIG
#endif // UE_COMPATIBILITY_CONSOLE_VARIABLE_RESOLVED_CONTEXT
	//~ End IConsoleVariable interface.

	//~ Begin IConsoleObject interface.
	virtual const TCHAR* GetHelp() const override { return *Help; }
	virtual void SetHelp(const TCHAR* InHelp) override { Help = InHelp; }

	virtual EConsoleVariableFlags GetFlags() const override { return static_cast<EConsoleVariableFlags>(Flags); }
	virtual void SetFlags(const EConsoleVariableFlags InFlags) override { Flags = InFlags; }

	virtual IConsoleVariable* AsVariable() override { return this; }

	virtual bool IsVariableBool() const override { return static_cast<bool>(TIsSame<TVariableType, bool>::Value); }
	virtual bool IsVariableInt() const override { return static_cast<bool>(TIsSame<TVariableType, int32>::Value || TIsEnum<TVariableType>::Value); }
	virtual bool IsVariableFloat() const override { return static_cast<bool>(TIsSame<TVariableType, float>::Value); }
	virtual bool IsVariableString() const override { return static_cast<bool>(TIsSame<TVariableType, FString>::Value || TIsSame<TVariableType, FName>::Value); }
	//~ End IConsoleObject interface.

private:
	FGetter Getter;
	FSetter Setter;

	IConsoleObject* Command = nullptr;
	UWorld* World = nullptr;

	FString Name;
	FString Help;
	uint32 Flags = 0;
	FConsoleVariableDelegate OnChangedSingle;
	FConsoleVariableMulticastDelegate OnChangedMulticast;

	FORCEINLINE UWorld* GetWorld() const { return (World) ? World : FDevActions::FindCurrentPlayWorld(); }
	FORCEINLINE void SetWorld(UWorld* InWorld) { World = InWorld; }

	void NotifyChanged(const EConsoleVariableFlags SetBy)
	{
		SetFlags(static_cast<EConsoleVariableFlags>((GetFlags() & ~ECVF_SetByMask) | SetBy));
		OnChangedSingle.ExecuteIfBound(this);
		OnChangedMulticast.Broadcast(this);
	}

	//~ Begin IConsoleObject interface.
	virtual void Release() override
	{
		FDevActionConsoleAccessorManager::Get().UnregisterConsoleAccessor(Name);
		if (Command)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Command);
			Command = nullptr;
		}
	}
	//~ End IConsoleObject interface.
};

extern template class TDevActionConsoleAccessor<bool>;
extern template class TDevActionConsoleAccessor<int32>;
extern template class TDevActionConsoleAccessor<float>;
extern template class TDevActionConsoleAccessor<FName>;
extern template class TDevActionConsoleAccessor<FString>;

using FDevActionBoolConsoleAccessor = TDevActionConsoleAccessor<bool>;
using FDevActionIntConsoleAccessor = TDevActionConsoleAccessor<int32>;
using FDevActionFloatConsoleAccessor = TDevActionConsoleAccessor<float>;
using FDevActionNameConsoleAccessor = TDevActionConsoleAccessor<FName>;
using FDevActionStringConsoleAccessor = TDevActionConsoleAccessor<FString>;

#undef UE_API
