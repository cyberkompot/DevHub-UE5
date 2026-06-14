// Copyright (c) Alexandr Pereverzev.

#include "ClassBag.h"

#include "Hash/xxhash.h"

namespace ClassBag::Private
{
	bool IsPropertyOverridable(const FProperty* InProperty)
	{
		if (!InProperty) { return false; }

		const uint64 PropertyFlags = InProperty->GetPropertyFlags();

		// Must be editable on an instance (not defaults-only), and not read-only in Details.
		const bool bInstanceEditable =
			(PropertyFlags & CPF_Edit) // Designer-editable.
			&& !(PropertyFlags & CPF_DisableEditOnInstance) // Excludes EditDefaultsOnly.
			&& !(PropertyFlags & CPF_EditConst); // Not read-only in Editor.

		// Must be "public" (both native and script visibility).
		const bool bPublic = !(PropertyFlags & (CPF_NativeAccessSpecifierPrivate | CPF_NativeAccessSpecifierProtected | CPF_Protected));

		// Runtime-relevant (don’t pipe editor-only/deprecated stuff).
		const bool bRuntimeRelevant = !(PropertyFlags & (CPF_EditorOnly | CPF_Deprecated));

		// Supportable by FInstancedPropertyBag: scalar/enum/struct/object values, Array/Set, but not TMap, delegates, or nested containers.
		const bool bSupportedType =
			!InProperty->IsA<FMapProperty>()
			&& !InProperty->IsA<FDelegateProperty>()
			&& !InProperty->IsA<FMulticastDelegateProperty>();

		return (bInstanceEditable && bPublic && bRuntimeRelevant && bSupportedType);
	}

	bool ArePropertyTypesCompatible(const FProperty* InA, const FProperty* InB)
	{
		return (InA && InA->SameType(InB));
	}

	FProperty* FindObjectProperty(const UClass* InClass, const FPropertyBagPropertyDesc& InDesc)
	{
		if (!InClass) { return nullptr; }

		FProperty* ObjectProperty = InClass->FindPropertyByName(InDesc.Name);

		// Fallback: match by redirected name, covering CoreRedirects.
		if (!ObjectProperty)
		{
			if (const FName RedirectedName = FProperty::FindRedirectedPropertyName((UClass*)InClass, InDesc.Name); !RedirectedName.IsNone())
			{
				ObjectProperty = InClass->FindPropertyByName(RedirectedName);
			}
		}

		// Fallback: match by stable ID, covering an in-editor rename.
#if WITH_EDITORONLY_DATA
		if (!ObjectProperty)
		{
			for (FClassBagCompatiblePropertiesIterator It(InClass); It; ++It)
			{
				if (FClassBagUtils::GetStablePropertyId(InClass, (*It)->GetFName()) == InDesc.ID)
				{
					ObjectProperty = *It;
					break;
				}
			}
		}
#endif

		return ObjectProperty;
	}

	FProperty* FindCompatibleObjectProperty(const UClass* InClass, const FPropertyBagPropertyDesc& InDesc)
	{
		if (FProperty* ObjectProperty = FindObjectProperty(InClass, InDesc);
			ObjectProperty
			&& IsPropertyOverridable(ObjectProperty)
			&& ArePropertyTypesCompatible(InDesc.CachedProperty, ObjectProperty))
		{
			return ObjectProperty;
		}
		return nullptr;
	}

	void ApplyObjectToPropertyBag(const UObject* InObject, FInstancedPropertyBag& InBag, FInstancedPropertyBag* OutOriginalValues)
	{
		if (OutOriginalValues)
		{
			OutOriginalValues->Reset();
		}

		if (!InBag.IsValid()) { return; }
		if (!InObject) { return; }

		const UPropertyBag* BagStruct = InBag.GetPropertyBagStruct();
		if (!BagStruct) { return; }

		const TConstArrayView<FPropertyBagPropertyDesc> BagDescs = BagStruct->GetPropertyDescs();
		if (BagDescs.IsEmpty()) { return; }

		if (OutOriginalValues)
		{
			OutOriginalValues->InitializeFromBagStruct(BagStruct);
		}

		const UClass* Class = InObject->GetClass();
		uint8* BagMemory = InBag.GetMutableValue().GetMemory();
		for (const FPropertyBagPropertyDesc& BagPropertyDesc : BagDescs)
		{
			const FProperty* ObjectProperty = FindCompatibleObjectProperty(Class, BagPropertyDesc);
			if (!ObjectProperty) { continue; }

			const FProperty* BagProperty = BagPropertyDesc.CachedProperty;
			const void* ObjectValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(InObject);

			if (OutOriginalValues)
			{
				OutOriginalValues->SetValue(BagPropertyDesc.Name, ObjectProperty, InObject);
			}

			BagProperty->SetValue_InContainer(BagMemory, ObjectValuePtr);
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

		const UPropertyBag* BagStruct = InBag.GetPropertyBagStruct();
		if (!BagStruct) { return; }

		const TConstArrayView<FPropertyBagPropertyDesc> BagDescs = BagStruct->GetPropertyDescs();
		if (BagDescs.IsEmpty()) { return; }

		if (OutOriginalValues)
		{
			OutOriginalValues->InitializeFromBagStruct(BagStruct);
		}

		const UClass* Class = InObject->GetClass();
		const uint8* BagMemory = InBag.GetValue().GetMemory();
		for (const FPropertyBagPropertyDesc& BagPropertyDesc : BagDescs)
		{
			const FProperty* ObjectProperty = FindCompatibleObjectProperty(Class, BagPropertyDesc);
			if (!ObjectProperty) { continue; }

			const FProperty* BagProperty = BagPropertyDesc.CachedProperty;
			const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);

			if (OutOriginalValues)
			{
				OutOriginalValues->SetValue(BagPropertyDesc.Name, ObjectProperty, InObject);
			}

			ObjectProperty->SetValue_InContainer(InObject, BagValuePtr);
		}
	}

#if WITH_EDITORONLY_DATA
	FGuid FindBlueprintPropertyGuid(const UClass* InClass, const FName InPropertyName)
	{
		// Walk the whole class chain looking for the Blueprint variable GUID, which is stable across renames.
		// Continue past non-Blueprint (native) ancestors so a Blueprint variable declared above a native base class still resolves to its GUID.
		for (const UClass* CurrentClass = InClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
		{
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
		}
		return FGuid();
	}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
	FInstancedPropertyBag MakeActualizedPropertyBag(const UClass* InClass, const FInstancedPropertyBag& InBag)
	{
		if (!InClass) { return FInstancedPropertyBag(); }
		if (!InBag.IsValid()) { return InBag; }

		const UPropertyBag* SourceStruct = InBag.GetPropertyBagStruct();
		if (!SourceStruct) { return InBag; }

		// Resolve stored properties onto the current class layout (honoring Blueprint renames, core redirects).
		const TConstArrayView<FPropertyBagPropertyDesc> SourceDescs = SourceStruct->GetPropertyDescs();
		TArray<const FProperty*, FClassBagCompatiblePropertiesIterator::FPropertiesInlineAllocator> SourceProperties;
		TArray<FPropertyBagPropertyDesc, FClassBagCompatiblePropertiesIterator::FPropertiesInlineAllocator> NewDescs;
		SourceProperties.Reserve(SourceDescs.Num());
		NewDescs.Reserve(SourceDescs.Num());
		for (const FPropertyBagPropertyDesc& OldDesc : SourceDescs)
		{
			if (OldDesc.CachedProperty)
			{
				if (const FProperty* LiveProperty = FindCompatibleObjectProperty(InClass, OldDesc))
				{
					SourceProperties.Emplace(OldDesc.CachedProperty);
					NewDescs.Emplace(FClassBagUtils::MakeStablePropertyDesc(InClass, LiveProperty));
				}
			}
		}

		// Creating new bag with actualized layout.
		FInstancedPropertyBag NewBag;
		if (NewDescs.IsEmpty()) { return NewBag; }

		NewBag.AddProperties(NewDescs);

		// Copy values across using the stable ID to resolved property mapping.
		const uint8* SourceMemory = InBag.GetValue().GetMemory();
		uint8* NewMemory = NewBag.GetMutableValue().GetMemory();
		for (int32 Index = 0; Index < NewDescs.Num(); ++Index)
		{
			const FProperty* SourceProperty = SourceProperties[Index];
			const FPropertyBagPropertyDesc* NewDesc = NewBag.FindPropertyDescByID(NewDescs[Index].ID);
			if (NewDesc && ArePropertyTypesCompatible(SourceProperty, NewDesc->CachedProperty))
			{
				const void* SourceValuePtr = SourceProperty->ContainerPtrToValuePtr<void>(SourceMemory);
				NewDesc->CachedProperty->SetValue_InContainer(NewMemory, SourceValuePtr);
			}
		}

		return NewBag;
	}
#endif // WITH_EDITOR
}

using namespace ClassBag::Private;


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

	TArray<FPropertyBagPropertyDesc, FClassBagCompatiblePropertiesIterator::FPropertiesInlineAllocator> NewBagDescs;
	for (FClassBagCompatiblePropertiesIterator It(Class); It; ++It)
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

	const UPropertyBag* BagStruct = InBag.GetPropertyBagStruct();
	if (!BagStruct) { return FInstancedPropertyBag(); }

	const UClass* Class = InObject->GetClass();
	const uint8* BagMemory = InBag.GetValue().GetMemory();
	TArray<FPropertyBagPropertyDesc, FClassBagCompatiblePropertiesIterator::FPropertiesInlineAllocator> NewBagDescs;
	for (const FPropertyBagPropertyDesc& BagPropertyDesc : BagStruct->GetPropertyDescs())
	{
		const FProperty* ObjectProperty = FindCompatibleObjectProperty(Class, BagPropertyDesc);
		if (!ObjectProperty) { continue; }

		const FProperty* BagProperty = BagPropertyDesc.CachedProperty;
		const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);
		const void* ObjectValuePtr = ObjectProperty->ContainerPtrToValuePtr<void>(InObject);

		// Keep only values that differ from the object/class default. Compare through the bag
		// property (its layout matches the stored value); the types are guaranteed compatible.
		if (!BagProperty->Identical(BagValuePtr, ObjectValuePtr))
		{
			NewBagDescs.Emplace(BagPropertyDesc);
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

FGuid FClassBagUtils::GetStablePropertyId(const UClass* InClass, const FName InPropertyName)
{
	if (!InClass) { return FGuid(); }

#if WITH_EDITORONLY_DATA
	// Prefer the Blueprint variable GUID (stable across renames inside the Blueprint).
	if (const FGuid Guid = FindBlueprintPropertyGuid(InClass, InPropertyName); Guid.IsValid())
	{
		return Guid;
	}
#endif // WITH_EDITORONLY_DATA

	alignas(uint32) uint8 HashBytes[16];
	FNameBuilder PropertyNameBuilder(InPropertyName);
	FXxHash128::HashBuffer(PropertyNameBuilder.GetData(), PropertyNameBuilder.Len()).ToByteArray(HashBytes);
	return FGuid::NewGuidFromHashBytes(HashBytes, UE_ARRAY_COUNT(HashBytes));
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

#if WITH_EDITOR
void FClassBagUtils::ActualizePropertyBag(const UClass* InClass, FInstancedPropertyBag& InOutBag)
{
	InOutBag = MakeActualizedPropertyBag(InClass, InOutBag);
}
#endif // WITH_EDITOR


FClassBagCompatiblePropertiesIterator::FClassBagCompatiblePropertiesIterator(const UClass* InClass)
	: It(TFieldIterator<FProperty>(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated, EFieldIteratorFlags::IncludeInterfaces))
{
	IterateToNext();
}

void FClassBagCompatiblePropertiesIterator::IterateToNext()
{
	while (It)
	{
		if (const FProperty* Property = *It; IsPropertyOverridable(Property))
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
