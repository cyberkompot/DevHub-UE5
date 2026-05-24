// Copyright (c) Alexandr Pereverzev.

#include "ClassBag.h"

namespace ClassBag::Private
{
	void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag, FInstancedPropertyBag* OutOriginalValues)
	{
		if (OutOriginalValues)
		{
			OutOriginalValues->Reset();
		}

		if (!InBag.IsValid()) { return; }
		if (!InObject) { return; }

		if (InBag.GetNumPropertiesInBag() == 0) { return; }

		if (OutOriginalValues)
		{
			OutOriginalValues->InitializeFromBagStruct(InBag.GetPropertyBagStruct());
		}

		const UClass* Class = InObject->GetClass();
		uint8* BagMemory = InBag.GetMutableValue().GetMemory();
		for (FClassOverrideablePropertiesIterator It(Class); It; ++It)
		{
			const FProperty* ObjectProperty = *It;
			const FGuid ClassPropertyGuid = FClassBagUtils::GetStablePropertyId(Class, ObjectProperty->GetFName());
			if (const FPropertyBagPropertyDesc* BagPropertyDesc = InBag.FindPropertyDescByID(ClassPropertyGuid))
			{
				const FProperty* BagProperty = BagPropertyDesc->CachedProperty;
				const void* ObjectValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(InObject);

				if (OutOriginalValues)
				{
					OutOriginalValues->SetValue(BagPropertyDesc->Name, ObjectProperty, InObject);
				}

				BagProperty->SetValue_InContainer(BagMemory, ObjectValuePtr);
			}
		}
	}

	void ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject, FInstancedPropertyBag* OutOriginalValues)
	{
		if (OutOriginalValues)
		{
			OutOriginalValues->Reset();
		}

		if (!InBag.IsValid()) { return; }
		if (!InObject) { return; }

		if (InBag.GetNumPropertiesInBag() == 0) { return; }

		if (OutOriginalValues)
		{
			OutOriginalValues->InitializeFromBagStruct(InBag.GetPropertyBagStruct());
		}

		const UClass* Class = InObject->GetClass();
		const uint8* BagMemory = InBag.GetValue().GetMemory();
		for (FClassOverrideablePropertiesIterator It(Class); It; ++It)
		{
			const FProperty* ObjectProperty = *It;
			const FGuid ClassPropertyGuid = FClassBagUtils::GetStablePropertyId(Class, ObjectProperty->GetFName());
			if (const FPropertyBagPropertyDesc* BagPropertyDesc = InBag.FindPropertyDescByID(ClassPropertyGuid))
			{
				const FProperty* BagProperty = BagPropertyDesc->CachedProperty;
				const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);

				if (OutOriginalValues)
				{
					OutOriginalValues->SetValue(BagPropertyDesc->Name, ObjectProperty, InObject);
				}

				ObjectProperty->SetValue_InContainer(InObject, BagValuePtr);
			}
		}
	}

	FGuid FindBlueprintPropertyGuid(const UClass* InClass, const FName InPropertyName)
	{
		// Try to get the Blueprint variable GUID for a property name up the class chain. Works across renames in the same Blueprint.
		for (const UClass* CurrentClass = InClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
		{
			// Look at the Blueprint’s variable table.
			if (const UBlueprint* Blueprint = UBlueprint::GetBlueprintFromClass(CurrentClass))
			{
				for (const FBPVariableDescription& Desc : Blueprint->NewVariables)
				{
					if (Desc.VarName == InPropertyName && Desc.VarGuid.IsValid())
					{
						return Desc.VarGuid;
					}
				}
			}
			else
			{
				break;
			}
		}
		return FGuid();
	}
}

using namespace ClassBag::Private;

void FClassBagUtils::AddPropertyBagToReferenceCollector(FReferenceCollector& InCollector, FInstancedPropertyBag& InBag)
{
	const UPropertyBag* BagStruct = InBag.GetPropertyBagStruct();
	if (!BagStruct) { return; }

	TObjectPtr<const UPropertyBag> BagStructPtr(BagStruct);
	InCollector.AddReferencedObject(BagStructPtr);

	if (uint8* BagMemory = InBag.GetMutableValue().GetMemory())
	{
		InCollector.AddPropertyReferencesWithStructARO(BagStruct, BagMemory);
	}
}

void FClassBagUtils::ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag)
{
	ClassBag::Private::ApplyObjectToPropertyBag(InObject, InBag, nullptr);
}

void FClassBagUtils::ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag, FInstancedPropertyBag& OutOriginalValues)
{
	ClassBag::Private::ApplyObjectToPropertyBag(InObject, InBag, &OutOriginalValues);
}

void FClassBagUtils::ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject)
{
	ClassBag::Private::ApplyPropertyBagToObject(InBag, InObject, nullptr);
}

void FClassBagUtils::ApplyPropertyBagToObject(const FInstancedPropertyBag& InBag, UObject* InObject, FInstancedPropertyBag& OutOriginalValues)
{
	ClassBag::Private::ApplyPropertyBagToObject(InBag, InObject, &OutOriginalValues);
}

FInstancedPropertyBag FClassBagUtils::MakePropertyBagByClass(const UClass* InClass, const bool InSetValues)
{
	if (!InClass) { return FInstancedPropertyBag(); }

	return MakePropertyBagByObject(InClass->GetDefaultObject(), InSetValues);
}

FInstancedPropertyBag FClassBagUtils::MakePropertyBagByObject(const UObject* InObject, const bool InSetValues)
{
	if (!InObject) { return FInstancedPropertyBag(); }

	const UClass* Class = InObject->GetClass();

	constexpr int32 ExpectedPropertiesNum = 32;
	TArray<FPropertyBagPropertyDesc, TInlineAllocator<ExpectedPropertiesNum>> NewBagDescs;
	for (FClassOverrideablePropertiesIterator It(Class); It; ++It)
	{
		const FProperty* ClassProperty = *It;
		NewBagDescs.Emplace(MakeStablePropertyDesc(Class, ClassProperty));
	}

	FInstancedPropertyBag NewBag;
	NewBag.AddProperties(NewBagDescs);

	if (InSetValues)
	{
		ApplyObjectToPropertyBag(InObject, NewBag);
	}

	return NewBag;
}

FInstancedPropertyBag FClassBagUtils::MakePropertyBagWithClassOverrides(const FInstancedPropertyBag& InBag, const UClass* InClass)
{
	if (!InClass) { return FInstancedPropertyBag(); }

	return MakePropertyBagWithObjectOverrides(InBag, InClass->GetDefaultObject());
}

FInstancedPropertyBag FClassBagUtils::MakePropertyBagWithObjectOverrides(const FInstancedPropertyBag& InBag, const UObject* InObject)
{
	if (!InObject) { return FInstancedPropertyBag(); }
	if (!InBag.IsValid()) { return FInstancedPropertyBag(); }

	const UClass* Class = InObject->GetClass();
	const uint8* BagMemory = InBag.GetValue().GetMemory();
	TArray<FPropertyBagPropertyDesc, FClassOverrideablePropertiesIterator::FPropertiesInlineAllocator> NewBagDescs;
	for (FClassOverrideablePropertiesIterator It(Class); It; ++It)
	{
		const FProperty* ObjectProperty = *It;
		const FGuid ObjectPropertyId = GetStablePropertyId(Class, ObjectProperty->GetFName());
		if (const FPropertyBagPropertyDesc* BagPropertyDesc = InBag.FindPropertyDescByID(ObjectPropertyId))
		{
			const FProperty* BagProperty = BagPropertyDesc->CachedProperty;

			const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);
			const void* ObjectValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(InObject);

			if (!ObjectProperty->Identical(BagValuePtr, ObjectValuePtr))
			{
				NewBagDescs.Emplace(*BagPropertyDesc);
			}
		}
	}

	FInstancedPropertyBag NewBag;
	if (!NewBagDescs.IsEmpty())
	{
		NewBag.AddProperties(NewBagDescs);
		NewBag.CopyMatchingValuesByID(InBag);
	}

	return NewBag;
}

bool FClassBagUtils::IsPropertyOverrideable(const FProperty* InProperty)
{
	if (!InProperty) { return false; }

	const uint64 PropertyFlags = InProperty->GetPropertyFlags();

	// Must be editable on an *instance* (not defaults-only), and not read-only in Details.
	const bool bInstanceEditable =
		(PropertyFlags & CPF_Edit) // Designer-editable.
		&& !(PropertyFlags & CPF_DisableEditOnInstance) // Excludes EditDefaultsOnly.
		&& !(PropertyFlags & CPF_EditConst); // Not read-only in Editor.

	// Must be "public" (both native and script visibility).
	const bool bPublic = !(PropertyFlags & (CPF_NativeAccessSpecifierPrivate | CPF_NativeAccessSpecifierProtected | CPF_Protected));

	// Runtime-relevant (don’t pipe editor-only/deprecated stuff).
	const bool bRuntimeRelevant = !(PropertyFlags & (CPF_EditorOnly | CPF_Deprecated));

	// Supported by PropertyBag (no Map and Delegates).
	const bool bSupportedType =
		!InProperty->IsA<FMapProperty>()
		&& !InProperty->IsA<FDelegateProperty>()
		&& !InProperty->IsA<FMulticastDelegateProperty>();

	return (bSupportedType && bInstanceEditable && bPublic && bRuntimeRelevant);
}

FGuid FClassBagUtils::GetStablePropertyId(const UClass* InClass, const FName InPropertyName)
{
	if (!InClass) { return FGuid(); }

	if (const FGuid Guid = FindBlueprintPropertyGuid(InClass, InPropertyName); Guid.IsValid())
	{
		// Prefer Blueprint var GUID (stable across renames inside the Blueprint).
		return Guid;
	}
	else
	{
		// Deterministic GUID from class path + property name (fallback when no Blueprint GUID).
		const uint32 NameHash = GetTypeHash(InPropertyName.ToString());
		const uint32 ClassHash = GetTypeHash(InClass->GetPathName());
		return FGuid(NameHash, ClassHash, 0xF3A9C7D2, 0xE8B4FF91);
	}
}

FPropertyBagPropertyDesc FClassBagUtils::MakeStablePropertyDesc(const UClass* InClass, const FProperty* InProperty)
{
	if (InClass && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();
		FPropertyBagPropertyDesc Desc(PropertyName, InProperty);
		Desc.ID = GetStablePropertyId(InClass, PropertyName);
		return Desc;
	}
	else
	{
		return FPropertyBagPropertyDesc();
	}
}

FClassOverrideablePropertiesIterator::FClassOverrideablePropertiesIterator(const UClass* InClass)
	: It(TFieldIterator<FProperty>(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated, EFieldIteratorFlags::IncludeInterfaces))
{
	IterateToNext();
}

void FClassOverrideablePropertiesIterator::IterateToNext()
{
	while (It)
	{
		if (const FProperty* Property = *It; FClassBagUtils::IsPropertyOverrideable(Property))
		{
			bool bIsAlreadyInSetPtr = false;
			if (VisitedProperties.Emplace(Property->GetFName(), &bIsAlreadyInSetPtr); !bIsAlreadyInSetPtr)
			{
				break;
			}
		}

		++It;
	}
}
