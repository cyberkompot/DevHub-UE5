// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleTypes.h"

#include "DevConsole.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "HAL/ConsoleManager.h"
#include "UObject/Class.h"

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

namespace DevConsole::Implementation
{
	DEFINE_PRIVATE_ACCESS_FUNCTION(FConsoleManager, AddConsoleObject)

	IConsoleObject* AddConsoleObject(const TCHAR* Name, IConsoleObject* Obj)
	{
		FConsoleManager& ConsoleManager = static_cast<FConsoleManager&>(IConsoleManager::Get());
		return PrivateAccess::AddConsoleObject(ConsoleManager)(Name, Obj);
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	TEnumType EnumFromString(const TCHAR* InValue)
	{
		const int64 ByName = StaticEnum<TEnumType>()->GetValueByNameString(FString(InValue), EGetByNameFlags::None);
		return (ByName != INDEX_NONE) ? static_cast<TEnumType>(ByName) : static_cast<TEnumType>(FCString::Atoi(InValue));
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	FString EnumToString(TEnumType Value)
	{
		return StaticEnum<TEnumType>()->GetNameStringByValue(static_cast<int64>(Value));
	}

	class FDevConsoleVariableImplBase : public IConsoleVariable
	{
	public:
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
		virtual void Release() override
		{
			delete this;
		}

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
		TIsSame<TVariableType, float>::Value || TIsSame<TVariableType, FString>::Value>::Type>
	struct TDevConsoleDynamicConsoleVariableImpl final : FDevConsoleVariableImplBase
	{
		using FGetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FGetter;
		using FSetter = typename TDevConsoleDynamicConsoleVariable<TVariableType>::FSetter;

		TDevConsoleDynamicConsoleVariableImpl(const TCHAR* InHelp, const uint32 InFlags, const FGetter& InGetter, const FSetter& InSetter)
			: FDevConsoleVariableImplBase(InHelp, InFlags), Getter(InGetter), Setter(InSetter) {}

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

			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute() ? 1 : 0; }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return Getter.Execute(); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return static_cast<int32>(Getter.Execute()); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atoi(*Getter.Execute()); }

			return 0;
		}

		virtual float GetFloat() const override
		{
			if (!Getter.IsBound()) { return 0.0f; }

			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute() ? 1.0f : 0.0f; }
			if constexpr (TIsSame<TVariableType, int32>::Value) { return static_cast<float>(Getter.Execute()); }
			if constexpr (TIsSame<TVariableType, float>::Value) { return Getter.Execute(); }
			if constexpr (TIsSame<TVariableType, FString>::Value) { return FCString::Atof(*Getter.Execute()); }

			return 0.0f;
		}

		virtual FString GetString() const override
		{
			if (!Getter.IsBound()) { return FString(); }

			if constexpr (TIsSame<TVariableType, bool>::Value) { return Getter.Execute() ? TEXT("1") : TEXT("0"); }
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

			NotifyChanged(SetBy);
		}

	private:
		FGetter Getter;
		FSetter Setter;
	};

	template<typename TVariableType, typename = typename TEnableIf<
		TIsSame<TVariableType, bool>::Value || TIsSame<TVariableType, int32>::Value ||
		TIsSame<TVariableType, float>::Value || TIsSame<TVariableType, FString>::Value>::Type>
	struct TDevConsoleDynamicConsoleVariableWithWorldImpl final : FDevConsoleVariableImplBase
	{
		using FGetter = typename TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::FGetter;
		using FSetter = typename TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::FSetter;

		TDevConsoleDynamicConsoleVariableWithWorldImpl(const TCHAR* InHelp, const uint32 InFlags, FGetter&& InGetter, FSetter&& InSetter)
			: FDevConsoleVariableImplBase(InHelp, InFlags), Getter(Forward<FGetter>(InGetter)), Setter(Forward<FSetter>(InSetter)) {}

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

			NotifyChanged(SetBy);
		}

	private:
		FGetter Getter;
		FSetter Setter;
	};
}

using namespace DevConsole::Implementation;

FDevConsoleVariableBase::FDevConsoleVariableBase(IConsoleObject* InRegisteredObject)
	: RegisteredObject(InRegisteredObject)
{
}

FDevConsoleVariableBase::~FDevConsoleVariableBase()
{
	if (RegisteredObject)
	{
		IConsoleManager::Get().UnregisterConsoleObject(RegisteredObject);
	}
}

FDevConsoleVariableBase::FDevConsoleVariableBase(FDevConsoleVariableBase&& Other)
	: RegisteredObject(Other.RegisteredObject)
{
	Other.RegisteredObject = nullptr;
}

FDevConsoleVariableBase& FDevConsoleVariableBase::operator=(FDevConsoleVariableBase&& Other)
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

template<typename TEnumType>
TDevConsoleEnumConsoleVariable<TEnumType>::TDevConsoleEnumConsoleVariable(const TCHAR* Name, const TCHAR* Help, const TEnumType& DefaultValue, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new TDevConsoleEnumConsoleVariableImpl<TEnumType>(Help, Flags, DefaultValue))) {}

template<typename TEnumType>
TDevConsoleEnumConsoleVariableRef<TEnumType>::TDevConsoleEnumConsoleVariableRef(const TCHAR* Name, const TCHAR* Help, TEnumType& RefValue, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new TDevConsoleEnumConsoleVariableRefImpl<TEnumType>(Help, Flags, RefValue))) {}

template<typename TVariableType>
TDevConsoleDynamicConsoleVariable<TVariableType>::TDevConsoleDynamicConsoleVariable(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new TDevConsoleDynamicConsoleVariableImpl<TVariableType>(Help, Flags, Forward<FGetter>(Getter), Forward<FSetter>(Setter)))) {}

template<typename TVariableType>
TDevConsoleDynamicConsoleVariableWithWorld<TVariableType>::TDevConsoleDynamicConsoleVariableWithWorld(const TCHAR* Name, const TCHAR* Help, FGetter&& Getter, FSetter&& Setter, const uint32 Flags)
	: FDevConsoleVariableBase(AddConsoleObject(Name, new TDevConsoleDynamicConsoleVariableWithWorldImpl<TVariableType>(Help, Flags, Forward<FGetter>(Getter), Forward<FSetter>(Setter)))) {}

#undef DEFINE_PRIVATE_ACCESS_FUNCTION
