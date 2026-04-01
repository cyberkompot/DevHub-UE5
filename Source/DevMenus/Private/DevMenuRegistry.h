// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuRegistry.generated.h"

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevMenuRegistry final : public UObject
{
	GENERATED_BODY()

public:
	UDevMenuRegistry();

	UDevMenu* RegisterMenu(const FName InMenuPath);

	UDevMenu* UnregisterMenu(const FName InMenuPath);

	UDevMenu* CreateMenu(const FName InMenuPath);

	void AddMenu(UDevMenu* InMenu) const;

	void RemoveMenu(UDevMenu* InMenu);

	void Dispose();

	void QueryEntries(const IDevMenuEntriesQuery& InQuery) const;

private:
	constexpr static auto CreatedMenuName = "CreatedMenu";
	constexpr static auto RegisteredMenuName = "RegisteredMenu";
	constexpr static auto RootMenuName = "RootMenu";

	UPROPERTY(Transient)
	TObjectPtr<UDevMenu> RootMenu;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDevMenu>> PrototypeMenus;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UDevMenu>> RegisteredMenus;

	IDevMenuEntry* AddRegistryEntry(IDevMenuEntry* InOuter, UDevMenu* InMenu, const FString& InPath) const;
	void RemoveRegistryEntry(IDevMenuEntry* InOuter, UDevMenu* InMenu, const FString& InPath) const;
};

