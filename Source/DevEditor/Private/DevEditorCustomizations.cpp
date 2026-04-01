// Copyright (c) Alexandr Pereverzev.

#include "DevEditorCustomizations.h"

#include "DetailWidgetRow.h"

namespace DevMenu::Editor
{
	bool GVarShowEntryId = true;
	static FAutoConsoleVariableRef CVarShowEntryId(TEXT("DevHub.Editor.ShowEntryId"), GVarShowEntryId, TEXT("Display and modify Dev Menu Entry IDs."));
}

using namespace DevMenu::Editor;

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
		PropertyHandle->GetChildHandle("Id")->CreatePropertyValueWidget()
	];
}
