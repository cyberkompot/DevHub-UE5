// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleDynamicConsoleVariable.h"

#include "DevConsole.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "HAL/ConsoleManager.h"

// Usage: DEFINE_PRIVATE_ACCESS_FUNCTION(FMyClass, MyFunction) in global scope, then PrivateAccess::MyFunction(MyObject)(MyArgs) from anywhere
#define DEFINE_PRIVATE_ACCESS_FUNCTION(Class, Function) \
	namespace PrivateAccess \
	{ \
		template<typename> \
		struct TClass_ ## Function; \
		\
		template<> \
		struct TClass_ ## Function<Class> \
		{ \
			template<auto FunctionPtr> \
			struct TFunction_ ## Function \
			{ \
				friend auto Function(Class& Object) \
				{ \
					return [&Object](auto&&... Args) \
					{ \
						return (Object.*FunctionPtr)(Forward<decltype(Args)>(Args)...); \
					}; \
				} \
			}; \
		}; \
		template struct TClass_ ## Function<Class>::TFunction_ ## Function<&Class::Function>; \
		\
		auto Function(Class& Object); \
		auto Function(const Class& Object) \
		{ \
			return Function(const_cast<Class&>(Object)); \
		} \
	}

namespace
{
	DEFINE_PRIVATE_ACCESS_FUNCTION(FConsoleManager, AddConsoleObject)
}

#undef DEFINE_PRIVATE_ACCESS_FUNCTION

/** Heap-allocated TDevConsoleDynamicConsoleVariable implementation. Ownership is transferred to FConsoleManager on registration. */
template<typename TVariableType, typename = typename TEnableIf<
	TIsSame<TVariableType, bool>::Value || TIsSame<TVariableType, int32>::Value ||
	TIsSame<TVariableType, float>::Value  || TIsSame<TVariableType, FString>::Value>::Type>
class TDevConsoleDynamicConsoleVariableImpl final : public IConsoleVariable
{
public:
	using FGetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FGetter;
	using FSetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FSetter;

	TDevConsoleDynamicConsoleVariableImpl(const TCHAR* InHelp, const EConsoleVariableFlags InFlags, const FGetter& InGetter, const FSetter& InSetter)
		: Getter(InGetter), Setter(InSetter), Help(InHelp), Flags(InFlags) {}

	//~ Begin IConsoleVariable interface.
	virtual const TCHAR* GetHelp()  const override { return *Help; }
	virtual void SetHelp(const TCHAR* InHelp) override { Help = InHelp; }

	virtual EConsoleVariableFlags GetFlags() const override { return Flags; }
	virtual void SetFlags(const EConsoleVariableFlags InFlags) override { Flags = InFlags; }

	virtual IConsoleVariable* AsVariable() override { return this; }

	virtual bool IsVariableBool() const override { return static_cast<bool>(TIsSame<TVariableType, bool>::Value); }
	virtual bool IsVariableInt() const override { return static_cast<bool>(TIsSame<TVariableType, int32>::Value); }
	virtual bool IsVariableFloat() const override { return static_cast<bool>(TIsSame<TVariableType, float>::Value); }
	virtual bool IsVariableString() const override { return static_cast<bool>(TIsSame<TVariableType, FString>::Value); }

	virtual bool GetBool() const override
	{
		if (!Getter.IsBound()) { return false; }

		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(); }
		if constexpr (TIsSame<TVariableType, int32>::Value) { return (Getter.Execute() != 0); }
		if constexpr (TIsSame<TVariableType, float>::Value) { return (Getter.Execute() != 0.0f); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { return (FCString::Atoi(*Getter.Execute()) != 0); }

		return false;
	}

	virtual int32 GetInt() const override
	{
		if (!Getter.IsBound()) { return 0; }

		if constexpr (TIsSame<TVariableType, bool>::Value)  { return Getter.Execute() ? 1 : 0; }
		if constexpr (TIsSame<TVariableType, int32>::Value) { return Getter.Execute(); }
		if constexpr (TIsSame<TVariableType, float>::Value) { return static_cast<int32>(Getter.Execute()); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atoi(*Getter.Execute()); }

		return 0;
	}

	virtual float GetFloat() const override
	{
		if (!Getter.IsBound()) { return 0.0f; }

		if constexpr (TIsSame<TVariableType, bool>::Value)  { return Getter.Execute() ? 1.0f : 0.0f; }
		if constexpr (TIsSame<TVariableType, int32>::Value) { return static_cast<float>(Getter.Execute()); }
		if constexpr (TIsSame<TVariableType, float>::Value) { return Getter.Execute(); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atof(*Getter.Execute()); }

		return 0.0f;
	}

	virtual FString GetString() const override
	{
		if (!Getter.IsBound()) { return FString(); }

		if constexpr (TIsSame<TVariableType, bool>::Value)  { return Getter.Execute() ? TEXT("1") : TEXT("0"); }
		if constexpr (TIsSame<TVariableType, int32>::Value) { return FString::FromInt(Getter.Execute()); }
		if constexpr (TIsSame<TVariableType, float>::Value) { return FString::Printf(TEXT("%g"), Getter.Execute()); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { return Getter.Execute(); }

		return FString();
	}

	virtual void Set(const TCHAR* InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode) override
	{
		if (!Setter.IsBound()) { return; }

		if constexpr (TIsSame<TVariableType, bool>::Value) { Setter.Execute(FCString::Atoi(InValue) != 0); }
		if constexpr (TIsSame<TVariableType, int32>::Value) { Setter.Execute(FCString::Atoi(InValue)); }
		if constexpr (TIsSame<TVariableType, float>::Value) { Setter.Execute(FCString::Atof(InValue)); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { Setter.Execute(FString(InValue)); }

		SetFlags(static_cast<EConsoleVariableFlags>((GetFlags() & ~ECVF_SetByMask) | SetBy));

		OnChangedSingle.ExecuteIfBound(this);
		OnChangedMulticast.Broadcast(this);
	}

	virtual void SetOnChangedCallback(const FConsoleVariableDelegate& Callback) override { OnChangedSingle = Callback; }

	virtual FConsoleVariableMulticastDelegate& OnChangedDelegate() override { return OnChangedMulticast; }
	//~ End IConsoleVariable interface.

private:
	virtual void Release() override {} // FConsoleManager::~FConsoleManager deletes this object directly.

	FGetter Getter;
	FSetter Setter;
	FString Help;
	EConsoleVariableFlags Flags;
	FConsoleVariableDelegate OnChangedSingle;
	FConsoleVariableMulticastDelegate OnChangedMulticast;
};


/** Heap-allocated TDevConsoleDynamicConsoleVariableWithWorld implementation. Ownership is transferred to FConsoleManager on registration. */
template<typename TVariableType, typename = typename TEnableIf<
	TIsSame<TVariableType, bool>::Value || TIsSame<TVariableType, int32>::Value ||
	TIsSame<TVariableType, float>::Value  || TIsSame<TVariableType, FString>::Value>::Type>
class TDevConsoleDynamicConsoleVariableWithWorldImpl final : public IConsoleVariable
{
public:
	using FGetter = typename TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::FGetter;
	using FSetter = typename TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::FSetter;

	TDevConsoleDynamicConsoleVariableWithWorldImpl(const TCHAR* InHelp, const EConsoleVariableFlags InFlags, FGetter&& InGetter, FSetter&& InSetter)
		: Getter(Forward<FGetter>(InGetter)), Setter(Forward<FSetter>(InSetter)), Help(InHelp), Flags(InFlags) {}

	//~ Begin IConsoleVariable interface.
	virtual const TCHAR* GetHelp()  const override { return *Help; }
	virtual void SetHelp(const TCHAR* InHelp) override { Help = InHelp; }

	virtual EConsoleVariableFlags GetFlags() const override { return Flags; }
	virtual void SetFlags(const EConsoleVariableFlags InFlags) override { Flags = InFlags; }

	virtual IConsoleVariable* AsVariable() override { return this; }

	virtual bool IsVariableBool() const override { return static_cast<bool>(TIsSame<TVariableType, bool>::Value); }
	virtual bool IsVariableInt() const override { return static_cast<bool>(TIsSame<TVariableType, int32>::Value); }
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

		return 0.0f;;
	}

	virtual FString GetString() const override
	{
		if (!Getter.IsBound()) { return FString(); }

		UWorld* World = FDevConsole::FindCurrentPlayWorld();

		if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute(World) ? TEXT("1") : TEXT("0"); }
		if constexpr (TIsSame<TVariableType, int32>::Value) { return FString::FromInt(Getter.Execute(World)); }
		if constexpr (TIsSame<TVariableType, float>::Value) { return FString::Printf(TEXT("%g"), Getter.Execute(World)); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { return Getter.Execute(World); }

		return FString();
	}

	virtual void Set(const TCHAR* InValue, const EConsoleVariableFlags SetBy = ECVF_SetByCode) override
	{
		if (!Setter.IsBound()) { return; }

		UWorld* World = FDevConsole::FindCurrentPlayWorld();

		if constexpr (TIsSame<TVariableType, bool>::Value) { Setter.Execute(World, FCString::Atoi(InValue) != 0); }
		if constexpr (TIsSame<TVariableType, int32>::Value) { Setter.Execute(World, FCString::Atoi(InValue)); }
		if constexpr (TIsSame<TVariableType, float>::Value) { Setter.Execute(World, FCString::Atof(InValue)); }
		if constexpr (TIsSame<TVariableType, FString>::Value) { Setter.Execute(World, FString(InValue)); }

		SetFlags(static_cast<EConsoleVariableFlags>((GetFlags() & ~ECVF_SetByMask) | SetBy));

		OnChangedSingle.ExecuteIfBound(this);
		OnChangedMulticast.Broadcast(this);
	}

	virtual void SetOnChangedCallback(const FConsoleVariableDelegate& Callback) override { OnChangedSingle = Callback; }

	virtual FConsoleVariableMulticastDelegate& OnChangedDelegate() override { return OnChangedMulticast; }
	//~ End IConsoleVariable interface.

private:
	virtual void Release() override {} // FConsoleManager::~FConsoleManager deletes this object directly.

	FGetter Getter;
	FSetter Setter;
	FString Help;
	EConsoleVariableFlags Flags;
	FConsoleVariableDelegate OnChangedSingle;
	FConsoleVariableMulticastDelegate OnChangedMulticast;
};

template <typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>::TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, uint32 Flags)
{
	if (!IConsoleManager::Get().FindConsoleObject(Name, false)) // Check that no console object with this name is already registered.
	{
		FConsoleManager& ConsoleManagerImpl = static_cast<FConsoleManager&>(IConsoleManager::Get());
		TDevConsoleDynamicConsoleVariableImpl<TVariableType>* ConsoleVariableImpl = new TDevConsoleDynamicConsoleVariableImpl<TVariableType>(Help, static_cast<EConsoleVariableFlags>(Flags), Forward<FGetter>(Getter), Forward<FSetter>(Setter));
		RegisteredObject = PrivateAccess::AddConsoleObject(ConsoleManagerImpl)(Name, ConsoleVariableImpl);
	}
	else
	{
		// ERROR
	}
}

template<typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>::~TDevConsoleDynamicConsoleVariable()
{
	if (RegisteredObject)
	{
		IConsoleManager::Get().UnregisterConsoleObject(RegisteredObject);
	}
}

template <typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>::TDevConsoleDynamicConsoleVariable(TDevConsoleDynamicConsoleVariable&& Other)
	: RegisteredObject(Other.RegisteredObject)
{
	Other.RegisteredObject = nullptr;
}

template <typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>& TDevConsoleDynamicConsoleVariable<TVariableType>::operator=(TDevConsoleDynamicConsoleVariable&& Other)
{
	if (this != &Other)
	{
		if (RegisteredObject)
		{
			IConsoleManager::Get().UnregisterConsoleObject(RegisteredObject);
		}
		RegisteredObject = Other.RegisteredObject;
		Other.RegisteredObject = nullptr;
	}
	return *this;
}


template <typename TVariableType>
TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::TDevConsoleDynamicConsoleVariableWithWorld(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, uint32 Flags)
{
	if (!IConsoleManager::Get().FindConsoleObject(Name, false)) // Check that no console object with this name is already registered.
	{
		FConsoleManager& ConsoleManagerImpl = static_cast<FConsoleManager&>(IConsoleManager::Get());
		TDevConsoleDynamicConsoleVariableWithWorldImpl<TVariableType>* ConsoleVariableImpl = new TDevConsoleDynamicConsoleVariableWithWorldImpl<TVariableType>(Help, static_cast<EConsoleVariableFlags>(Flags), Forward<FGetter>(Getter), Forward<FSetter>(Setter));
		RegisteredObject = PrivateAccess::AddConsoleObject(ConsoleManagerImpl)(Name, ConsoleVariableImpl);
	}
	else
	{
		// ERROR
	}
}

template<typename TVariableType>
TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::~TDevConsoleDynamicConsoleVariableWithWorld()
{
	if (RegisteredObject)
	{
		IConsoleManager::Get().UnregisterConsoleObject(RegisteredObject);
	}
}

template <typename TVariableType>
TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::TDevConsoleDynamicConsoleVariableWithWorld(TDevConsoleDynamicConsoleVariableWithWorld&& Other)
	: RegisteredObject(Other.RegisteredObject)
{
	Other.RegisteredObject = nullptr;
}

template <typename TVariableType>
TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>& TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::operator=(TDevConsoleDynamicConsoleVariableWithWorld&& Other)
{
	if (this != &Other)
	{
		if (RegisteredObject)
		{
			IConsoleManager::Get().UnregisterConsoleObject(RegisteredObject);
		}
		RegisteredObject = Other.RegisteredObject;
		Other.RegisteredObject = nullptr;
	}
	return *this;
}
