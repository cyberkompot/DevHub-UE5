// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"
#include UE_COMPATIBILITY_INCLUDE_PROPERTY_BAG_PATH

struct FInstancedPropertyBag;

struct FDevCoreOverrideablePropertiesIterator final
{
	static constexpr int32 ExpectedPropertiesNum = 32;
	using FPropertiesInlineAllocator = TInlineAllocator<ExpectedPropertiesNum>;

	explicit FDevCoreOverrideablePropertiesIterator(const UClass* InClass);

	void operator ++() { ++It; IterateToNext(); }
	operator bool() const { return !!It; }
	FProperty* operator *() { return *It; }
	const FProperty* operator *() const { return *It; }

private:
	TFieldIterator<FProperty> It;
	TSet<FName, DefaultKeyFuncs<FName>, TInlineSparseSetAllocator<ExpectedPropertiesNum>> VisitedProperties; // Avoid duplicates by name (can happen via interface + class, or shadowed members).

	void IterateToNext();
};

struct FDevCorePropertyBagUtils final
{
	static DEVCORE_API void AddPropertyBagToReferenceCollector(FReferenceCollector& InCollector, FInstancedPropertyBag& InBag);

	static DEVCORE_API void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag);
	static DEVCORE_API void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag, FInstancedPropertyBag& OutOriginalValues);

	static DEVCORE_API void ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject);
	static DEVCORE_API void ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject, FInstancedPropertyBag& OutOriginalValues);

	static DEVCORE_API FInstancedPropertyBag MakePropertyBagByClass(const UClass* InClass, const bool InSetValues = true);
	static DEVCORE_API FInstancedPropertyBag MakePropertyBagByObject(const UObject* InObject, const bool InSetValues = true);

	static DEVCORE_API FInstancedPropertyBag MakePropertyBagWithClassOverrides(const FInstancedPropertyBag& InBag, const UClass* InClass);
	static DEVCORE_API FInstancedPropertyBag MakePropertyBagWithObjectOverrides(const FInstancedPropertyBag& InBag, const UObject* InObject);

	static DEVCORE_API bool IsPropertyOverrideable(const FProperty* InProperty);

	static DEVCORE_API FGuid GetStablePropertyId(const UClass* InClass, FName InPropertyName);
	static DEVCORE_API FPropertyBagPropertyDesc MakeStablePropertyDesc(const UClass* InClass, const FProperty* InProperty);
};
