// Copyright (c) Alexandr Pereverzev.

#include "DevEditorCustomizations.h"

#include "DetailWidgetRow.h"
#include "DevActionTypes.h"
#include "DevMenuTypes.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
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
	Collector.AddReferencedObject(ObjectClass);
	FDevCorePropertyBagUtils::AddPropertyBagToReferenceCollector(Collector, CDOBag);
	FDevCorePropertyBagUtils::AddPropertyBagToReferenceCollector(Collector, DisplayBag);
}

FDevActionObjectCustomization::~FDevActionObjectCustomization()
{
	if (OnObjectsReinstancedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReinstanced.Remove(OnObjectsReinstancedHandle);
	}
}

void FDevActionObjectCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ObjectClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionScript));
	ObjectPropertiesHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDevActionObject, ActionProperties));

	ObjectClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnObjectClassChanged));

	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();
	OnObjectsReinstancedHandle = FCoreUObjectDelegates::OnObjectsReinstanced.AddSP(this, &FDevActionObjectCustomization::OnObjectsReinstanced);

	Initialize();

	HeaderRow
		//.CopyAction(FUIAction(FExecuteAction::CreateSP(this, &FDevActionObjectCustomization::OnCopyAction)))
		//.PasteAction(FUIAction(FExecuteAction::CreateSP(this, &FDevActionObjectCustomization::OnPasteAction), FCanExecuteAction::CreateSP(this, &FDevActionObjectCustomization::OnCanExecutePasteAction)))
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		.VAlign(VAlign_Center)
		[
			ObjectClassHandle->CreatePropertyValueWidget()
		]
		.IsEnabled(PropertyHandle->IsEditable() && ObjectPropertiesHandle->IsEditable() );
}

void FDevActionObjectCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	ObjectClassHandle->MarkHiddenByCustomization();
	ObjectPropertiesHandle->MarkHiddenByCustomization();

	if (!ObjectClass) { return; }
	if (!ObjectProperties) { return; }
	if (DisplayBag.GetNumPropertiesInBag() == 0) { return; }

	ObjectPropertiesHandle->RemoveChildren();

	const TArray<TSharedPtr<IPropertyHandle>> BagPropertyHandles = ObjectPropertiesHandle->AddChildStructure(DisplayScope.ToSharedRef());
	const FSimpleDelegate OnBagPropertyChanged = FSimpleDelegate::CreateSP(this, &FDevActionObjectCustomization::OnDisplayBagChanged);

	const TWeakPtr<FDevActionObjectCustomization> WeakSelf = StaticCastSharedRef<FDevActionObjectCustomization>(AsShared());
	for (const TSharedPtr<IPropertyHandle>& BagPropertyHandle : BagPropertyHandles)
	{
		if (!BagPropertyHandle) { continue; }

		BagPropertyHandle->SetOnPropertyValueChanged(OnBagPropertyChanged);
		BagPropertyHandle->SetOnChildPropertyValueChanged(OnBagPropertyChanged);

		IDetailPropertyRow& BagPropertyRow = ChildBuilder.AddProperty(BagPropertyHandle.ToSharedRef());
		BagPropertyRow.OverrideResetToDefault(FResetToDefaultOverride::Create(
			FIsResetToDefaultVisible::CreateSP(this, &FDevActionObjectCustomization::OnDisplayBagPropertyIsResetToDefaultVisible),
			FResetToDefaultHandler::CreateSP(this, &FDevActionObjectCustomization::OnDisplayBagPropertyResetToDefaultClicked)));
	}
}

void FDevActionObjectCustomization::Initialize()
{
	ObjectClass = GetObjectClass();
	ObjectProperties = GetObjectProperties();

	if (ObjectClass && ObjectProperties)
	{
		CDOBag = FDevCorePropertyBagUtils::MakePropertyBagByClass(ObjectClass, true);

		DisplayBag = *ObjectProperties;
		DisplayBag.MigrateToNewBagInstance(CDOBag);

		*ObjectProperties = FDevCorePropertyBagUtils::MakePropertyBagWithClassOverrides(DisplayBag, ObjectClass);
	}

	if (DisplayBag.IsValid())
	{
		DisplayScope = MakeShared<FStructOnScope>(DisplayBag.GetPropertyBagStruct(), DisplayBag.GetMutableValue().GetMemory());
	}
}

void FDevActionObjectCustomization::Reset()
{
	ObjectClassHandle.Reset();
	ObjectPropertiesHandle.Reset();
	CDOBag.Reset();
	DisplayBag.Reset();
	DisplayScope.Reset();
}

UClass* FDevActionObjectCustomization::GetObjectClass() const
{
	void* RawData = nullptr;
	ObjectClassHandle->GetValueData(RawData);
	const TSoftClassPtr<UObject>* ObjectClassPtr = static_cast<TSoftClassPtr<UObject>*>(RawData);
	UClass* ObjectClassValue = (ObjectClassPtr && !ObjectClassPtr->IsNull()) ? ObjectClassPtr->LoadSynchronous() : nullptr;
	return ObjectClassValue;
}

FInstancedPropertyBag* FDevActionObjectCustomization::GetObjectProperties() const
{
	void* RawData = nullptr;
	ObjectPropertiesHandle->GetValueData(RawData);
	FInstancedPropertyBag* ObjectPropertiesValue = static_cast<FInstancedPropertyBag*>(RawData);
	return ObjectPropertiesValue;
}

void FDevActionObjectCustomization::OnObjectClassChanged()
{
	if (!PropertyUtilities) { return; }

	ObjectPropertiesHandle->NotifyPreChange();
	PropertyUtilities->RequestForceRefresh();
	ObjectPropertiesHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
}

void FDevActionObjectCustomization::OnDisplayBagChanged()
{
	if (!ObjectClass) { return; }
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

bool FDevActionObjectCustomization::OnDisplayBagPropertyIsResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (!ObjectClass) { return false; }

	const FName DisplayBagPropertyName = PropertyHandle->GetProperty()->GetFName();
	const FPropertyBagPropertyDesc* DisplayBagPropertyDesc = DisplayBag.FindPropertyDescByName(DisplayBagPropertyName);
	if (!DisplayBagPropertyDesc) { return false; }

	const FGuid DisplayBagPropertyId = DisplayBagPropertyDesc->ID;
	const FPropertyBagPropertyDesc* CDOBagPropertyDesc = CDOBag.FindPropertyDescByID(DisplayBagPropertyId);
	if (!CDOBagPropertyDesc) { return false; }

	const FProperty* CDOBagProperty = CDOBagPropertyDesc->CachedProperty;
	const FProperty* DisplayBagProperty = DisplayBagPropertyDesc->CachedProperty;

	const uint8* CDOBagMemory = CDOBag.GetValue().GetMemory();
	const uint8* DisplayBagMemory = DisplayBag.GetValue().GetMemory();

	const void* CDOBagValuePtr = CDOBagProperty->ContainerPtrToValuePtr<void>(CDOBagMemory);
	const void* DisplayBagValuePtr = DisplayBagProperty->ContainerPtrToValuePtr<void>(DisplayBagMemory);

	return !CDOBagProperty->Identical(DisplayBagValuePtr, CDOBagValuePtr);
}

void FDevActionObjectCustomization::OnDisplayBagPropertyResetToDefaultClicked(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	const FName DisplayBagPropertyName = PropertyHandle->GetProperty()->GetFName();
	const FPropertyBagPropertyDesc* DisplayBagPropertyDesc = DisplayBag.FindPropertyDescByName(DisplayBagPropertyName);
	if (!DisplayBagPropertyDesc) { return; }

	const FGuid DisplayBagPropertyId = DisplayBagPropertyDesc->ID;
	const FPropertyBagPropertyDesc* CDOBagPropertyDesc = CDOBag.FindPropertyDescByID(DisplayBagPropertyId);
	if (!CDOBagPropertyDesc) { return; }

	const FProperty* CDOBagProperty = CDOBagPropertyDesc->CachedProperty;
	const FProperty* DisplayBagProperty = DisplayBagPropertyDesc->CachedProperty;

	const uint8* CDOBagMemory = CDOBag.GetValue().GetMemory();
	uint8* DisplayBagMemory = DisplayBag.GetMutableValue().GetMemory();

	const void* CDOBagValuePtr = CDOBagProperty->ContainerPtrToValuePtr<void>(CDOBagMemory);

	DisplayBagProperty->SetValue_InContainer(DisplayBagMemory, CDOBagValuePtr);
}

void FDevActionObjectCustomization::OnObjectsReinstanced(const TMap<UObject*, UObject*>& ObjectMap)
{
	if (!PropertyUtilities) { return; }
	if (ObjectMap.IsEmpty()) { return; }

	// Force update the details when BP is compiled, since we may cached hold references to the old object or class.
	PropertyUtilities->RequestRefresh();
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
