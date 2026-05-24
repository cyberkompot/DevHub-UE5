// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "IPropertyTypeCustomization.h"

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
