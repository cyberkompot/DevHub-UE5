// Copyright (c) Alexandr Pereverzev.

#include "DevEditorCustomizations.h"

#include "DetailWidgetRow.h"
#include "DevActionTypes.h"
#include "DevMenuTypes.h"
#include "IDetailChildrenBuilder.h"
#include "Framework/DevCorePropertyBag.h"

PRAGMA_DISABLE_OPTIMIZATION

namespace DevMenu::Editor
{
	bool GVarShowEntryId = true;
	static FAutoConsoleVariableRef CVarShowEntryId(TEXT("DevHub.Editor.ShowEntryId"), GVarShowEntryId, TEXT("Display and modify Dev Menu Entry IDs."));
}

using namespace DevMenu::Editor;

void FDevActionObjectCustomization::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (UPropertyBag* DisplayBagStruct = const_cast<UPropertyBag*>(DisplayBag.GetPropertyBagStruct()))
	{
		Collector.AddReferencedObject(DisplayBagStruct);
	}
}

void FDevActionObjectCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	];
}

void FDevActionObjectCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ActionClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionScript));
	ActionPropertiesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionProperties));

	Initialize();

	ChildBuilder.AddProperty(ActionClassHandle.ToSharedRef());
	ActionPropertiesHandle->MarkHiddenByCustomization();
	IDetailPropertyRow* ActionPropertiesRow = ChildBuilder.AddExternalStructure(DisplayScope.ToSharedRef());
	ActionPropertiesRow->DisplayName(ActionPropertiesHandle->GetPropertyDisplayName());

	ActionClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnActionClassChanged));
	ActionPropertiesRow->GetPropertyHandle()->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnDisplayBagChanged));
}

UClass* FDevActionObjectCustomization::GetActionClass() const
{
	void* RawData = nullptr;
	ActionClassHandle->GetValueData(RawData);
	TSoftClassPtr<UObject>* ActionClassPtr = static_cast<TSoftClassPtr<UObject>*>(RawData);
	UClass* ActionClass = (ActionClassPtr && !ActionClassPtr->IsNull()) ? ActionClassPtr->LoadSynchronous() : nullptr;
	return ActionClass;
}

FInstancedPropertyBag* FDevActionObjectCustomization::GetActionProperties() const
{
	void* RawData = nullptr;
	ActionPropertiesHandle->GetValueData(RawData);
	FInstancedPropertyBag* ActionProperties = static_cast<FInstancedPropertyBag*>(RawData);
	return ActionProperties;
}

void FDevActionObjectCustomization::Initialize()
{
	const bool bInit = (!DisplayScope.IsValid());

	UClass* ActionClass = GetActionClass();
	FInstancedPropertyBag* ActionProperties = GetActionProperties();

	if (bInit)
	{
		DisplayBag = (ActionProperties) ? *ActionProperties : FInstancedPropertyBag();
		DisplayScope = MakeShared<FStructOnScope>();
	}

	TArray<FProperty*, TInlineAllocator<32>> ActionClassProperties;
	TArray<FPropertyBagPropertyDesc, TInlineAllocator<32>> ActionClassBagDescs;
	if (ActionClass)
	{
		TSet<FName> AddedProperties; // Avoid duplicates by name (can happen via interface + class, or shadowed members).
		for (TFieldIterator<FProperty> It(ActionClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated, EFieldIteratorFlags::IncludeInterfaces); It; ++It)
		{
			FProperty* Property = *It;
			const FName PropertyName = Property->GetFName();
			if (AddedProperties.Contains(PropertyName)) { continue; }
			if (!FDevCorePropertyBagUtils::IsPropertyOverrideable(Property)) { continue; }

			AddedProperties.Add(PropertyName);
			ActionClassProperties.Emplace(Property);
			ActionClassBagDescs.Emplace(FDevCorePropertyBagUtils::MakePropertyDesc(ActionClass, Property));
		}
	}

	FInstancedPropertyBag ActionClassBag;
	ActionClassBag.AddProperties(ActionClassBagDescs);
	const UPropertyBag* ActionClassBagStruct = ActionClassBag.GetPropertyBagStruct();

	// CDO default values.
	if (ActionClass)
	{
		const UObject* ActionCDO = ActionClass->GetDefaultObject();
		for (const FProperty* Property : ActionClassProperties)
		{
			ActionClassBag.SetValue(Property->GetFName(), Property, ActionCDO);
		}
	}

	DisplayBag.MigrateToNewBagInstance(ActionClassBag);
	*DisplayScope = FStructOnScope(DisplayBag.GetPropertyBagStruct(), DisplayBag.GetMutableValue().GetMutableMemory());


/*
	TArray<FName, TInlineAllocator<32>> ActionPropertiesToRemove;
	const UPropertyBag* ActionPropertiesBagStruct = ActionProperties->GetPropertyBagStruct();
	const TConstArrayView<FPropertyBagPropertyDesc> ActionPropertiesBagDescs = ActionPropertiesBagStruct->GetPropertyDescs();
	for (const FPropertyBagPropertyDesc& ActionPropertiesPropertyDesc : ActionPropertiesBagDescs)
	{
		bool bPropertyFound = ActionClassBagDescs.FindByPredicate([ID = ActionPropertiesPropertyDesc.ID](const FPropertyBagPropertyDesc& Item)
		{
			return (Item.ID == ID);
		});
		if (!bPropertyFound)
		{
			ActionPropertiesToRemove.Add(ActionPropertiesPropertyDesc.Name);
		}
	}

	FInstancedPropertyBag IntersectingBag = ActionClassBag;
	IntersectingBag.RemovePropertiesByName()
	for (const FPropertyBagPropertyDesc& ActionClassPropertyDesc : ActionClassBagDescs)
	{
		bool bPropertyFound = ActionClassBagDescs.FindByPredicate([ID = ActionClassPropertyDesc.ID](const FPropertyBagPropertyDesc& Item)
		{
			return (Item.ID == ID);
		});
		if (bPropertyFound)
		{
			const FProperty* DisplayProp = Desc.CachedProperty;
			const FProperty* CDOProp = Class->FindPropertyByName(ActionClassPropertyDesc.Name);
			if (!DisplayProp || !CDOProp)
			{
				continue;
			}

			const void* DisplayValuePtr =
				DisplayProp->ContainerPtrToValuePtr<void>(const_cast<uint8*>(DisplayMem));
			const void* CDOValuePtr = CDOProp->ContainerPtrToValuePtr<void>(CDO);

			if (CDOProp->Identical(DisplayValuePtr, CDOValuePtr))
			{
				// Value matches CDO default — remove from sparse bag
				Action->ActionProperties.RemovePropertyByName(Desc.Name);
			}
			else
			{
				// Value differs from CDO default — store as override
				Action->ActionProperties.SetValue(
					Desc.Name, DisplayProp, const_cast<uint8*>(DisplayMem));
			}
		}
		if (!ActionPropertiesBagDescs.FindByPredicate([ID = ActionClassPropertyDesc.ID] { return  }))
			|| ActionProperties->) { continue; }
	}


	const UPropertyBag* BagB = ParametersB.GetPropertyBagStruct();
	if (!BagA || !BagB)
	{
		return BagA == BagB;
	}

	const TConstArrayView<FPropertyBagPropertyDesc> DescsA = BagA->GetPropertyDescs();

	ActionProperties->GetNumPropertiesInBag()
	ActionProperties->FindPropertyDescByID()
	const UPropertyBag* ActionClassBagStruct = IntersectingBag.GetPropertyBagStruct();
	ActionProperties->MigrateToNewBagInstance(ActionClassBag);
	*/
}

void FDevActionObjectCustomization::Reset()
{
	ActionClassHandle.Reset();
	ActionPropertiesHandle.Reset();
	DisplayBag.Reset();
	DisplayScope.Reset();
}

void FDevActionObjectCustomization::OnActionClassChanged()
{
	Initialize(); // Re-initialise.
}



TArray<FProperty*> GetClassOverrideableProperties(const UClass& InClass)
{
	TArray<FProperty*, TInlineAllocator<32>> ClassProperties;
		TSet<FName> AddedProperties; // Avoid duplicates by name (can happen via interface + class, or shadowed members).
		for (TFieldIterator<FProperty> It(&InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::ExcludeDeprecated, EFieldIteratorFlags::IncludeInterfaces); It; ++It)
		{
			FProperty* Property = *It;
			const FName PropertyName = Property->GetFName();
			if (AddedProperties.Contains(PropertyName)) { continue; }
			if (!FDevCorePropertyBagUtils::IsPropertyOverrideable(Property)) { continue; }

			AddedProperties.Add(PropertyName);
			ClassProperties.Emplace(Property);
		}
	return TArray<FProperty*>(ClassProperties);
}

FInstancedPropertyBag GetPropertyBagWithOnlyNonDefaultValues(const FInstancedPropertyBag& InBag, const UClass& InClass, const TConstArrayView<FProperty*> InClassPropertiesToCheck)
{
	TArray<FPropertyBagPropertyDesc, TInlineAllocator<32>> ReducedBagDescs;
	if (const uint8* BagMemory = InBag.GetValue().GetMemory())
	{
		if (const UObject* CDO = InClass.GetDefaultObject())
		{
			for (const FProperty* ClassProperty : InClassPropertiesToCheck)
			{
				if (const FPropertyBagPropertyDesc* BagPropertyDesc = InBag.FindPropertyDescByID(FDevCorePropertyBagUtils::GetStablePropertyId(&InClass, ClassProperty->GetFName())))
				{
					const FProperty* BagProperty = BagPropertyDesc->CachedProperty;

					const void* BagValuePtr = BagProperty->ContainerPtrToValuePtr<void>(BagMemory);
					const void* CDOValuePtr = ClassProperty->ContainerPtrToValuePtr<void>(CDO);

					if (!ClassProperty->Identical(BagValuePtr, CDOValuePtr))
					{
						ReducedBagDescs.Emplace(*BagPropertyDesc);
					}
				}
			}
		}
	}

	FInstancedPropertyBag ReducedBag;
	if (!ReducedBagDescs.IsEmpty())
	{
		ReducedBag.AddProperties(ReducedBagDescs);
		ReducedBag.CopyMatchingValuesByID(InBag);
	}
	return ReducedBag;
}

FInstancedPropertyBag GetPropertyBagWithOnlyNonDefaultValues(const FInstancedPropertyBag& InBag, const UClass& InClass)
{
	const TArray<FProperty*> ClassProperties = GetClassOverrideableProperties(InClass);
	return GetPropertyBagWithOnlyNonDefaultValues(InBag, InClass, ClassProperties);
}


void FDevActionObjectCustomization::OnDisplayBagChanged()
{
	const UClass* ActionClass = GetActionClass();
	if (!ActionClass) { return; }

	FInstancedPropertyBag* ActionProperties = GetActionProperties();
	if (!ActionProperties) { return; }

	FInstancedPropertyBag ChangedActionProperties = GetPropertyBagWithOnlyNonDefaultValues(DisplayBag, *ActionClass);
	if (ActionProperties->GetValue() != ChangedActionProperties.GetValue())
	{
		// Only dirty the asset if the property bag actually changed.
		ActionPropertiesHandle->NotifyPreChange();
		*ActionProperties = ChangedActionProperties;
		ActionPropertiesHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FDevInputShortcutCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		PropertyHandle->GetChildHandle("ShortcutName")->CreatePropertyValueWidget()
	];
}

void FDevMenuEntryIdCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (!GVarShowEntryId) { return; }

	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevMenuEntryId, Id))->CreatePropertyValueWidget()
	];
}

PRAGMA_ENABLE_OPTIMIZATION



/*
	TSharedPtr<IPropertyHandle> ValueHandle = PropertyHandle->GetChildHandle(TEXT("Value"));
	if (ValueHandle.IsValid())
	{
		// Snapshot current state to detect actual changes
		const FInstancedPropertyBag OldProperties = AbilityData->OverridableProperties;

		// update the bag with newest data, something might have changed in meantime
		AbilityData->UpdateProperties(Owner.Get());

		// Only dirty the asset if the property bag actually changed
		if (!AbilityData->OverridableProperties.Identical(&OldProperties, PPF_None))
		{
			ValueHandle->NotifyPreChange();
			if (EditorData)
			{
				EditorData->OnParametersChanged(*EditorData->GetOuterUStateTree());
			}
			ValueHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			RefreshPropertiesView(OverridablePropertyHandle, PropUtils);
		}
	}

	if (!ActionClassHandle.IsValid() || !StructHandle.IsValid())
	{
		return;
	}

	UObject* ClassObject = nullptr;
	ActionClassHandle->GetValue(ClassObject);
	const UClass* Class = Cast<UClass>(ClassObject);
	if (!Class)
	{
		return;
	}

	const UPropertyBag* DisplayBagStruct = DisplayBag.GetPropertyBagStruct();
	if (!DisplayBagStruct)
	{
		return;
	}

	const uint8* DisplayMem = DisplayBag.GetValue().GetMemory();
	if (!DisplayMem)
	{
		return;
	}

	UObject* CDO = Class->GetDefaultObject();

	TArray<void*> RawData;
	StructHandle->AccessRawData(RawData);

	ActionPropertiesHandle->NotifyPreChange();

	for (void* Data : RawData)
	{
		FWxHitZoneBlueprintAction* Action = static_cast<FWxHitZoneBlueprintAction*>(Data);

		for (const FPropertyBagPropertyDesc& Desc : DisplayBagStruct->GetPropertyDescs())
		{
			const FProperty* DisplayProp = Desc.CachedProperty;
			const FProperty* CDOProp = Class->FindPropertyByName(Desc.Name);
			if (!DisplayProp || !CDOProp)
			{
				continue;
			}

			const void* DisplayValuePtr =
				DisplayProp->ContainerPtrToValuePtr<void>(const_cast<uint8*>(DisplayMem));
			const void* CDOValuePtr = CDOProp->ContainerPtrToValuePtr<void>(CDO);

			if (CDOProp->Identical(DisplayValuePtr, CDOValuePtr))
			{
				// Value matches CDO default — remove from sparse bag
				Action->ActionProperties.RemovePropertyByName(Desc.Name);
			}
			else
			{
				// Value differs from CDO default — store as override
				Action->ActionProperties.SetValue(
					Desc.Name, DisplayProp, const_cast<uint8*>(DisplayMem));
			}
		}
	}

	ActionPropertiesHandle->NotifyPostChange(EPropertyChangeType::ValueSet);

*/
