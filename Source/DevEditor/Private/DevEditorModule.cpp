// Copyright (c) Alexandr Pereverzev.

#include "DevEditorModule.h"

#include "DevEditorAssetTypeActions.h"
#include "DevEditorCustomizations.h"
#include "DevEditorFactories.h"
#include "ClassBagCustomization.h"
#include "PropertyEditorModule.h"

using namespace DevMenu::Editor;

IMPLEMENT_MODULE(FDevEditorModule, DevEditor)

void FDevEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	if (!IsIntegrationAvailable()) { return; }

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	if (AssetTools.FindAdvancedAssetCategory(MenuCategoryName) == EAssetTypeCategories::Misc)
	{
		AssetTools.RegisterAdvancedAssetCategory(MenuCategoryName, INVTEXT("Debug"));
	}

	/** Class Bag. */
	ClassBagCustomizations.Initialize();

	/** Dev Inputs. */
	PropertyModule.RegisterCustomPropertyTypeLayout(FDevInputShortcut::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDevInputShortcutCustomization::MakeInstance));

	/** Dev Menus. */
	AssetTools.RegisterAssetTypeActions((DevMenuAssetTypeActions = MakeShared<FAssetTypeActions_DevMenu>()).ToSharedRef());
	PropertyModule.RegisterCustomPropertyTypeLayout(FDevMenuEntryId::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDevMenuEntryIdCustomization::MakeInstance));

	/** Dev Pad. */
	AssetTools.RegisterAssetTypeActions((DevPadPageAssetTypeActions = MakeShared<FAssetTypeActions_DevPadPage>()).ToSharedRef());

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FDevEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
	if (!IsIntegrationAvailable()) { return; }

	/** Class Bag. */
	ClassBagCustomizations.Uninitialize();

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		/** Dev Inputs. */
		PropertyModule.UnregisterCustomPropertyTypeLayout(FDevInputShortcut::StaticStruct()->GetFName());

		/** Dev Menus. */
		PropertyModule.UnregisterCustomPropertyTypeLayout(FDevMenuEntryId::StaticStruct()->GetFName());

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

		/** Dev Menus. */
		if (DevMenuAssetTypeActions)
		{
			AssetToolsModule.Get().UnregisterAssetTypeActions(DevMenuAssetTypeActions.ToSharedRef());
		}

		/** Dev Pad. */
		if (DevPadPageAssetTypeActions)
		{
			AssetToolsModule.Get().UnregisterAssetTypeActions(DevPadPageAssetTypeActions.ToSharedRef());
		}
	}
}
