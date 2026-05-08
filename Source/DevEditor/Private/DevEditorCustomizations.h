// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "StructUtils/PropertyBag.h"

class FStructOnScope;

struct FDevActionObjectCustomization final : IPropertyTypeCustomization, FGCObject
{
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDevActionObjectCustomization()); }

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FDevActionObjectCustomization"); }
	//~ End FGCObject Interface

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	UClass* GetActionClass() const;
	FInstancedPropertyBag* GetActionProperties() const;
	//~ End IPropertyTypeCustomization Interface

private:
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> ActionClassHandle;
	TSharedPtr<IPropertyHandle> ActionPropertiesHandle;

	FInstancedPropertyBag DisplayBag;
	TSharedPtr<FStructOnScope> DisplayScope;

	void Initialize();
	void Reset();

	void OnActionClassChanged();
	void OnDisplayBagChanged();
};

struct FDevInputShortcutCustomization final : IPropertyTypeCustomization
{
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDevInputShortcutCustomization()); }

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual bool ShouldInlineKey() const override { return true; }
	//~ End IPropertyTypeCustomization Interface
};

struct FDevMenuEntryIdCustomization final : IPropertyTypeCustomization
{
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDevMenuEntryIdCustomization()); }

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual bool ShouldInlineKey() const override { return true; }
	//~ End IPropertyTypeCustomization Interface
};
