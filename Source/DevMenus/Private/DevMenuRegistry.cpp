// Copyright (c) Alexandr Pereverzev.

#include "DevMenuRegistry.h"

#include "DevMenuFactory.h"
#include "DevMenuLogging.h"
#include "DevMenuRegistryEntries.h"
#include "DevMenuUtils.h"

UDevMenuRegistry::UDevMenuRegistry()
{
	RootMenu = CreateDefaultSubobject<UDevMenuRegistryMenu>(RootMenuName, true);
}

UDevMenu* UDevMenuRegistry::RegisterMenu(const FName InMenuPath)
{
	if (const TObjectPtr<UDevMenu>* RegisteredMenu = RegisteredMenus.Find(InMenuPath))
	{
		return *RegisteredMenu;
	}

	UDevMenu* NewMenu = UDevMenu::CreateMenu(InMenuPath, this, RegisteredMenuName);
	RegisteredMenus.Add(InMenuPath, NewMenu);
	AddMenu(NewMenu);
	return NewMenu;
}

UDevMenu* UDevMenuRegistry::UnregisterMenu(const FName InMenuPath)
{
	TObjectPtr<UDevMenu> RegisteredMenu;
	if (RegisteredMenus.RemoveAndCopyValue(InMenuPath, RegisteredMenu))
	{
		RemoveMenu(RegisteredMenu);
	}
	return RegisteredMenu;
}

UDevMenu* UDevMenuRegistry::CreateMenu(const FName InMenuPath)
{
	UDevMenu* NewMenu = UDevMenu::CreateMenu(InMenuPath, this, CreatedMenuName);
	return NewMenu;
}

void UDevMenuRegistry::AddMenu(UDevMenu* InMenu) const
{
	if (!InMenu)
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Menu is not defined"));
		return;
	}

	AddRegistryEntry(RootMenu, InMenu, InMenu->GetEntryName().ToString());
}

void UDevMenuRegistry::RemoveMenu(UDevMenu* InMenu)
{
	if (!InMenu)
	{
		UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Menu is not defined"));
		return;
	}

	const FName MenuPath = InMenu->GetEntryName();
	if (const TObjectPtr<UDevMenu>* RegisteredMenuPtr = RegisteredMenus.Find(MenuPath); RegisteredMenuPtr && *RegisteredMenuPtr == InMenu)
	{
		RegisteredMenus.Remove(MenuPath);
	}

	RemoveRegistryEntry(RootMenu, InMenu, MenuPath.ToString());
}

void UDevMenuRegistry::Dispose()
{
	RootMenu->GetSubEntries()->Reset();
	RegisteredMenus.Reset();
}

void UDevMenuRegistry::QueryEntries(const IDevMenuEntriesQuery& InQuery) const
{
	RootMenu->ResolveEntryPath();
	RootMenu->QuerySubEntries(InQuery);
}

IDevMenuEntry* UDevMenuRegistry::AddRegistryEntry(IDevMenuEntry* InOuter, UDevMenu* InMenu, const FString& InPath) const
{
	FString RootDirectoryPath, RemainingPath;
	FDevMenuPaths::ExtractRootDirectory(InPath, RootDirectoryPath, RemainingPath);

	IDevMenuEntry* RegistryEntry;

	if (RootDirectoryPath.IsEmpty())
	{
		const FName RegistryEntryName(RemainingPath);
		FDevMenuRegistryProxyEntry RegistryProxyEntry = FDevMenuFactory::CreateEntry<FDevMenuRegistryProxyEntry>(RegistryEntryName);
		RegistryProxyEntry.Menu = InMenu;
		RegistryEntry = InOuter->GetSubEntries()->Emplace_GetRef(MoveTemp(RegistryProxyEntry)).GetMutablePtr<FDevMenuEntry>();
	}
	else
	{
		IDevMenuEntry* SubOuter = nullptr;

		const FName RegistryEntryName(RootDirectoryPath);
		FDevMenuInstancedEntries* InstancedEntries = InOuter->GetSubEntries();
		for (FDevMenuInstancedEntry& InstancedEntry : *InstancedEntries)
		{
			if (FDevMenuRegistrySubMenuEntry* Entry = IDevMenuEntry::CastTo<FDevMenuRegistrySubMenuEntry>(InstancedEntry))
			{
				if (Entry && Entry->GetEntryName() == RegistryEntryName)
				{
					SubOuter = Entry;
					break;
				}
			}
		}

		if (!SubOuter)
		{
			FDevMenuRegistrySubMenuEntry RegistrySubMenuEntry = FDevMenuFactory::CreateEntry<FDevMenuRegistrySubMenuEntry>(RegistryEntryName);
			SubOuter = InOuter->GetSubEntries()->Emplace_GetRef(MoveTemp(RegistrySubMenuEntry)).GetMutablePtr<FDevMenuEntry>();
		}

		RegistryEntry = AddRegistryEntry(SubOuter, InMenu, RemainingPath);
	}

	return RegistryEntry;
}

void UDevMenuRegistry::RemoveRegistryEntry(IDevMenuEntry* InOuter, UDevMenu* InMenu, const FString& InPath) const
{
	FString RootDirectoryPath, RemainingPath;
	FDevMenuPaths::ExtractRootDirectory(InPath, RootDirectoryPath, RemainingPath);

	if (RootDirectoryPath.IsEmpty())
	{
		FDevMenuInstancedEntries* InstancedEntries = InOuter->GetSubEntries();
		for (int32 i = 0, Num = InstancedEntries->Num(); i < Num; ++i)
		{
			if (const FDevMenuRegistryProxyEntry* Entry = IDevMenuEntry::CastTo<const FDevMenuRegistryProxyEntry>((*InstancedEntries)[i]))
			{
				if (Entry && Entry->Menu == InMenu)
				{
					InstancedEntries->RemoveAt(i);
					return;
				}
			}
		}
	}
	else
	{
		const FName RegistryEntryName(RootDirectoryPath);
		FDevMenuInstancedEntries* InstancedEntries = InOuter->GetSubEntries();
		for (int32 i = 0, Num = InstancedEntries->Num(); i < Num; ++i)
		{
			if (FDevMenuRegistrySubMenuEntry* Entry = IDevMenuEntry::CastTo<FDevMenuRegistrySubMenuEntry>((*InstancedEntries)[i]))
			{
				if (Entry && Entry->GetEntryName() == RegistryEntryName)
				{
					RemoveRegistryEntry(Entry, InMenu, RemainingPath);
					if (Entry->GetSubEntries()->IsEmpty()) // When no sub-entries left, we can remove sub-menu.
					{
						InstancedEntries->RemoveAt(i);
					}
					return;
				}
			}
		}
	}

	const FString MenuPath = InMenu->GetEntryName().ToString();
	const FString MissingMenuPath = FDevMenuPaths::TrimPath(MenuPath.LeftChop(RemainingPath.Len()));
	UE_LOG_FUNCTION(LogDevMenus, Warning, TEXT("Failed to remove menu. Menu not found: Menu = %s, Missing menu = %s"), *MenuPath, *MissingMenuPath);
}
