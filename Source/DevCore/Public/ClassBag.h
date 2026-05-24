// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Misc/EngineVersionComparison.h"

#ifndef UE_VERSION_AT_LEAST
	#define UE_VERSION_AT_LEAST(MajorVersion, MinorVersion, PatchVersion) UE_VERSION_NEWER_THAN(MajorVersion, MinorVersion, PatchVersion - 1)
#endif

#if UE_VERSION_AT_LEAST(5, 5, 0) && !(defined(UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5) && UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5)
	#include "StructUtils/PropertyBag.h"
#else
	#include "PropertyBag.h"
#endif

struct FClassBagUtils final
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

struct DEVCORE_API FClassOverrideablePropertiesIterator final
{
	static constexpr int32 ExpectedPropertiesNum = 32;
	using FPropertiesInlineAllocator = TInlineAllocator<ExpectedPropertiesNum>;

	explicit FClassOverrideablePropertiesIterator(const UClass* InClass);

	void operator ++() { ++It; IterateToNext(); }
	operator bool() const { return !!It; }
	FProperty* operator *() { return *It; }
	const FProperty* operator *() const { return *It; }

private:
	TFieldIterator<FProperty> It;
	TSet<FName, DefaultKeyFuncs<FName>, TInlineSparseSetAllocator<ExpectedPropertiesNum>> VisitedProperties; // Avoid duplicates by name (can happen via interface + class, or shadowed members).

	void IterateToNext();
};
