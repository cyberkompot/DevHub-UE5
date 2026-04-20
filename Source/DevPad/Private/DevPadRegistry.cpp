// Copyright (c) Alexandr Pereverzev.

#include "DevPadRegistry.h"

#include "DevPadLogging.h"
#include "DevPadSettings.h"

UDevPadPage* FDevPadRegistryEntry::LoadSynchronous() const
{
	CachedPtr = (!SoftPtr.IsNull()) ? SoftPtr.LoadSynchronous() : nullptr;
	return CachedPtr;
}

void FDevPadRegistryEntry::Reset()
{
	SoftPtr = nullptr;
	CachedPtr = nullptr;
}

FDevPadRegistryEntry& FDevPadRegistryEntry::operator =(const TObjectPtr<UDevPadPage>& InPtr)
{
	SoftPtr = InPtr;
	CachedPtr = InPtr;
	return *this;
}

FDevPadRegistryEntry& FDevPadRegistryEntry::operator =(const TSoftObjectPtr<UDevPadPage>& InSoftPtr)
{
	SoftPtr = InSoftPtr;
	CachedPtr = InSoftPtr.Get();
	return *this;
}

void UDevPadRegistry::Initialize()
{
	if (const UDevPadSettings* Settings = GetDefault<UDevPadSettings>())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad common page asset added to pad: %s"), *SoftObjectPtrToPackageLog(Settings->CommonPage));
		CommonPage = Settings->CommonPage;

		RegisteredPages.Reserve(Settings->MainPages.Num());
		for (const TSoftObjectPtr<UDevPadPage>& Page : Settings->MainPages)
		{
			if (!Page.IsNull())
			{
				UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad page asset added to pad: %s"), *SoftObjectPtrToPackageLog(Page));
				RegisteredPages.Emplace(Page);
			}
		}
	}
}

void UDevPadRegistry::Reset()
{
	CommonPage.Reset();
	RegisteredPages.Reset();
}

UDevPadPage* UDevPadRegistry::GetCommonPage() const
{
	return CommonPage.LoadSynchronous();
}

UDevPadPage* UDevPadRegistry::GetFirstPage() const
{
	return (RegisteredPages.IsValidIndex(0)) ? RegisteredPages[0].LoadSynchronous() : nullptr;
}

UDevPadPage* UDevPadRegistry::GetLastPage() const
{
	return (!RegisteredPages.IsEmpty()) ? RegisteredPages.Last().LoadSynchronous() : nullptr;
}

UDevPadPage* UDevPadRegistry::GetPreviousPage(const UDevPadPage* InCurrentPage, const bool InLoop) const
{
	const int32 Index = GetPageIndex(InCurrentPage);
	if (Index == INDEX_NONE) { return nullptr; }

	if (RegisteredPages.IsValidIndex(Index - 1))
	{
		return RegisteredPages[Index - 1].LoadSynchronous();
	}
	if (InLoop)
	{
		return GetLastPage();
	}

	return nullptr;
}

UDevPadPage* UDevPadRegistry::GetNextPage(const UDevPadPage* InCurrentPage, const bool InLoop) const
{
	const int32 Index = GetPageIndex(InCurrentPage);
	if (Index == INDEX_NONE) { return nullptr; }

	if (RegisteredPages.IsValidIndex(Index + 1))
	{
		return RegisteredPages[Index + 1].LoadSynchronous();
	}
	if (InLoop)
	{
		return GetFirstPage();
	}

	return nullptr;
}

int32 UDevPadRegistry::GetPageIndex(const UDevPadPage* InCurrentPage) const
{
	for (int32 i = 0, Num = RegisteredPages.Num(); i < Num; ++i)
	{
		if (RegisteredPages[i] == InCurrentPage) { return i; }
	}
	return INDEX_NONE;
}
