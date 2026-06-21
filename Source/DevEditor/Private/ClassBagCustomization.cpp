// Copyright (c) Alexandr Pereverzev.

#include "ClassBagCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyBagDetails.h"

#if UE_VERSION_OLDER_THAN(5, 6, 0)
#include "PropertyEditor/Private/PropertyNode.h"
#include "PropertyEditor/Private/PropertyHandleImpl.h"

// See DEFINE_PRIVATE_ACCESS in https://github.com/VoxelPlugin/VoxelCore/blob/master/Source/VoxelCore/Public/VoxelMinimal/VoxelMacros.h
#define DEFINE_PRIVATE_ACCESS(Class, Property) \
	namespace PrivateAccess \
	{ \
		template<typename> \
		struct TClass_ ## Property; \
		\
		template<> \
		struct TClass_ ## Property<Class> \
		{ \
			template<auto PropertyPtr> \
			struct TProperty_ ## Property \
			{ \
				friend auto& Property(Class& Object) \
				{ \
					return Object.*PropertyPtr; \
				} \
				friend auto& Property(const Class& Object) \
				{ \
					return Object.*PropertyPtr; \
				} \
			}; \
		}; \
		template struct TClass_ ## Property<Class>::TProperty_ ## Property<&Class::Property>; \
		\
		auto& Property(Class& Object); \
		auto& Property(const Class& Object); \
	}

namespace ClassBag::EngineCompatibility
{
	DEFINE_PRIVATE_ACCESS(FPropertyNode, ChildNodes);
}
#endif

namespace ClassBag::EngineCompatibility
{
	TSharedPtr<SWidget> FPropertyBagDetails_MakeAddPropertyWidget(const TSharedRef<IPropertyHandle> PropertyHandle, const IPropertyTypeCustomizationUtils& CustomizationUtils)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 2, 0)
		return FPropertyBagDetails::MakeAddPropertyWidget(PropertyHandle, CustomizationUtils.GetPropertyUtilities());
#else
		return FPropertyBagDetails::MakeAddPropertyWidget(PropertyHandle, CustomizationUtils.GetPropertyUtilities().Get());
#endif
	}

	EPropertyChangeType::Type EPropertyChangeType_ResetToDefault()
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 6, 0)
		return  EPropertyChangeType::ResetToDefault;
#else
		return EPropertyChangeType::ValueSet;
#endif
	};

	uint8* FStructView_GetMutableValue(FStructView StructView)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 3, 0)
		return StructView.GetMemory();
#else
		return StructView.GetMutableMemory();
#endif
	};

	TSharedRef<SWidget> IPropertyHandle_CreatePropertyValueWidgetWithCustomization(const TSharedPtr<IPropertyHandle>& PropertyHandle, const IDetailsView* DetailsView)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 4, 0)
		return PropertyHandle->CreatePropertyValueWidgetWithCustomization(nullptr);
#else
		return PropertyHandle->CreatePropertyValueWidget(false);
#endif
	}

	void IPropertyHandle_RemoveChildren(const TSharedRef<IPropertyHandle>& PropertyHandle)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 6, 0)
		PropertyHandle->RemoveChildren();
#else
		if (const TSharedPtr<FPropertyNode> PropertyNode = StaticCastSharedRef<FPropertyHandleBase>(PropertyHandle)->GetPropertyNode())
		{
			PrivateAccess::ChildNodes(*PropertyNode).Empty();
		}
#endif
	}

	void IPropertyUtilities_RequestForceRefresh(const TSharedPtr<IPropertyUtilities>& PropertyUtilities)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 4, 0)
		PropertyUtilities->RequestForceRefresh();
#else
		static bool bForceRefreshRequested = false;
		if (!bForceRefreshRequested)
		{
			bForceRefreshRequested = true;
			PropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([PropertyUtilitiesWeak = TWeakPtr<IPropertyUtilities>(PropertyUtilities)]
			{
				bForceRefreshRequested = false;
				if (const TSharedPtr<IPropertyUtilities> PropertyUtilitiesPtr = PropertyUtilitiesWeak.Pin())
				{
					PropertyUtilitiesPtr->ForceRefresh();
				}
			}));
		}
#endif
	}
}

using namespace ClassBag::EngineCompatibility;


namespace ClassBag::Editor
{
	const FLazyName ClassPropertyMetaKeyName = "ClassProperty";
	const FLazyName InlineClassMetaKeyName = "InlineClass";

	bool ClassBagCustomization = true;
	static FAutoConsoleVariableRef CVarClassBagCustomization(TEXT("ClassBag.Editor.Customizations"), ClassBagCustomization, TEXT("Possibility to disable ClassBag customization for debugging purposes."));

	bool ArePropertyBagsEqual(const FInstancedPropertyBag& InA, const FInstancedPropertyBag& InB)
	{
		const UPropertyBag* StructA = InA.GetPropertyBagStruct();
		const UPropertyBag* StructB = InB.GetPropertyBagStruct();
		return (StructA == StructB) && (!StructA || StructA->CompareScriptStruct(InA.GetValue().GetMemory(), InB.GetValue().GetMemory(), PPF_None));
	}

	bool IsClassTypeProperty(const FProperty* Property)
	{
		return CastField<FSoftClassProperty>(Property)
			|| CastField<FClassProperty>(Property)
			|| (CastField<FObjectProperty>(Property)
				&& CastField<FObjectProperty>(Property)->PropertyClass == UClass::StaticClass())
			|| (CastField<FStructProperty>(Property)
				&& CastField<FStructProperty>(Property)->Struct == TBaseStructure<FSoftClassPath>::Get());
	}

	bool IsContainerTypeProperty(const FProperty* Property)
	{
		return CastField<FArrayProperty>(Property)
			|| CastField<FSetProperty>(Property)
			|| CastField<FMapProperty>(Property);
	}

	bool IsInstancedPropertyBagProperty(const FProperty* Property)
	{
		return CastField<FStructProperty>(Property)
			&& CastField<FStructProperty>(Property)->Struct == FInstancedPropertyBag::StaticStruct();
	}

	bool IsContainerPropertyHandle(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		return (PropertyHandle.IsValid() && IsContainerTypeProperty(PropertyHandle->GetProperty()));
	}

	bool IsGroupPropertyHandle(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		return (PropertyHandle.IsValid() && !PropertyHandle->GetProperty());
	}

	TSharedPtr<IPropertyHandle> FindOwnerHandle(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		// Climb past category-like handles that have no backing FProperty, and container properties (TArray/TSet/TMap).
		TSharedPtr<IPropertyHandle> OwnerPropertyHandle = nullptr;
		for (TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle; (ParentHandle = ParentHandle->GetParentHandle()); /** Nop. */)
		{
			OwnerPropertyHandle = ParentHandle;
			if (!IsGroupPropertyHandle(OwnerPropertyHandle) && !IsContainerPropertyHandle(OwnerPropertyHandle)) { break; }
		}
		return OwnerPropertyHandle;
	}

	TSharedPtr<IPropertyHandle> FindChildHandleRecursive(const TSharedPtr<IPropertyHandle>& PropertyHandle, const TFunctionRef<bool(const TSharedPtr<IPropertyHandle>&)>& Predicate)
	{
		if (!PropertyHandle.IsValid()) { return nullptr; }

		uint32 NumChildren = 0;
		if (PropertyHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success) { return nullptr; }

		for (uint32 i = 0; i < NumChildren; ++i)
		{
			TSharedPtr<IPropertyHandle> ChildPropertyHandle = PropertyHandle->GetChildHandle(i);
			if (!ChildPropertyHandle.IsValid()) { continue; }

			if (Predicate(ChildPropertyHandle))
			{
				return ChildPropertyHandle;
			}
			if (!ChildPropertyHandle->GetProperty())
			{
				// Descend into category-like handles that have no backing FProperty, allowing traversal across category boundaries.
				if (TSharedPtr<IPropertyHandle> Found = FindChildHandleRecursive(ChildPropertyHandle, Predicate))
				{
					return Found;
				}
			}
		}

		return nullptr;
	}

	TSharedPtr<IPropertyHandle> GetPropertyHandleForClassProperty(const TSharedPtr<IPropertyHandle>& PropertyHandle, FText& OutWarningText)
	{
		const TSharedPtr<IPropertyHandle> OwnerHandle = FindOwnerHandle(PropertyHandle);
		if (!OwnerHandle) { return nullptr; }

		const FName ClassPropertyName = PropertyHandle->HasMetaData(ClassPropertyMetaKeyName) ? FName(PropertyHandle->GetMetaData(ClassPropertyMetaKeyName)) : NAME_None;
		if (ClassPropertyName.IsNone())
		{
			OutWarningText = INVTEXT("“ClassProperty” meta tag value is missing.");
			return nullptr;
		}

		TSharedPtr<IPropertyHandle> ClassPropertyHandle = FindChildHandleRecursive(OwnerHandle, [ClassPropertyName](const TSharedPtr<IPropertyHandle>& ChildPropertyHandle)
		{
			const FProperty* ChildProperty = ChildPropertyHandle->GetProperty();
			return ChildProperty && (ChildProperty->GetFName() == ClassPropertyName);
		});
		const FProperty* ClassProperty = (ClassPropertyHandle) ? ClassPropertyHandle->GetProperty() : nullptr;

		if (!ClassProperty)
		{
			OutWarningText = FText::Format(INVTEXT("Class property “{0}” was not found."), FText::FromName(ClassPropertyName));
			return nullptr;
		}

		if (!IsClassTypeProperty(ClassProperty))
		{
			OutWarningText = FText::Format(INVTEXT("Class property “{0}” must be of type FSoftClassPath, TSoftClassPtr, TSubclassOf or TObjectPtr<UClass>."), FText::FromName(ClassPropertyName));
			return nullptr;
		}

		OutWarningText = FText::GetEmpty();
		return ClassPropertyHandle;
	}

	TSharedPtr<IPropertyHandle> GetPropertyHandleForInlineClass(const TSharedPtr<IPropertyHandle>& PropertyHandle, FText& OutWarningText)
	{
		const TSharedPtr<IPropertyHandle> OwnerHandle = FindOwnerHandle(PropertyHandle);
		if (!OwnerHandle) { return nullptr; }

		const FProperty* Property = PropertyHandle->GetProperty();
		if (!Property) { return nullptr; }

		const TStringView<FNameBuilder::ElementType> PropertyName = FNameBuilder(Property->GetFName()).ToView();

		bool bFoundInsideContainer = false;
		TSharedPtr<IPropertyHandle> BagPropertyHandle = FindChildHandleRecursive(OwnerHandle, [&PropertyName, &bFoundInsideContainer](const TSharedPtr<IPropertyHandle>& ChildPropertyHandle)
		{
			const FProperty* ChildProperty = ChildPropertyHandle->GetProperty();
			if (!ChildProperty) { return false; }
			if (!ChildPropertyHandle->HasMetaData(ClassPropertyMetaKeyName)) { return false; }
			if (ChildPropertyHandle->GetMetaData(ClassPropertyMetaKeyName) != PropertyName) { return false; }

			if (IsInstancedPropertyBagProperty(ChildProperty)) { return true; }

			bFoundInsideContainer =	(CastField<FArrayProperty>(ChildProperty) && IsInstancedPropertyBagProperty(CastField<FArrayProperty>(ChildProperty)->Inner))
									|| (CastField<FSetProperty>(ChildProperty) && IsInstancedPropertyBagProperty(CastField<FSetProperty>(ChildProperty)->ElementProp))
									|| (CastField<FMapProperty>(ChildProperty) && IsInstancedPropertyBagProperty(CastField<FMapProperty>(ChildProperty)->ValueProp));
			return bFoundInsideContainer;
		});
		const FProperty* BagProperty = (BagPropertyHandle) ? BagPropertyHandle->GetProperty() : nullptr;

		if (!BagProperty)
		{
			OutWarningText = FText::Format(INVTEXT("Bag property specified by meta = (ClassProperty = \"{0}\") was not found."), FText::FromStringView(PropertyName));
			return nullptr;
		}

		if (bFoundInsideContainer)
		{
			OutWarningText = FText::Format(INVTEXT("Bag property specified by meta = (ClassProperty = \"{0}\") was found in a container property (TArray, TSet, or TMap). “InlineClass” metadata tag is ignored."), FText::FromStringView(PropertyName));
			return nullptr;
		}

		if (!IsInstancedPropertyBagProperty(BagProperty))
		{
			OutWarningText = FText::Format(INVTEXT("Bag property specified by meta = (ClassProperty = \"{0}\") must be of type FInstancedPropertyBag."), FText::FromStringView(PropertyName));
			return nullptr;
		}

		OutWarningText = FText::GetEmpty();
		return BagPropertyHandle;
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
				return IPropertyHandle_CreatePropertyValueWidgetWithCustomization(PropertyHandle, nullptr);
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
	const UScriptStruct* BagStruct = FInstancedPropertyBag::StaticStruct();
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 2, 0)
	Collector.AddPropertyReferencesWithStructARO(BagStruct, &CDOBag);
	Collector.AddPropertyReferencesWithStructARO(BagStruct, &DisplayBag);
#else
	Collector.AddReferencedObjects(BagStruct, &CDOBag);
	Collector.AddReferencedObjects(BagStruct, &DisplayBag);
#endif
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
		const bool bInlineClassMode = ClassHandle->HasMetaData(InlineClassMetaKeyName) && !IsContainerPropertyHandle(PropertyHandle->GetParentHandle()); // InlineClass is no make sense for bags in TArray/TSet/TMap.

		Initialize();

		HeaderRow
			.NameContent()
			[
				PropertyHandle->CreatePropertyNameWidget()
			];
		if (bInlineClassMode)
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
						IPropertyHandle_CreatePropertyValueWidgetWithCustomization(ClassHandle, nullptr)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						ClassHandle->CreateDefaultPropertyButtonWidgets()
					]
				]
				.IsEnabled(PropertyHandle->IsEditable())
				.OverrideResetToDefault(FResetToDefaultOverride::Create(
					FIsResetToDefaultVisible::CreateSP(this, &FClassBagClassPropertyCustomization::OnInlineClassIsResetToDefaultVisible),
					FResetToDefaultHandler::CreateSP(this, &FClassBagClassPropertyCustomization::OnInlineClassResetToDefaultClicked)));
		}
		else
		{
			const FSlateFontInfo ValueFont = FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont");
			const FText ValueText = (ObjectClass) ? FText::FromName(ObjectClass->GetFName()) : INVTEXT("None");
			HeaderRow
				.ValueContent()
				.VAlign(VAlign_Center)
				[
					SNew(SEditableTextBox)
					.IsEnabled(false)
					.Font(ValueFont)
					.Text(ValueText)
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
				return FPropertyBagDetails_MakeAddPropertyWidget(PropertyHandle, CustomizationUtils).ToSharedRef();
			});
	}
}

void FClassBagClassPropertyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// If bag's class is missing, invalid, or wrong-type, use the standard FInstancedPropertyBag children renderer.
	if (!ClassHandle)
	{
#if UE_VERSION_NEWER_THAN_OR_EQUAL(5, 6, 0)
		FPropertyBagInstanceDataDetails::FConstructParams Params;
		Params.BagStructProperty = PropertyHandle;
		Params.PropUtils = CustomizationUtils.GetPropertyUtilities();
		TSharedRef<FPropertyBagInstanceDataDetails> CustomBuilder = MakeShared<FPropertyBagInstanceDataDetails>(Params);
#elif UE_VERSION_NEWER_THAN_OR_EQUAL(5, 2, 0)
		TSharedRef<FPropertyBagInstanceDataDetails> CustomBuilder = MakeShared<FPropertyBagInstanceDataDetails>(PropertyHandle, CustomizationUtils.GetPropertyUtilities(), false);
#else
		TSharedRef<FPropertyBagInstanceDataDetails> CustomBuilder = MakeShared<FPropertyBagInstanceDataDetails>(PropertyHandle, CustomizationUtils.GetPropertyUtilities().Get(), false);
#endif
		ChildBuilder.AddCustomBuilder(MoveTemp(CustomBuilder));
		return;
	}

	PropertyHandle->MarkHiddenByCustomization();

	if (!ObjectClass) { return; }
	if (!GetObjectProperties()) { return; }
	if (DisplayBag.GetNumPropertiesInBag() == 0) { return; }

	IPropertyHandle_RemoveChildren(PropertyHandle);

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

	// Unlike ObjectClass, we cannot cache the bag pointer.
	// The bag may reside in a TArray, TSet, or TMap whose storage can be reallocated, leaving any cached pointer dangling.
	if (FInstancedPropertyBag* ObjectProperties = GetObjectProperties())
	{
		// Always actualize the bag data to resolve redirects and type changes,
		// but do not mark the asset as dirty, as this is not a user-initiated modification.
		FClassBagUtils::ActualizePropertyBag(ObjectClass, *ObjectProperties);

		if (ObjectClass)
		{
			CDOBag = FClassBagUtils::MakePropertyBagByClass(ObjectClass, true);
			DisplayBag = *ObjectProperties;
			DisplayBag.MigrateToNewBagInstance(CDOBag);
		}
	}

	if (DisplayBag.IsValid())
	{
		DisplayScope = MakeShared<FStructOnScope>(DisplayBag.GetPropertyBagStruct(), FStructView_GetMutableValue(DisplayBag.GetMutableValue()));
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
	if (const FStructProperty* StructProperty = CastField<FStructProperty>(SchemaProperty); StructProperty && StructProperty->Struct == TBaseStructure<FSoftClassPath>::Get())
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
	if (!BagHandle.IsValid()) { return; }

	if (PropertyUtilities)
	{
		IPropertyUtilities_RequestForceRefresh(PropertyUtilities);
	}
}

void FClassBagClassPropertyCustomization::OnDisplayBagChanged()
{
	if (!ObjectClass) { return; }
	if (!BagHandle.IsValid()) { return; }

	FInstancedPropertyBag* ObjectProperties = GetObjectProperties();
	if (!ObjectProperties) { return; }

	const FInstancedPropertyBag ObjectPropertiesOverrides = FClassBagUtils::MakePropertyBagWithClassOverrides(DisplayBag, ObjectClass);

	// Only dirty the asset if the overrides actually changed (deep value compare).
	if (!ArePropertyBagsEqual(*ObjectProperties, ObjectPropertiesOverrides))
	{
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
	if (!PropertyHandle.IsValid() || !PropertyHandle->GetProperty()) { return; }

	const FName DisplayBagPropertyName = PropertyHandle->GetProperty()->GetFName();
	const FPropertyBagPropertyDesc* DisplayBagPropertyDesc = DisplayBag.FindPropertyDescByName(DisplayBagPropertyName);
	if (!DisplayBagPropertyDesc) { return; }

	const FPropertyBagPropertyDesc* CDOBagPropertyDesc = CDOBag.FindPropertyDescByID(DisplayBagPropertyDesc->ID);
	if (!CDOBagPropertyDesc) { return; }

	const FProperty* CDOBagProperty = CDOBagPropertyDesc->CachedProperty;
	const FProperty* DisplayBagProperty = DisplayBagPropertyDesc->CachedProperty;
	if (!CDOBagProperty || !DisplayBagProperty) { return; }

	const uint8* CDOBagMemory = CDOBag.GetValue().GetMemory();
	uint8* DisplayBagMemory = FStructView_GetMutableValue(DisplayBag.GetMutableValue());
	const void* CDOBagValuePtr = CDOBagProperty->ContainerPtrToValuePtr<void>(CDOBagMemory);

	PropertyHandle->NotifyPreChange();
	DisplayBagProperty->SetValue_InContainer(DisplayBagMemory, CDOBagValuePtr);
	PropertyHandle->NotifyPostChange(EPropertyChangeType_ResetToDefault());

	OnDisplayBagChanged();
}

bool FClassBagClassPropertyCustomization::OnInlineClassIsResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	return ClassHandle.IsValid() && ClassHandle->DiffersFromDefault();
}

void FClassBagClassPropertyCustomization::OnInlineClassResetToDefaultClicked(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (ClassHandle.IsValid())
	{
		ClassHandle->ResetToDefault();
	}
}

void FClassBagClassPropertyCustomization::OnBlueprintClassRecompiled(UBlueprint* Blueprint)
{
	if (PropertyUtilities)
	{
		IPropertyUtilities_RequestForceRefresh(PropertyUtilities);
	}
}

void FClassBagClassPropertyCustomization::OnNativeClassReloaded(EReloadCompleteReason Reason)
{
	if (PropertyUtilities)
	{
		IPropertyUtilities_RequestForceRefresh(PropertyUtilities);
	}
}

void FClassBagClassPropertyCustomization::OnObjectsReinstanced(const TMap<UObject*, UObject*>& ObjectMap)
{
	// Force update the details when Blueprint is compiled, since we may cached hold references to the old object or class.
	if (ObjectClass && ObjectMap.Contains(ObjectClass))
	{
		if (PropertyUtilities)
		{
			IPropertyUtilities_RequestForceRefresh(PropertyUtilities);
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
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FObjectProperty::StaticClass()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier); // TODO: Fix support for TObjectPtr<UClass>.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FClassProperty::StaticClass()->GetFName(), // ClassProperty
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier); // TODO: Fix support for TSubclassOf<>.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FStructProperty::StaticClass()->GetFName(), //StructProperty
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassBagInlineClassCustomization::MakeInstance),
		DevPropertyBagInlineSchemaIdentifier); // TODO: Fix support for FSoftClassPath.

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
		PropertyModule.UnregisterCustomPropertyTypeLayout(FObjectProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		PropertyModule.UnregisterCustomPropertyTypeLayout(FClassProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);
		PropertyModule.UnregisterCustomPropertyTypeLayout(FStructProperty::StaticClass()->GetFName(), DevPropertyBagInlineSchemaIdentifier);

		PropertyModule.UnregisterCustomPropertyTypeLayout(FInstancedPropertyBag::StaticStruct()->GetFName(), PropertyBagSchemaPropertyIdentifier);

		PropertyModule.NotifyCustomizationModuleChanged();
	}
}
