// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "DevMenus.generated.h"

class UDevInputs;
class UDevMenu;
class UDevMenuGenerator;
class UDevMenuManager;
class UDevMenuRegistry;
class UDevMenuSettings;
class UDevMenuWidgetBuilder;
struct FDevInputShortcut;

UCLASS(NotBlueprintable, Category = "DevHub|Menu", DisplayName = "Dev Menus")
class DEVMENUS_API UDevMenus final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UDevMenus* Get(const UObject* WorldContextObject);

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	UDevMenu* RegisterMenu(const FName InMenuPath) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	UDevMenu* UnregisterMenu(const FName InMenuPath) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	UDevMenu* CreateMenu(const FName InMenuPath) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void AddMenu(UDevMenu* InMenu) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void RemoveMenu(UDevMenu* InMenu) const;

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void ShowMenu(const FName InMenuPath = "MainMenu");

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void ToggleMenu(const FName InMenuPath = "MainMenu");

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void HideMenu(const FName InMenuPath = "MainMenu");

	UFUNCTION(BlueprintCallable, Category = "DevHub|Menu")
	void HideAllMenus();

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface
	
private:
	UDevMenus();

	UPROPERTY(Transient)
	UDevMenuGenerator* MenuGenerator;

	UPROPERTY(Transient)
	UDevMenuRegistry* MenuRegistry;

	UPROPERTY(Transient)
	UDevMenuManager* MenuManager;

	UPROPERTY(Transient)
	UDevMenuWidgetBuilder* MenuWidgetBuilder;

	void BindShortcuts(const UDevMenuSettings& InSettings);
	void UnbindShortcuts() const;

	void BindMenuShortcut(UDevInputs& InDevInputs, const FName InMenuPath, const FDevInputShortcut& InInputShortcut);

	void LoadMenuAssets() const;
	void LoadMenuSettings(const UDevMenuSettings& InSettings) const;

	void OnAssetRegistryReady() const;
	void OnMenuAssetsLoaded(TArray<FSoftObjectPath> InMenuAssetPaths) const;
};
