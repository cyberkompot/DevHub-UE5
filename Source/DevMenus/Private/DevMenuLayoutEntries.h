// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "DevMenuUtils.h"
#include "DevMenuLayoutEntries.generated.h"

USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct FDevMenuLayoutBeginSection : public FDevMenuItemBase
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | EDevMenuEntryFlags::Section | EDevMenuEntryFlags::ProxyEntry; };
	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override { InContext.GetMenuBuilder().BeginSection(GetEntryPath(), GetLabel()); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuItemBase;
};

USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct FDevMenuLayoutEndSection : public FDevMenuItemBase
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | EDevMenuEntryFlags::Section | EDevMenuEntryFlags::ProxyEntry; };
	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override { InContext.GetMenuBuilder().EndSection(); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuItemBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct FDevMenuLayoutSubMenu : public FDevMenuItemBase
{
	GENERATED_BODY()

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | EDevMenuEntryFlags::SubMenu | EDevMenuEntryFlags::ProxyEntry; };
	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override { InContext.GetMenuBuilder().AddSubMenu(GetLabel(), GetToolTip(), InContext.CreatePopulateNewMenuDelegate(*this), false, FSlateIcon(), false,GetEntryPath()); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuItemBase;
};


/** Dev Menu layout entries factory. */
struct FDevMenuLayoutFactory final
{
	constexpr static EDevMenuEntryFlags LayoutFlags = EDevMenuEntryFlags::SubMenu | EDevMenuEntryFlags::Section | EDevMenuEntryFlags::Embedded;

	FORCEINLINE static bool IsLayoutEntry(const IDevMenuEntry& InEntry) { return InEntry.HasAnyEntryFlags(LayoutFlags); }

	struct FLayoutContext final
	{
		explicit FLayoutContext(UDevMenu& InMenu)
			: FLayoutContext(InMenu, InMenu) {}
		explicit FLayoutContext(UDevMenu& InMenu, IDevMenuEntry& InOuter, const FName InEmbeddedEntryPath = NAME_None)
			: Menu(InMenu), Outer(InOuter), EmbeddedPath(InEmbeddedEntryPath) {}

		UDevMenu& Menu;
		IDevMenuEntry& Outer;
		FName EmbeddedPath;
	};

	template<typename TEntryType,
			 typename = std::enable_if_t<std::is_base_of_v<FDevMenuItemBase, std::decay_t<TEntryType>>>>
	static TEntryType CreateLayoutEntry(IDevMenuEntry& InSourceEntry, const FLayoutContext& InLayoutContext)
	{
		TEntryType Entry{};
		Entry.EntryName = FDevMenuPaths::Combine(InLayoutContext.EmbeddedPath, InSourceEntry.GetEntryPath());
		Entry.Label = InSourceEntry.GetLabel().Get();
		Entry.ToolTip = InSourceEntry.GetToolTip().Get();
		// TODO: Set duplicate flag.
		return Entry;
	}

	static FDevMenuInstancedEntry DuplicateLayoutEntry(IDevMenuEntry& InSourceEntry, const FLayoutContext& InLayoutContext)
	{
		FDevMenuInstancedEntry InstancedEntry = InSourceEntry.DuplicateEntry();
		FDevMenuEntry& Entry = InstancedEntry.GetMutable<FDevMenuEntry>();
		Entry.SetEntryName(FDevMenuPaths::Combine(InLayoutContext.EmbeddedPath, InSourceEntry.GetEntryPath()));
		// TODO: Set duplicate flag.
		return InstancedEntry;
	}

	static FDevMenuInstancedEntry CreateInstancedLayoutBeginEntryIfNeeded(IDevMenuEntry& InSourceEntry, const FLayoutContext& InLayoutContext)
	{
		switch (InSourceEntry.GetEntryFlags() & LayoutFlags)
		{
			case EDevMenuEntryFlags::Section:
				return (InLayoutContext.Outer.HasAnyEntryFlags(EDevMenuEntryFlags::Section)) // Nested section layouts converted to sub-menus.
					? FDevMenuInstancedEntry::Make(CreateLayoutEntry<FDevMenuLayoutSubMenu>(InSourceEntry, InLayoutContext))
					: FDevMenuInstancedEntry::Make(CreateLayoutEntry<FDevMenuLayoutBeginSection>(InSourceEntry, InLayoutContext));
			case EDevMenuEntryFlags::SubMenu:
				return FDevMenuInstancedEntry::Make(CreateLayoutEntry<FDevMenuLayoutSubMenu>(InSourceEntry, InLayoutContext));
			case EDevMenuEntryFlags::Embedded:
			default:
				return FDevMenuInstancedEntry(); // Embedded layouts converts to nothing.
		}
	}

	static FDevMenuInstancedEntry CreateInstancedLayoutEndEntryIfNeeded(IDevMenuEntry& InSourceEntry, const FLayoutContext& InLayoutContext)
	{
		// Only non-nested section layouts requires ending entry.
		if (InSourceEntry.HasAnyEntryFlags(EDevMenuEntryFlags::Section)
			&& !InLayoutContext.Outer.HasAnyEntryFlags(EDevMenuEntryFlags::Section))
		{
			return FDevMenuInstancedEntry::Make(CreateLayoutEntry<FDevMenuLayoutEndSection>(InSourceEntry, InLayoutContext));
		}
		return FDevMenuInstancedEntry();
	}
};
