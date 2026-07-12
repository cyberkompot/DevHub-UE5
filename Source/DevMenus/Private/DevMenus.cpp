// Copyright (c) Alexandr Pereverzev.

#include "DevMenus.h"

#include "DevInputs.h"
#include "DevMenuGenerator.h"
#include "DevMenuLogging.h"
#include "DevMenuRegistry.h"
#include "DevMenuManager.h"
#include "DevMenuSettings.h"
#include "DevMenusModule.h"
#include "DevMenuWidgetBuilder.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

#define UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS UE_VERSION_AT_LEAST(5, 1, 0)

namespace DevMenu::Console
{
	static FAutoConsoleCommandWithWorldAndArgs ShowMenuCommand(
		TEXT("DevHub.Menu.ShowMenu"),
		TEXT("Shows the default Dev Menu, or the named Dev Menu if one is provided."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (UDevMenus* DevMenus = UDevMenus::Get(World))
			{
				if (Args.IsEmpty())
				{
					DevMenus->ShowMenu();
				}
				else
				{
					for (const FString& Arg : Args)
					{
						DevMenus->ShowMenu(FName(*Arg));
					}
				}
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs HideMenuCommand(
		TEXT("DevHub.Menu.HideMenu"),
		TEXT("Hides the default Dev Menu, or the named Dev Menu if one is provided."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (UDevMenus* DevMenus = UDevMenus::Get(World))
			{
				if (Args.IsEmpty())
				{
					DevMenus->HideMenu();
				}
				else
				{
					for (const FString& Arg : Args)
					{
						DevMenus->HideMenu(FName(*Arg));
					}
				}
			}
		}));

	static FAutoConsoleCommandWithWorld HideAllMenusCommand(
		TEXT("DevHub.Menu.HideAllMenus"),
		TEXT("Hides all Dev Menus."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](const UWorld* World)
		{
			if (UDevMenus* DevMenus = UDevMenus::Get(World))
			{
				DevMenus->HideAllMenus();
			}
		}));

}


UDevMenus::UDevMenus()
{
	MenuRegistry = CreateDefaultSubobject<UDevMenuRegistry>("MenuRegistry", true);

	MenuGenerator = CreateDefaultSubobject<UDevMenuGenerator>("MenuGenerator", true);
	MenuGenerator->SetMenuRegistry(*MenuRegistry);

	MenuWidgetBuilder = CreateDefaultSubobject<UDevMenuWidgetBuilder>("MenuWidgetBuilder", true);
	MenuWidgetBuilder->SetMenuGenerator(*MenuGenerator);

	MenuManager = CreateDefaultSubobject<UDevMenuManager>("MenuManager", true);
	MenuManager->SetMenuRegistry(*MenuRegistry);
	MenuManager->SetMenuWidgetBuilder(*MenuWidgetBuilder);
}

UDevMenus* UDevMenus::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<ThisClass>();
		}
	}
	return nullptr;
}

void UDevMenus::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Initializing Dev Menus"));

	Collection.InitializeDependency<UDevInputs>();

	MenuManager->Initialize();

	if (const UDevMenuSettings* Settings = GetDefault<UDevMenuSettings>())
	{
		BindShortcuts(*Settings);
		LoadMenuSettings(*Settings);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Dev Menus settings are missing. The menu could not be constructed and bound"));
	}

	LoadMenuAssets();
}

void UDevMenus::Deinitialize()
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Deinitializing Dev Menus"));

	UnbindShortcuts();

	MenuManager->Dispose();
	MenuWidgetBuilder->Dispose();
	MenuRegistry->Dispose();
}

UDevMenu* UDevMenus::RegisterMenu(const FName InMenuPath) const
{
	return MenuRegistry->RegisterMenu(InMenuPath);
}

UDevMenu* UDevMenus::UnregisterMenu(const FName InMenuPath) const
{
	return MenuRegistry->UnregisterMenu(InMenuPath);
}

UDevMenu* UDevMenus::CreateMenu(const FName InMenuPath) const
{
	return MenuRegistry->CreateMenu(InMenuPath);
}

void UDevMenus::AddMenu(UDevMenu* InMenu) const
{
	MenuRegistry->AddMenu(InMenu);
}

void UDevMenus::RemoveMenu(UDevMenu* InMenu) const
{
	MenuRegistry->RemoveMenu(InMenu);
}

void UDevMenus::ShowMenu(const FName InMenuPath)
{
	MenuManager->ShowMenu(InMenuPath);
}

void UDevMenus::ToggleMenu(const FName InMenuPath)
{
	MenuManager->ToggleMenu(InMenuPath);
}

void UDevMenus::HideMenu(const FName InMenuPath)
{
	MenuManager->HideMenu(InMenuPath);
}

void UDevMenus::HideAllMenus()
{
	MenuManager->HideAllMenus();
}

FDevMenuEvent& UDevMenus::OnMenuShow()
{
	return MenuManager->OnMenuShow;
}

FDevMenuEvent& UDevMenus::OnMenuHide()
{
	return MenuManager->OnMenuHide;
}

void UDevMenus::BindShortcuts(const UDevMenuSettings& InSettings)
{
	if (UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		BindMenuShortcut(*DevInputs, EDevMenuPaths::MainMenu, InSettings.Shortcuts.MainMenu);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Dev Inputs is missing. The menu could not be bound"));
	}
}

void UDevMenus::UnbindShortcuts() const
{
	if (UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->ClearBindingsForObject(this);
	}
}

void UDevMenus::BindMenuShortcut(UDevInputs& InDevInputs, const FName InMenuPath, const FDevInputShortcut& InInputShortcut)
{
	if (InInputShortcut.IsNone())
	{
		UE_LOG_FUNCTION(LogDevMenus, Log, TEXT("Dev Menu input shortcut is undefined for menu: %s"), *InMenuPath.ToString());
		return;
	}

	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Dev Menu bound to input shortcut: %s → %s"), *InMenuPath.ToString(), *InInputShortcut.ToString());
	InDevInputs.BindShortcut(InInputShortcut).Delegate.BindDelegate(this, &ThisClass::ToggleMenu, InMenuPath);
}

void UDevMenus::LoadMenuAssets() const
{
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Asset Registry is unavailable. The menu could not be constructed"));
		return;
	}

	if (FDevMenusModule* DevMenusModule = FDevMenusModule::Get())
	{
		if (DevMenusModule->IsPrimaryAssetTypeReady())
		{
			OnAssetRegistryReady();
		}
		else
		{
			DevMenusModule->OnPrimaryAssetTypeReady.AddUObject(this, &ThisClass::OnAssetRegistryReady);
		}
	}
}

void UDevMenus::LoadMenuSettings(const UDevMenuSettings& InSettings) const
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Loading Dev Menu settings"));

	UDevMenu* DevMenu = CreateMenu(EDevMenuPaths::MainMenu);
	DevMenu->MenuType = EDevMenuType::MenuBar;
	DevMenu->Entries = InSettings.MainMenuEntries;
	AddMenu(DevMenu);
}

void UDevMenus::OnAssetRegistryReady() const
{
	if (FDevMenusModule* DevMenusModule = FDevMenusModule::Get())
	{
		DevMenusModule->OnPrimaryAssetTypeReady.RemoveAll(this);
	}

	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Loading Dev Menu assets"));

	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();

	FARFilter MenuAssetsFilter;
#if UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
	MenuAssetsFilter.ClassPaths.Emplace(UDevMenu::StaticClass()->GetClassPathName());
#else
	MenuAssetsFilter.ClassNames.Add(UDevMenu::StaticClass()->GetFName());
#endif // UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
	MenuAssetsFilter.bRecursiveClasses = true;

	TArray<FSoftObjectPath> MenuAssetsList;
	AssetRegistry->EnumerateAssets(MenuAssetsFilter, [&MenuAssetsList](const FAssetData& AssetData)
	{
#if UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
		UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Dev Menu asset found: %s"), *AssetData.GetObjectPathString());
#else
		UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Dev Menu asset found: %s"), *AssetData.AssetClass.ToString());
#endif // UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
		MenuAssetsList.Emplace(AssetData.ToSoftObjectPath());
		return true;
	});

	if (MenuAssetsList.Num())
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(MenuAssetsList, FStreamableDelegate::CreateUObject(this, &ThisClass::OnMenuAssetsLoaded, MenuAssetsList));
	}
	else
	{
		UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("No Dev Menu assets found"));
	}
}

void UDevMenus::OnMenuAssetsLoaded(TArray<FSoftObjectPath> InMenuAssetPaths) const
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Dev Menu assets loaded"));

	TArray<UDevMenu*> MenusList;
	MenusList.Reserve(InMenuAssetPaths.Num());

	for (FSoftObjectPath& MenuAssetPath : InMenuAssetPaths)
	{
		if (UDevMenu* Menu = Cast<UDevMenu>(MenuAssetPath.ResolveObject()))
		{
			MenusList.Add(Menu);
		}
		else
		{
			UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Dev Menu asset resolving failed, or asset has an unsupported type: %s"), *MenuAssetPath.GetAssetPathString());
		}
	}

	Algo::Sort(MenusList,[](const UDevMenu* A, const UDevMenu* B)
	{
		const FName EntryPathA = A->GetEntryName();
		const FName EntryPathB = B->GetEntryName();

		const int32 PathDepthA = FDevMenuPaths::GetPathDepth(EntryPathA);
		const int32 PathDepthB = FDevMenuPaths::GetPathDepth(EntryPathB);

		return (PathDepthA != PathDepthB)
			? (PathDepthA < PathDepthB)
			: (EntryPathA.LexicalLess(EntryPathB));
	});

	for (UDevMenu* Menu : MenusList)
	{
		UE_LOG_FUNCTION(LogDevMenus, VeryVerbose, TEXT("Dev Menu asset added to menu: %s → %s"), *Menu->GetPackage()->GetLoadedPath().GetPackageName(), *Menu->GetEntryName().ToString());
		AddMenu(Menu);
	}
}
