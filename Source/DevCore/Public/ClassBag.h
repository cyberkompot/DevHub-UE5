// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Misc/EngineVersionComparison.h"

#ifndef UE_VERSION_NEWER_THAN_OR_EQUAL
#define UE_VERSION_NEWER_THAN_OR_EQUAL(MajorVersion, MinorVersion, PatchVersion) UE_GREATER_SORT(ENGINE_MAJOR_VERSION, MajorVersion, UE_GREATER_SORT(ENGINE_MINOR_VERSION, MinorVersion, UE_GREATER_SORT(ENGINE_PATCH_VERSION, PatchVersion, true)))
#endif

#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 5, 0) && !UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
#include "StructUtils/PropertyBag.h"
#else
#include "PropertyBag.h"
#endif

#define UE_API DEVCORE_API

/**
 * Utilities for transferring overridable property values between an object (or CDO) and a property bag,
 * and for creating property bags whose structure mirrors the layout of a class's overridable properties.
 */
struct FClassBagUtils final
{
	/**
	 * Captures object's current values into the bag, keeping the bag's struct unchanged.
	 * Entries are skipped if the corresponding property does not exist, is not overridable,
	 * or has an incompatible type. No-op if the bag is invalid or the object is null.
	 *
	 * @param InObject Source object.
	 * @param InBag Target bag.
	 */
	static UE_API void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag);

	/**
	 * As the two-argument FClassBagUtils::ApplyObjectToPropertyBag overload, but first captures
	 * a copy of the bag's values into output bag, enabling restore-after-apply patterns.
	 *
	 * @param InObject Source object.
	 * @param InBag Target bag.
	 * @param OutOriginalValues Receives a copy of the bag's values prior to applying values from the object.
	 */
	static UE_API void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag, FInstancedPropertyBag& OutOriginalValues);

	/**
	 * Applies values from the bag to the object. Entries are skipped if the corresponding property does not exist,
	 * is not overridable, or has an incompatible type. No-op if the bag is invalid or the object is null.
	 *
	 * @param InBag Source bag.
	 * @param InObject Target object.
	 * @warning When the object is a CDO, the override becomes visible to every spawn of that class until it is restored.
	 */
	static UE_API void ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject);

	/**
	 * As the two-argument FClassBagUtils::ApplyPropertyBagToObject overload, but first captures
	 * a copy of the object's values into output bag, enabling restore-after-apply patterns.
	 *
	 * @param InBag Source bag.
	 * @param InObject Target object.
	 * @param OutOriginalValues Receives a copy of the object's values prior to applying values from the bag.
	 * @warning When the object is a CDO, the override becomes visible to every spawn of that class until it is restored.
	 *          Apply and restore back-to-back, ideally behind an RAII guard.
	 */
	static UE_API void ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject, FInstancedPropertyBag& OutOriginalValues);

	/**
	 * Makes a bag whose struct mirrors the class's overridable properties layout.
	 *
	 * @param InClass Class to describe. Returns an empty bag when null.
	 * @param InSetValues When true (default), sets each entry with the CDO value. When false, struct only.
	 */
	static UE_API FInstancedPropertyBag MakePropertyBagByClass(const UClass* InClass, const bool InSetValues = true);

	/**
	 * Makes a bag whose struct mirrors the object's overridable properties layout.
	 *
	 * @param InObject Object whose class is described. Returns an empty bag when null.
	 * @param InSetValues When true (default), sets each entry with object's current value. When false, struct only.
	 */
	static UE_API FInstancedPropertyBag MakePropertyBagByObject(const UObject* InObject, const bool InSetValues = true);

	/**
	 * Returns a copy of the bag reduced to only the entries whose value differs from class's CDO value.
	 *
	 * @param InBag Bag of candidate values.
	 * @param InClass Class whose CDO supplies the baseline. Returns an empty bag when null.
	 */
	static UE_API FInstancedPropertyBag MakePropertyBagWithClassOverrides(const FInstancedPropertyBag& InBag, const UClass* InClass);

	/**
	 * Returns a copy of the bag reduced to only the entries whose value differs from object's value.
	 *
	 * @param InBag Bag of candidate values.
	 * @param InObject Object supplying the baseline. Returns an empty bag when null.
	 */
	static UE_API FInstancedPropertyBag MakePropertyBagWithObjectOverrides(const FInstancedPropertyBag& InBag, const UObject* InObject);

	/**
	 * Returns a rename-stable GUID for Blueprint property in editor builds, and a deterministic hash of the property name in non-editor builds.
	 *
	 * @param InClass Class used to look up the Blueprint GUID.
	 * @param InPropertyName Property name to identify.
	 * @see FClassBagUtils::MakeStablePropertyDesc.
	 */
	static UE_API FGuid GetStablePropertyId(const UClass* InClass, FName InPropertyName);

	/**
	 * Makes a description for a property in the property bag using a stable ID that persists across Blueprint property renames.
	 *
	 * @param InClass Class used to look up the Blueprint properties GUID.
	 * @param InProperty Property to describe.
	 * @see FClassBagUtils::GetStablePropertyId.
	 */
	static UE_API FPropertyBagPropertyDesc MakeStablePropertyDesc(const UClass* InClass, const FProperty* InProperty);

#if WITH_EDITOR
	/**
	 * Migrates the bag in place so its structure matches the current layout of the class, re-keying surviving
	 * values and dropping entries whose property no longer exists.
	 *
	 * Typically called from the owner's PreSave so the cooked bag resolves by direct name match.
	 *
	 * @param InClass Class whose layout the bag should match.
	 * @param InOutBag Bag migrated in place.
	 */
	static UE_API void ActualizePropertyBag(const UClass* InClass, FInstancedPropertyBag& InOutBag);
#endif // WITH_EDITOR
};

/** Forward iterator over the overridable properties of a class. */
struct UE_API FClassBagCompatiblePropertiesIterator final
{
	/** Empirical estimate of overridable properties count for a class, including extra reserved capacity. */
	static constexpr int32 ExpectedPropertiesNum = 32;

	/** Inline allocator for callers gathering descriptors/properties. */
	using FPropertiesInlineAllocator = TInlineAllocator<ExpectedPropertiesNum>;

	/**
	 * Begins iteration at the first overridable property of the class (or at the end, if it has none).
	 * @param InClass Class to iterate. Supers and implemented interfaces are included.
	 */
	explicit FClassBagCompatiblePropertiesIterator(const UClass* InClass);

	/** Advances to the next overridable, not-yet-visited property. */
	void operator ++() { ++It; IterateToNext(); }

	/** @return True while the iterator points at a valid property. */
	operator bool() const { return !!It; }

	/** @return The property currently pointed at. */
	FProperty* operator *() { return *It; }

	/** @return The property currently pointed at. */
	const FProperty* operator *() const { return *It; }

private:
	TFieldIterator<FProperty> It;
	TSet<FName, DefaultKeyFuncs<FName>, TInlineSetAllocator<ExpectedPropertiesNum>> VisitedProperties; // Avoid duplicates by name (can happen via interface + class, or shadowed members).

	/** Advances iterator until it lands on an overridable, not-yet-visited property, or reaches the end. */
	void IterateToNext();
};

#undef UE_API