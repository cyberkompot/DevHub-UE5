// Copyright (c) Alexandr Pereverzev.

#include "ClassBagCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyBagDetails.h"

namespace ClassBag::Editor
{
	const FLazyName ClassPropertyMetaKeyName = "ClassProperty";
	const FLazyName InlineClassMetaKeyName = "InlineClass";

	bool ClassBagCustomization = true;
	static FAutoConsoleVariableRef CVarClassBagCustomization(TEXT("ClassBag.Editor.Customizations"), ClassBagCustomization, TEXT("Possibility to disable ClassBag customization for debugging purposes."));

	bool IsClassTypeProperty(const FProperty* Property)
	{
		return CastField<FSoftClassProperty>(Property)
			|| CastField<FClassProperty>(Property)
			|| (CastField<FObjectProperty>(Property)
				&& CastField<FObjectProperty>(Property)->PropertyClass == UClass::StaticClass())
			|| (CastField<FStructProperty>(Property)
				&& CastField<FStructProperty>(Property)->Struct == FSoftClassPath::StaticStruct());
	}

	void ConstructDefaultCustomizationWithWarning(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, FText&& WarningText, const TFunction<TSharedRef<SWidget>()>& ValueWidget)
	{
		HeaderRow
			.NameContent()
			[
				PropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.MinDesiredWidth(250.f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(FMargin(2, 0, 4, 0))
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Icons.WarningWithColor"))
					.DesiredSizeOverride(FVector2D(16, 16))
					.ToolTipText(Forward<FText>(WarningText))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				.FillWidth(1)
				[
					ValueWidget()
				]
			]
			.IsEnabled(PropertyHandle->IsEditable());
	}

	TSharedPtr<IPropertyHandle> GetPropertyHandleForClassProperty(const TSharedPtr<IPropertyHandle>& PropertyHandle, FText& OutWarningText)
	{
		const TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle->GetParentHandle();
		if (!ParentHandle) { return nullptr; }

		const FName SchemaPropertyName = PropertyHandle->HasMetaData(ClassPropertyMetaKeyName) ? FName(PropertyHandle->GetMetaData(ClassPropertyMetaKeyName)) : NAME_Name;
		if (SchemaPropertyName.IsNone())
		{
			OutWarningText = INVTEXT("“ClassProperty” meta tag value is missing.");
			return nullptr;
		}

		TSharedPtr<IPropertyHandle> SchemaHandle = ParentHandle->GetChildHandle(SchemaPropertyName);
		const FProperty* SchemaProperty = (SchemaHandle) ? SchemaHandle->GetProperty() : nullptr;
		if (!SchemaProperty)
		{
			OutWarningText = FText::Format(INVTEXT("No sibling shema property named “{0}” was found."), FText::FromName(SchemaPropertyName));
			return nullptr;
		}

		if (IsClassTypeProperty(SchemaProperty))
		{
			OutWarningText = FText::GetEmpty();
			return SchemaHandle;
		}

		OutWarningText = FText::Format(INVTEXT("Sibling property shema “{0}” must be FSoftClassPath, TSoftClassPtr, TSubclassOf or TObjectPtr<UClass>."), FText::FromName(SchemaPropertyName));
		return nullptr;
	}

	TSharedPtr<IPropertyHandle> GetPropertyHandleForInlineClass(const TSharedPtr<IPropertyHandle>& PropertyHandle, FText& OutWarningText)
	{
		const TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle->GetParentHandle();
		if (!ParentHandle) { return nullptr; }

		uint32 NumChildren = 0;
		if (ParentHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success) { return nullptr; }

		const FString PropertyName = PropertyHandle->GetProperty()->GetFName().ToString();
		for (uint32 i = 0; i < NumChildren; ++i)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = ParentHandle->GetChildHandle(i);
			const FProperty* ChildProperty = ChildHandle->GetProperty();

			const FStructProperty* ChildStructProperty = CastField<FStructProperty>(ChildProperty);
			if (ChildStructProperty
				&& ChildStructProperty->Struct == FInstancedPropertyBag::StaticStruct()
				&& ChildHandle->HasMetaData(ClassPropertyMetaKeyName)
				&& ChildHandle->GetMetaData(ClassPropertyMetaKeyName) == PropertyName)
			{
				return ChildHandle;
			}
		}

		OutWarningText = FText::Format(INVTEXT("No sibling property bag found with meta = (ClassProperty = \"{0}\")."), FText::FromString(PropertyName));
		return nullptr;
	}
}

using namespace ClassBag::Editor;


bool FClassBagInlineClassIdentifier::IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const
{
	return ClassBagCustomization && !SuppressCustomisation && PropertyHandle.HasMetaData(InlineClassMetaKeyName) && IsClassTypeProperty(PropertyHandle.GetProperty());
}

bool FClassBagClassPropertyIdentifier::IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const
{
	return ClassBagCustomization && !SuppressCustomisation && PropertyHandle.HasMetaData(ClassPropertyMetaKeyName);
}

void FClassBagInlineClassCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FText WarningText;

	if (GetPropertyHandleForInlineClass(PropertyHandle, WarningText))
	{
		HeaderRow.Visibility(EVisibility::Collapsed);
	}
	else
	{
		const TGuardValue<bool> SuppressCustomisationGuard(FClassBagInlineClassIdentifier::SuppressCustomisation, true);
		ConstructDefaultCustomizationWithWarning(PropertyHandle, HeaderRow, MoveTemp(WarningText),
			[&PropertyHandle]
			{
			 return PropertyHandle->CreatePropertyValueWidgetWithCustomization(nullptr);
			});
	}
}

FClassBagClassPropertyCustomization::~FClassBagClassPropertyCustomization()
{
	if (ClassHandle)
	{
		ClassHandle->SetOnPropertyValueChanged(FSimpleDelegate());
	}
	if (BlueprintClassRecompiledHandle.IsValid())
	{
		if (UBlueprint* Blueprint = UBlueprint::GetBlueprintFromClass(ObjectClass))
		{
			Blueprint->OnCompiled().Remove(BlueprintClassRecompiledHandle);
		}
	}
	if (NativeClassReloadedHandle.IsValid())
	{
		FCoreUObjectDelegates::ReloadCompleteDelegate.Remove(NativeClassReloadedHandle);
	}
	if (ObjectsReinstancedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReinstanced.Remove(ObjectsReinstancedHandle);
	}
}

void FClassBagClassPropertyCustomization::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ObjectClass);
	FClassBagUtils::AddPropertyBagToReferenceCollector(Collector, CDOBag);
	FClassBagUtils::AddPropertyBagToReferenceCollector(Collector, DisplayBag);
}

void FClassBagClassPropertyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FText WarningText;

	ClassHandle = GetPropertyHandleForClassProperty(PropertyHandle, WarningText);
	if (ClassHandle)
	{
		BagHandle = PropertyHandle;
		PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

		ClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FClassBagClassPropertyCustomization::OnSchemaChanged));

		Initialize();

		HeaderRow
			.NameContent()
			[
				PropertyHandle->CreatePropertyNameWidget()
			];
		if (ClassHandle->HasMetaData(InlineClassMetaKeyName))
		{
			TGuardValue<bool> SuppressCustomisationGuard(FClassBagInlineClassIdentifier::SuppressCustomisation, true);
			HeaderRow
				.ValueContent()
				.MinDesiredWidth(250.f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.FillWidth(1)
					[
						ClassHandle->CreatePropertyValueWidgetWithCustomization(nullptr)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						ClassHandle->CreateDefaultPropertyButtonWidgets()
					]
				]
				.IsEnabled(PropertyHandle->IsEditable());
		}
	}
	else
	{
		const TGuardValue<bool> SuppressCustomisationGuard(FClassBagClassPropertyIdentifier::SuppressCustomisation, true);
		ConstructDefaultCustomizationWithWarning(PropertyHandle, HeaderRow, MoveTemp(WarningText),
			[&PropertyHandle, &CustomizationUtils]
			{
				return FPropertyBagDetails::MakeAddPropertyWidget(PropertyHandle, CustomizationUtils.GetPropertyUtilities()).ToSharedRef();
			});
	}
}

void FClassBagClassPropertyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// If bag's class is missing, invalid, or wrong-type, use the standard FInstancedPropertyBag children renderer.
	if (!ClassHandle)
	{
		FPropertyBagInstanceDataDetails::FConstructParams Params;
		Params.BagStructProperty = PropertyHandle;
		Params.PropUtils = CustomizationUtils.GetPropertyUtilities();
		ChildBuilder.AddCustomBuilder(MakeShared<FPropertyBagInstanceDataDetails>(Params));
		return;
	}

	PropertyHandle->MarkHiddenByCustomization();

	if (!ObjectClass) { return; }
	if (!ObjectProperties) { return; }
	if (DisplayBag.GetNumPropertiesInBag() == 0) { return; }

	PropertyHandle->RemoveChildren();

	const TArray<TSharedPtr<IPropertyHandle>> BagPropertyHandles = PropertyHandle->AddChildStructure(DisplayScope.ToSharedRef());
	const FSimpleDelegate OnBagPropertyChanged = FSimpleDelegate::CreateSP(this, &FClassBagClassPropertyCustomization::OnDisplayBagChanged);
	for (const TSharedPtr<IPropertyHandle>& BagPropertyHandle : BagPropertyHandles)
	{
		if (!BagPropertyHandle) { continue; }

		BagPropertyHandle->SetOnPropertyValueChanged(OnBagPropertyChanged);
		BagPropertyHandle->SetOnChildPropertyValueChanged(OnBagPropertyChanged);

		IDetailPropertyRow& BagPropertyRow = ChildBuilder.AddProperty(BagPropertyHandle.ToSharedRef());
		BagPropertyRow.OverrideResetToDefault(FResetToDefaultOverride::Create(
			FIsResetToDefaultVisible::CreateSP(this, &FClassBagClassPropertyCustomization::OnDisplayBagPropertyIsResetToDefaultVisible),
			FResetToDefaultHandler::CreateSP(this, &FClassBagClassPropertyCustomization::OnDisplayBagPropertyResetToDefaultClicked)));
	}

	// Layout update on class reload and recompilation.
	if (UBlueprint* Blueprint = UBlueprint::GetBlueprintFromClass(ObjectClass))
	{
		BlueprintClassRecompiledHandle = Blueprint->OnCompiled().AddSP(this, &FClassBagClassPropertyCustomization::OnBlueprintClassRecompiled);
	}
	else
	{
		NativeClassReloadedHandle = FCoreUObjectDelegates::ReloadCompleteDelegate.AddSP(this, &FClassBagClassPropertyCustomization::OnNativeClassReloaded);
	}

	ObjectsReinstancedHandle = FCoreUObjectDelegates::OnObjectsReinstanced.AddSP(this, &FClassBagClassPropertyCustomization::OnObjectsReinstanced);
}

void FClassBagClassPropertyCustomization::Initialize()
{
	ObjectClass = GetObjectClass();
	ObjectProperties = GetObjectProperties();

	if (ObjectClass && ObjectProperties)
	{
		CDOBag = FClassBagUtils::MakePropertyBagByClass(ObjectClass, true);

		DisplayBag = *ObjectProperties;
		DisplayBag.MigrateToNewBagInstance(CDOBag);

		*ObjectProperties = FClassBagUtils::MakePropertyBagWithClassOverrides(DisplayBag, ObjectClass);
	}

	if (DisplayBag.IsValid())
	{
		DisplayScope = MakeShared<FStructOnScope>(DisplayBag.GetPropertyBagStruct(), DisplayBag.GetMutableValue().GetMemory());
	}
}

UClass* FClassBagClassPropertyCustomization::GetObjectClass() const
{
	if (!ClassHandle) { return nullptr; }

	void* RawData = nullptr;
	if (ClassHandle->GetValueData(RawData) != FPropertyAccess::Success || !RawData) { return nullptr; }

	const FProperty* SchemaProperty = ClassHandle->GetProperty();
	if (CastField<FSoftClassProperty>(SchemaProperty))
	{
		const TSoftClassPtr<UObject>* SoftClassPtr = static_cast<TSoftClassPtr<UObject>*>(RawData);
		return (SoftClassPtr && !SoftClassPtr->IsNull()) ? SoftClassPtr->LoadSynchronous() : nullptr;
	}
	if (CastField<FObjectProperty>(SchemaProperty))
	{
		const TObjectPtr<UObject>* ObjectPtr = static_cast<TObjectPtr<UObject>*>(RawData);
		return (ObjectPtr) ? Cast<UClass>(ObjectPtr->Get()) : nullptr;
	}
	if (CastField<FClassProperty>(SchemaProperty))
	{
		UClass** ClassPtr = static_cast<UClass**>(RawData);
		return ClassPtr ? *ClassPtr : nullptr;
	}
	if (const FStructProperty* StructProperty = CastField<FStructProperty>(SchemaProperty); StructProperty && StructProperty->Struct == FSoftClassPath::StaticStruct())
	{
		const FSoftClassPath* SoftClassPathPtr = static_cast<FSoftClassPath*>(RawData);
		return (SoftClassPathPtr && !SoftClassPathPtr->IsNull()) ? SoftClassPathPtr->ResolveClass() : nullptr;
	}

	return nullptr;
}

FInstancedPropertyBag* FClassBagClassPropertyCustomization::GetObjectProperties() const
{
	if (!BagHandle.IsValid()) { return nullptr; }

	void* RawData = nullptr;
	if (BagHandle->GetValueData(RawData) != FPropertyAccess::Success) { return nullptr; }
	return static_cast<FInstancedPropertyBag*>(RawData);
}

void FClassBagClassPropertyCustomization::OnSchemaChanged()
{
	if (!PropertyUtilities) { return; }
	if (!BagHandle.IsValid()) { return; }

	BagHandle->NotifyPreChange();
	PropertyUtilities->RequestForceRefresh();
	BagHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
}

void FClassBagClassPropertyCustomization::OnDisplayBagChanged()
{
	if (!ObjectClass) { return; }
	if (!ObjectProperties) { return; }
	if (!BagHandle.IsValid()) { return; }

	const FInstancedPropertyBag ObjectPropertiesOverrides = FClassBagUtils::MakePropertyBagWithClassOverrides(DisplayBag, ObjectClass);

	if (ObjectProperties->GetValue() != ObjectPropertiesOverrides.GetValue())
	{
		// Only dirty the asset if the property bag actually changed.
		BagHandle->NotifyPreChange();
		*ObjectProperties = ObjectPropertiesOverrides;
		BagHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

bool FClassBagClassPropertyCustomization::OnDisplayBagPropertyIsResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
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

void FClassBagClassPropertyCustomization::OnDisplayBagPropertyResetToDefaultClicked(TSharedPtr<IPropertyHandle> PropertyHandle)
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

void FClassBagClassPropertyCustomization::OnBlueprintClassRecompiled(UBlueprint* Blueprint)
{
	if (PropertyUtilities)
	{
		PropertyUtilities->RequestForceRefresh();
	}
}

void FClassBagClassPropertyCustomization::OnNativeClassReloaded(EReloadCompleteReason Reason)
{
	if (PropertyUtilities)
	{
		PropertyUtilities->RequestForceRefresh();
	}
}

void FClassBagClassPropertyCustomization::OnObjectsReinstanced(const TMap<UObject*, UObject*>& ObjectMap)
{
	// Force update the details when Blueprint is compiled, since we may cached hold references to the old object or class.
	if (ObjectClass && ObjectMap.Contains(ObjectClass))
	{
		if (PropertyUtilities)
		{
			PropertyUtilities->RequestForceRefresh();
		}
	}
}

void FClassBagCustomizations::Initialize()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DevPropertyBagInlineSchemaIdentifier = MakeShared<FClassBagInlineClassIdentifier>();
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FSoftClassProperty::StaticClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier);
	/*
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FObjectProperty::StaticClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier);
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FClassProperty::StaticClass()->GetFName(), // ClassProperty
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier);
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FStructProperty::StaticClass()->GetFName(), //StructProperty
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier);
	*/
	
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FInstancedPropertyBag::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagClassPropertyCustomization::MakeInstance),
		(PropertyBagSchemaPropertyIdentifier = MakeShared<FClassBagClassPropertyIdentifier>()));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FClassBagCustomizations::Uninitialize()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomPropertyTypeLayout(FSoftClassProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		/*
		PropertyModule.UnregisterCustomPropertyTypeLayout(FObjectProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		PropertyModule.UnregisterCustomPropertyTypeLayout(FClassProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		PropertyModule.UnregisterCustomPropertyTypeLayout(FStructProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		*/
		
		PropertyModule.UnregisterCustomPropertyTypeLayout(FInstancedPropertyBag::StaticStruct()->GetFName(), PropertyBagSchemaPropertyIdentifier);

		PropertyModule.NotifyCustomizationModuleChanged();
	}
}
