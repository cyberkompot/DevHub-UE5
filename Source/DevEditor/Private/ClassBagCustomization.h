// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "ClassBag.h"

class FStructOnScope;

struct FClassBagInlineClassIdentifier final : IPropertyTypeIdentifier
{
	inline static bool SuppressCustomisation = false;
	virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override;
};

struct FClassBagClassPropertyIdentifier final : IPropertyTypeIdentifier
{
	inline static bool SuppressCustomisation = false;
	virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override;
};

struct FClassBagInlineClassCustomization final : IPropertyTypeCustomization
{
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FClassBagInlineClassCustomization()); }

	//~ Begin IPropertyTypeCustomization Interface.
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override { /** Nop */ }
	//~ End IPropertyTypeCustomization Interface.
};

struct FClassBagClassPropertyCustomization final : IPropertyTypeCustomization, FGCObject
{
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FClassBagClassPropertyCustomization()); }

	virtual ~FClassBagClassPropertyCustomization() override;

	//~ Begin FGCObject Interface.
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FDevPropertyBagSchemaCustomization"); }
	//~ End FGCObject Interface.

	//~ Begin IPropertyTypeCustomization Interface.
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface.

private:
	TSharedPtr<IPropertyHandle> ClassHandle;
	TSharedPtr<IPropertyHandle> BagHandle;

	TObjectPtr<UClass> ObjectClass = nullptr;
	FInstancedPropertyBag* ObjectProperties = nullptr;

	FInstancedPropertyBag CDOBag;
	FInstancedPropertyBag DisplayBag;
	TSharedPtr<FStructOnScope> DisplayScope = nullptr;

	TSharedPtr<IPropertyUtilities> PropertyUtilities = nullptr;

	FDelegateHandle BlueprintClassRecompiledHandle;
	FDelegateHandle NativeClassReloadedHandle;
	FDelegateHandle ObjectsReinstancedHandle;

	void Initialize();

	UClass* GetObjectClass() const;
	FInstancedPropertyBag* GetObjectProperties() const;

	void OnSchemaChanged();

	void OnDisplayBagChanged();
	bool OnDisplayBagPropertyIsResetToDefaultVisible(TSharedPtr<IPropertyHandle> PropertyHandle);
	void OnDisplayBagPropertyResetToDefaultClicked(TSharedPtr<IPropertyHandle> PropertyHandle);

	void OnBlueprintClassRecompiled(UBlueprint* Blueprint);
	void OnNativeClassReloaded(EReloadCompleteReason Reason);
	void OnObjectsReinstanced(const TMap<UObject*, UObject*>& ObjectMap);
};

struct FClassBagCustomizations final
{
	void Initialize();
	void Uninitialize();

private:
	TSharedPtr<IPropertyTypeIdentifier> DevPropertyBagInlineSchemaIdentifier = nullptr;
	TSharedPtr<IPropertyTypeIdentifier> PropertyBagSchemaPropertyIdentifier = nullptr;
};
