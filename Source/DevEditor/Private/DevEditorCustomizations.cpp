// Copyright (c) Alexandr Pereverzev.

#include "DevEditorCustomizations.h"

#include "DetailWidgetRow.h"
#include "DevActionTypes.h"
#include "DevMenuTypes.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
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
	ObjectClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionScript));
	ObjectPropertiesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionProperties));

	Initialize();

	ChildBuilder.AddProperty(ObjectClassHandle.ToSharedRef());
	ObjectClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnObjectClassChanged, TWeakPtr<IPropertyUtilities>(CustomizationUtils.GetPropertyUtilities())));

	ObjectPropertiesHandle->MarkHiddenByCustomization();
	if (DisplayBag.IsValid())
	{
		IDetailPropertyRow* ActionPropertiesRow = ChildBuilder.AddExternalStructure(DisplayScope.ToSharedRef());
		ActionPropertiesRow->DisplayName(ObjectPropertiesHandle->GetPropertyDisplayName());
		ActionPropertiesRow->GetPropertyHandle()->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnDisplayBagChanged));
	}
}

UClass* FDevActionObjectCustomization::GetObjectClass() const
{
	void* RawData = nullptr;
	ObjectClassHandle->GetValueData(RawData);
	const TSoftClassPtr<UObject>* ObjectClassPtr = static_cast<TSoftClassPtr<UObject>*>(RawData);
	UClass* ObjectClass = (ObjectClassPtr && !ObjectClassPtr->IsNull()) ? ObjectClassPtr->LoadSynchronous() : nullptr;
	return ObjectClass;
}

FInstancedPropertyBag* FDevActionObjectCustomization::GetObjectProperties() const
{
	void* RawData = nullptr;
	ObjectPropertiesHandle->GetValueData(RawData);
	FInstancedPropertyBag* ObjectProperties = static_cast<FInstancedPropertyBag*>(RawData);
	return ObjectProperties;
}

void FDevActionObjectCustomization::Initialize()
{
	const UClass* ObjectClass = GetObjectClass();
	FInstancedPropertyBag* ObjectProperties = GetObjectProperties();

	DisplayBag.Reset();
	DisplayScope.Reset();

	if (ObjectClass && ObjectProperties)
	{
		if (!bInitialized)
		{
			DisplayBag = *ObjectProperties; // First initialization based on ObjectProperties.
		}

		const FInstancedPropertyBag ObjectClassBag = FDevCorePropertyBagUtils::MakePropertyBagByClass(ObjectClass, true);
		DisplayBag.MigrateToNewBagInstance(ObjectClassBag);
	}

	if (DisplayBag.IsValid())
	{
		DisplayScope = MakeShared<FStructOnScope>(DisplayBag.GetPropertyBagStruct(), DisplayBag.GetMutableValue().GetMemory());
	}

	bInitialized = true;
}

void FDevActionObjectCustomization::Reset()
{
	ObjectClassHandle.Reset();
	ObjectPropertiesHandle.Reset();
	DisplayBag.Reset();
	DisplayScope.Reset();
}

void FDevActionObjectCustomization::OnObjectClassChanged(const TWeakPtr<IPropertyUtilities> PropertyUtilitiesPtr)
{
	if (const TSharedPtr<IPropertyUtilities> PropertyUtilities = PropertyUtilitiesPtr.Pin())
	{
		PropertyUtilities->RequestForceRefresh(); //ForceRefresh();
	}
}

void FDevActionObjectCustomization::OnDisplayBagChanged()
{
	const UClass* ObjectClass = GetObjectClass();
	if (!ObjectClass) { return; }

	FInstancedPropertyBag* ObjectProperties = GetObjectProperties();
	if (!ObjectProperties) { return; }

	const FInstancedPropertyBag ObjectPropertiesOverrides = FDevCorePropertyBagUtils::MakePropertyBagWithClassOverrides(DisplayBag, ObjectClass);

	if (ObjectProperties->GetValue() != ObjectPropertiesOverrides.GetValue())
	{
		// Only dirty the asset if the property bag actually changed.
		ObjectPropertiesHandle->NotifyPreChange();
		*ObjectProperties = ObjectPropertiesOverrides;
		ObjectPropertiesHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
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
