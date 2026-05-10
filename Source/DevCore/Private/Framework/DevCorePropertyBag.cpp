// Copyright (c) Alexandr Pereverzev.

#include "Framework/DevCorePropertyBag.h"

namespace DevCore::PropertyBag
{
	void ApplyPropertyBagToObject(UObject* InObject, const FInstancedPropertyBag& InBag, FInstancedPropertyBag* OutOriginalValues)
	{
		if (const uint8* BagMemory = InBag.GetValue().GetMemory(); InObject && BagMemory && InBag.GetNumPropertiesInBag() != 0)
		{
			if (OutOriginalValues)
			{
				OutOriginalValues->AddProperties(InBag.GetPropertyBagStruct()->GetPropertyDescs());
			}

			const UClass* Class = InObject->GetClass();
			for (FDevCoreOverrideablePropertiesIterator It(Class); It; ++It)
			{
				const FProperty* ClassProperty = *It;
				const FGuid ClassPropertyGuid = FDevCorePropertyBagUtils::GetStablePropertyId(Class, ClassProperty->GetFName());
				if (const FPropertyBagPropertyDesc* BagPropertyDesc = InBag.FindPropertyDescByID(ClassPropertyGuid))
				{
					const FProperty* BagProperty = BagPropertyDesc->CachedProperty;
					const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);

					if (OutOriginalValues)
					{
						OutOriginalValues->SetValue(BagPropertyDesc->Name, ClassProperty, InObject);
					}

					ClassProperty->SetValue_InContainer(InObject, BagValuePtr);
				}
			}
		}
		else
		{
			if (OutOriginalValues)
			{
				OutOriginalValues->Reset();
			}

			// Warning
		}
	}

	FGuid FindBlueprintPropertyGuid(const UClass* InClass, const FName InPropertyName)
	{
		// Try to get the Blueprint variable GUID for a property name up the class chain. Works across renames in the same Blueprint.
		for (const UClass* CurrentClass = InClass; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
		{
			// Look at the Blueprint’s variable table.
			if (const UBlueprint* Blueprint = Cast<UBlueprint>(CurrentClass->ClassGeneratedBy))
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

using namespace DevCore::PropertyBag;

FDevCoreOverrideablePropertiesIterator::FDevCoreOverrideablePropertiesIterator(const UClass* InClass)
	: It(TFieldIterator<FProperty>(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated, EFieldIteratorFlags::IncludeInterfaces))
{
	IterateToNext();
}

void FDevCoreOverrideablePropertiesIterator::IterateToNext()
{
	while (It)
	{
		if (FDevCorePropertyBagUtils::IsPropertyOverrideable(*It))
		{
			break;
		}
		else
		{
			++It;
		}
	}
}

void FDevCorePropertyBagUtils::ApplyPropertyBagToObject(UObject* InObject, const FInstancedPropertyBag& InBag)
{
	DevCore::PropertyBag::ApplyPropertyBagToObject(InObject, InBag, nullptr);
}

void FDevCorePropertyBagUtils::ApplyPropertyBagToObject(UObject* InObject, const FInstancedPropertyBag& InBag, FInstancedPropertyBag& OutOriginalValues)
{
	DevCore::PropertyBag::ApplyPropertyBagToObject(InObject, InBag, &OutOriginalValues);
}

bool FDevCorePropertyBagUtils::IsPropertyOverrideable(const FProperty* InProperty)
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

FGuid FDevCorePropertyBagUtils::GetStablePropertyId(const UClass* InClass, const FName InPropertyName)
{
	if (!InClass) { return FGuid(); }

	if (const FGuid Guid = FindBlueprintPropertyGuid(InClass, InPropertyName); Guid.IsValid())
	{
		// Prefer Blueprint var GUID (stable across renames inside the Blueprint).
		return Guid;
	}
	else
	{
		// Deterministic GUID from class path + property name (fallback when no Blueprint GUID and no prior ID).
		const uint32 ClassHash = GetTypeHash(InClass->GetPathName());
		const uint32 NameHash = GetTypeHash(InPropertyName);
		const uint32 Seed = HashCombineFast(ClassHash, NameHash);
		return FGuid(ClassHash, NameHash, 0xF3A9C7D2, 0xE8B4FF91);
	}
}

FPropertyBagPropertyDesc FDevCorePropertyBagUtils::MakePropertyDesc(const UClass* Class, const FProperty* Property)
{
	if (Class && Property)
	{
		const FName PropertyName = Property->GetFName();
		FPropertyBagPropertyDesc Desc(PropertyName, Property);
		Desc.ID = GetStablePropertyId(Class, PropertyName);
		return Desc;
	}
	else
	{
		return FPropertyBagPropertyDesc();
	}
}
