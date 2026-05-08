// Copyright (c) Alexandr Pereverzev.

#pragma once
#include "StructUtils/PropertyBag.h"

struct FInstancedPropertyBag;

struct FDevCoreOverrideablePropertiesIterator final
{
	explicit FDevCoreOverrideablePropertiesIterator(const UClass* InClass);

	void operator ++() { ++It; IterateToNext(); }
	operator bool() const { return !!It; }
	FProperty* operator *() { return *It; }
	const FProperty* operator *() const { return *It; }

private:
	TFieldIterator<FProperty> It;
	void IterateToNext();
};

struct FDevCorePropertyBagUtils final
{
	static DEVCORE_API void ApplyPropertyBagToObject(UObject* InObject, const FInstancedPropertyBag& InBag);
	static DEVCORE_API void ApplyPropertyBagToObject(UObject* InObject, const FInstancedPropertyBag& InBag, FInstancedPropertyBag& OutOriginalValues);

	static DEVCORE_API bool IsPropertyOverrideable(const FProperty* InProperty);

	static DEVCORE_API FGuid GetStablePropertyId(const UClass* InClass, FName InPropertyName);
	static DEVCORE_API FPropertyBagPropertyDesc MakePropertyDesc(const UClass* Class, const FProperty* Property);
};
