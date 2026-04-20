// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.h"
#include "DevPadTypes.h"
#include "DevPadRegistry.generated.h"

class UDevPadSettings;

USTRUCT(NotBlueprintType, NotBlueprintable)
struct FDevPadRegistryEntry
{
	GENERATED_BODY()

	FDevPadRegistryEntry() = default;
	FDevPadRegistryEntry(const TObjectPtr<UDevPadPage>& InPtr)
		: SoftPtr(InPtr), CachedPtr(InPtr) {}
	FDevPadRegistryEntry(const TSoftObjectPtr<UDevPadPage>& InSoftPtr)
		: SoftPtr(InSoftPtr) {}

	FORCEINLINE TSoftObjectPtr<UDevPadPage> GetSoftPtr() const { return SoftPtr; }
	FORCEINLINE bool IsNull() const { return SoftPtr.IsNull(); }

	UDevPadPage* LoadSynchronous() const;

	void Reset();

	FDevPadRegistryEntry& operator =(const TObjectPtr<UDevPadPage>& InPtr);
	FDevPadRegistryEntry& operator =(const TSoftObjectPtr<UDevPadPage>& InSoftPtr);
	bool operator ==(const UDevPadPage* InPtr) const { return SoftPtr == InPtr; }
	bool operator ==(const TObjectPtr<UDevPadPage>& InPtr) const { return SoftPtr == InPtr; }
	bool operator ==(const TSoftObjectPtr<UDevPadPage>& InSoftPtr) const { return SoftPtr == InSoftPtr; }

private:
	UPROPERTY(VisibleAnywhere, Category = "DevPad")
	TSoftObjectPtr<UDevPadPage> SoftPtr = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "DevPad")
	mutable TObjectPtr<UDevPadPage> CachedPtr = nullptr;
};

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevPadRegistry final : public UDevCoreResettable
{
	GENERATED_BODY()

public:
	void Initialize();
	virtual void Reset() override;

public:
	UDevPadPage* GetCommonPage() const;

	UDevPadPage* GetFirstPage() const;
	UDevPadPage* GetLastPage() const;

	UDevPadPage* GetPreviousPage(const UDevPadPage* InCurrentPage, bool InLoop) const;
	UDevPadPage* GetNextPage(const UDevPadPage* InCurrentPage, bool InLoop) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "DevPad")
	FDevPadRegistryEntry CommonPage;

	UPROPERTY(VisibleAnywhere, Category = "DevPad")
	TArray<FDevPadRegistryEntry> RegisteredPages;

	int32 GetPageIndex(const UDevPadPage* InCurrentPage) const;
};
