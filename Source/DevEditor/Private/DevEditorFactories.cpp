// Copyright (c) Alexandr Pereverzev.

#include "DevEditorFactories.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Pages/DevPadControllerPage.h"

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed= !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InClass->CanCreateAssetOfClass()
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};


bool UDevPadPage_Factory::ConfigureProperties()
{
	PadPageClass = nullptr;

	FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.InitiallySelectedClass = UDevPadControllerPage::StaticClass();
	Options.ExtraPickerCommonClasses.Add(UDevPadControllerPage::StaticClass());

	const TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
	Filter->AllowedChildrenOfClasses.Add(SupportedClass);

	const FText TitleText = INVTEXT("Pick Class For DevHub Pad Page");
	UClass* ChosenClass = UDevPadControllerPage::StaticClass();
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, SupportedClass);

	if ( bPressedOk )
	{
		PadPageClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UDevPadPage_Factory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return (PadPageClass)
		? NewObject<UDevPadPage>(InParent, PadPageClass, InName, Flags | RF_Transactional)
		: NewObject<UDevPadPage>(InParent, InClass, InName, Flags);
}
