// Copyright (c) Alexandr Pereverzev.

#include "Entries/DevMenuConsoleVariableButtons.h"

#include "DevMenuFactory.h"
#include "DevMenuLogging.h"
#include "DevMenuUtils.h"
#include "Actions/DevActionConsoleVariable.h"

FDevMenuInstancedEntries FDevMenuConsoleVariableGroup::CreateDynamicEntries() const
{
	FDevMenuInstancedEntries DynamicEntries;

	if (CVarName.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("CVar is not defined"));
		return DynamicEntries;
	}

#if !NO_CVARS
	DynamicEntries.Reserve(CVarValuesInfos.Num());

	for (const FDevMenuConsoleVariableValueInfo& CVarValuesInfo : CVarValuesInfos)//FDevMenuConsoleVariableButton
	{
		const FName DynamicEntryName = (!CVarValuesInfo.CVarValue.IsEmpty()) ? FName(CVarValuesInfo.CVarValue) : EDevMenuPaths::Undefined;
		DynamicEntries.Emplace(FDevMenuFactory::CreateEntry<FDevMenuConsoleVariableButton>(
			DynamicEntryName,
			FormatEntryLabel(CVarValuesInfo),
			[this, &CVarValuesInfo](FDevMenuConsoleVariableButton& Entry)
			{
				Entry.CVarName = CVarName;
				Entry.CVarValue = CVarValuesInfo.CVarValue;
			}));
		/*
		DynamicEntries.Emplace(FDevMenuFactory::CreateActionButton<FDevActionConsoleVariable>(
			DynamicEntryName,
			FormatEntryLabel(CVarValuesInfo),
			FDevActionConsoleVariable(CVarName, CVarValuesInfo.CVarValue)));
			*/
	}
#endif // !NO_CVARS

	return DynamicEntries;
}

FText FDevMenuConsoleVariableGroup::FormatEntryLabel(const FDevMenuConsoleVariableValueInfo& CVarValueInfo) const
{
	const FText& Fmt = (CVarValueInfo.LabelOverride.IsEmpty()) ? LabelFormat : CVarValueInfo.LabelOverride;
	return FText::FormatNamed(Fmt,
		TEXT("Variable"), FText::FromString(CVarName),
		TEXT("Value"), FText::FromString(CVarValueInfo.CVarValue));
}
