// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleTypes.h"

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

namespace DevConsole::Implementation
{
	DEFINE_PRIVATE_ACCESS_FUNCTION(FConsoleManager, AddConsoleObject)
}

IDevConsoleVariableExt* FDevConsoleVariableBase::AddConsoleObject(const TCHAR* Name, IDevConsoleVariableExt* Obj)
{
	FConsoleManager& ConsoleManager = static_cast<FConsoleManager&>(IConsoleManager::Get());
	return static_cast<IDevConsoleVariableExt*>(DevConsole::Implementation::PrivateAccess::AddConsoleObject(ConsoleManager)(Name, Obj));
}

FDevConsoleVariableBase::FDevConsoleVariableBase(IDevConsoleVariableExt* InRegisteredObject)
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

#undef DEFINE_PRIVATE_ACCESS_FUNCTION
