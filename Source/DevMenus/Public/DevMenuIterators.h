// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"

struct DEVMENUS_API FDevMenuIteratorOverMenus final : IDevMenuEntriesIterator
{
	explicit FDevMenuIteratorOverMenus(FDevMenuMenus& InMenus)
		: Menus(InMenus) {}

	//~ Begin IDevMenuEntriesIterator interface.
	virtual void operator ++() override { ++Index; }
	virtual explicit operator bool() const override { return Menus.IsValidIndex(Index); }
	virtual IDevMenuEntry* operator *() override { return (*this) ? Menus.GetData()[Index].Get() : nullptr; }
	//~ End IDevMenuEntriesIterator interface.

private:
	FDevMenuMenus& Menus;
	int32 Index = 0;
};


struct DEVMENUS_API FDevMenuIteratorOverEntries final : IDevMenuEntriesIterator
{
	explicit FDevMenuIteratorOverEntries(FDevMenuInstancedEntries& InEntries)
		: Entries(InEntries) {}

	//~ Begin IDevMenuEntriesIterator interface.
	virtual void operator ++() override { ++Index; }
	virtual explicit operator bool() const override { return Entries.IsValidIndex(Index); }
	virtual IDevMenuEntry* operator *() override { return (*this) ? Entries.GetData()[Index].GetMutablePtr<FDevMenuEntry>() : nullptr; }
	//~ End IDevMenuEntriesIterator interface.

private:
	FDevMenuInstancedEntries& Entries;
	int32 Index = 0;
};


struct DEVMENUS_API FDevMenuIteratorOverDynamicEntries final : IDevMenuEntriesIterator
{
	explicit FDevMenuIteratorOverDynamicEntries(const FDevMenuInstancedEntries& InEntries)
		: Entries(InEntries) {}
	explicit FDevMenuIteratorOverDynamicEntries(FDevMenuInstancedEntries&& InEntries)
		: Entries(MoveTempIfPossible(InEntries)) {}

	//~ Begin IDevMenuEntriesIterator interface.
	virtual void operator ++() override { ++Index; }
	virtual explicit operator bool() const override { return Entries.IsValidIndex(Index); }
	virtual IDevMenuEntry* operator *() override { return (*this) ? Entries.GetData()[Index].GetMutablePtr<FDevMenuEntry>() : nullptr; }
	//~ End IDevMenuEntriesIterator interface.

private:
	FDevMenuInstancedEntries Entries;
	int32 Index = 0;
};
