// Copyright (c) Alexandr Pereverzev.

#include "DevEditorModule.h"

#include "DevEditorAssetTypeActions.h"
#include "DevEditorCustomizations.h"
#include "DevEditorFactories.h"
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

	/** Dev Inputs. */
	PropertyModule.RegisterCustomPropertyTypeLayout(FDevInputShortcut::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDevInputShortcutCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	/** Dev Menus. */
	AssetTools.RegisterAssetTypeActions((DevMenuAssetTypeActions = MakeShared<FAssetTypeActions_DevMenu>()).ToSharedRef());

	PropertyModule.RegisterCustomPropertyTypeLayout(FDevMenuEntryId::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDevMenuEntryIdCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	/** Dev Pad. */
	AssetTools.RegisterAssetTypeActions((DevMenuAssetTypeActions = MakeShared<FAssetTypeActions_DevPadPage>()).ToSharedRef());
}

void FDevEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
	if (!IsIntegrationAvailable()) { return; }

	/** Dev Menus. */
	if (DevMenuAssetTypeActions)
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools")) { FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(DevMenuAssetTypeActions.ToSharedRef()); }
		DevMenuAssetTypeActions = nullptr;
	}
}
