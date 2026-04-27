// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreObjectPtr.h"
#include "DevPadTypes.h"
#include "DevPadStackController.generated.h"

class UDevPadPage;

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevPadStackController final : public UDevPadStack
{
	GENERATED_BODY()

public:
	void Reset();

public:
	//~ Begin IDevPadStack interface.
	virtual const TArray<UDevPadPage*>& GetAllPages() const override { return DevCore::Utils::ObjectPtr::ObjectPtrDecay(Pages); }
	virtual UDevPadPage* GetCommonPage() const override { return CommonPage; }
	virtual UDevPadPage* GetMainPage() const override { return (Pages.IsValidIndex(GetMainPageIndex()) ? Pages[GetMainPageIndex()] : nullptr); }
	virtual UDevPadPage* GetTopPage() const override { return Pages.IsValidIndex(GetMainPageIndex()) ? Pages.Top() : nullptr; }
	virtual bool IsStackEmpty() const override { return (Pages.Num() <= GetMainPageIndex()); }
	virtual bool IsCommonPage(const UDevPadPage* InPage) const override { return (CommonPage && CommonPage == InPage); }
	virtual bool IsMainPage(const UDevPadPage* InPage) const override { return (Pages.IndexOfByKey(InPage) == GetMainPageIndex()); }
	virtual bool IsSubPage(const UDevPadPage* InPage) const override { return (Pages.IndexOfByKey(InPage) > GetMainPageIndex()); }
	//~ End IDevPadStack interface.

public:
	void PushToStack(UDevPadPage* InPage);
	void PopFromStack();
	void ResetStack();

public:
	void SetCommonPage(UDevPadPage* InPage);
	void ResetCommonPage();

private:
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<UDevPadPage> CommonPage = nullptr;

	UPROPERTY(Transient, SkipSerialization)
	TArray<TObjectPtr<UDevPadPage>> Pages;

	FORCEINLINE int32 GetMainPageIndex() const { return (CommonPage) ? 1 : 0; }
};
