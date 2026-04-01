// Copyright (c) Alexandr Pereverzev.

#include "DevMenusModule.h"

#include "DevMenuLogging.h"
#include "DevMenuTypes.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"

IMPLEMENT_MODULE(FDevMenusModule, DevMenus)

FDevMenusModule* FDevMenusModule::Get()
{
	return FModuleManager::Get().GetModulePtr<FDevMenusModule>(TEXT("DevMenus"));
}

void FDevMenusModule::StartupModule()
{
	UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDevMenusModule::OnAssetManagerReady));
}

void FDevMenusModule::ShutdownModule()
{
	if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
	{
		AssetRegistry->OnFilesLoaded().RemoveAll(this);
	}
}

void FDevMenusModule::BroadcastPrimaryAssetTypeReady()
{
	bPrimaryAssetTypeReady = true;
	OnPrimaryAssetTypeReady.Broadcast();
	OnPrimaryAssetTypeReady.Clear();
}

void FDevMenusModule::OnAssetManagerReady()
{
	FModuleManager::Get().LoadModule(TEXT("AssetRegistry"), ELoadModuleFlags::LogFailures);

	if (IAssetRegistry* AssetRegistry = IAssetRegistry::Get())
	{
		if (AssetRegistry->IsLoadingAssets())
		{
			AssetRegistry->OnFilesLoaded().AddRaw(this, &FDevMenusModule::OnAssetRegistryReady);
		}
		else
		{
			OnAssetRegistryReady();
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Asset Registry is unavailable. The menu primary asset type could not be registered"));
		BroadcastPrimaryAssetTypeReady();
	}
}

void FDevMenusModule::OnAssetRegistryReady()
{
	IAssetRegistry::Get()->OnFilesLoaded().RemoveAll(this);

	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Registering Dev Menu primary asset type"));

	// See UGameFeaturesSubsystem::AddGameFeatureToAssetManager() for references.
	UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.PushBulkScanning();

	// Registering DevMenu primary asset type.
	auto CreateDirectoryPath = [](const FString& InPath) { FDirectoryPath DirectoryPath{}; DirectoryPath.Path = InPath; return DirectoryPath; };

	FPrimaryAssetTypeInfo TypeInfo;
	TypeInfo.PrimaryAssetType = UDevMenu::PrimaryAssetType;
	TypeInfo.bHasBlueprintClasses = false;
	TypeInfo.bIsEditorOnly = false;
	TypeInfo.Rules.bApplyRecursively = true;
	TypeInfo.Rules.CookRule = EPrimaryAssetCookRule::DevelopmentAlwaysCook;
#if WITH_EDITOR
	TypeInfo.SetAssetBaseClass(UDevMenu::StaticClass());
	TypeInfo.GetDirectories().Emplace(CreateDirectoryPath(TEXT("/Game")));
	TypeInfo.GetDirectories().Emplace(CreateDirectoryPath(TEXT("/DevMenus")));
#endif // WITH_EDITOR

	// This function fills out runtime data on the copy.
	if (AssetManager.ShouldScanPrimaryAssetType(TypeInfo))
	{
		FPrimaryAssetTypeInfo ExistingAssetTypeInfo;
		const bool bAlreadyExisted = AssetManager.GetPrimaryAssetTypeInfo(FPrimaryAssetType(TypeInfo.PrimaryAssetType), ExistingAssetTypeInfo);
		AssetManager.ScanPathsForPrimaryAssets(TypeInfo.PrimaryAssetType, TypeInfo.AssetScanPaths, TypeInfo.AssetBaseClassLoaded, TypeInfo.bHasBlueprintClasses, TypeInfo.bIsEditorOnly, false /** No need for force scan after OnAssetRegistryReady() event. */);

		if (!bAlreadyExisted)
		{
			// If we did not previously scan anything for our primary asset type, try to reuse the cook rules from the config instead of the one in the code, which should not be modifying cook rules.
			const FPrimaryAssetTypeInfo* ConfigTypeInfo = AssetManager.GetSettings().PrimaryAssetTypesToScan.FindByPredicate([&TypeInfo](const FPrimaryAssetTypeInfo& PATI) -> bool { return PATI.PrimaryAssetType == TypeInfo.PrimaryAssetType; });
			if (ConfigTypeInfo)
			{
				UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Registering Dev Menu primary with Asset Manager projects settings"));
				AssetManager.SetPrimaryAssetTypeRules(TypeInfo.PrimaryAssetType, ConfigTypeInfo->Rules);
			}
			else
			{
				UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Registering Dev Menu primary with default settings"));
				AssetManager.SetPrimaryAssetTypeRules(TypeInfo.PrimaryAssetType, TypeInfo.Rules);
			}
		}
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Registering Dev Menu primary asset type not needed. Primary asset type is already registered"));
	}

	AssetManager.PopBulkScanning();

	BroadcastPrimaryAssetTypeReady();
}
