// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenus.h"
#include "DevMenuTypes.h"
#include "DevMenuUtils.h"
#include "Templates/SharedPointer.h"
#include "DevMenuWidgetBuilder.generated.h"

class SWidget;
class UDevMenu;
class UDevMenuGenerator;

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevMenuWidgetBuilder final : public UObject
{
	GENERATED_BODY()

public:
	TSharedRef<SWidget> MakeWidget(const FName InMenuPath);
	TSharedRef<SWidget> MakeWidget(UDevMenu& InGeneratedMenu);

	void Dispose();

	FORCEINLINE void SetMenuGenerator(UDevMenuGenerator& InMenuGenerator) { MenuGenerator = &InMenuGenerator; }

	//~ Begin UObject Interface.
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	//~ End UObject Interface.

private:
	friend struct FDevMenuWidgetBuilderContext;

	UPROPERTY(Transient)
	UDevMenuGenerator* MenuGenerator;

	TArray<TWeakPtr<FMultiBox>> WidgetObjectReferences;

	TSharedRef<SWidget> MakeMainMenuWidget(UDevMenu& InGeneratedMenu);
	TSharedRef<SWidget> MakeSubMenuWidget(UDevMenu& InGeneratedMenu);

	void PopulateMainMenu(FMenuBarBuilder& InMenuBuilder, const FName InMenuPath);
	void PopulateMainMenu(FMenuBarBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu);

	void PopulateSubMenu(FMenuBuilder& InMenuBuilder, const FName InMenuPath);
	void PopulateSubMenu(FMenuBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu);

	void AddEmptyEntryIfNeeded(FMenuBarBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu) const;
	void AddEmptyEntryIfNeeded(FMenuBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu) const;

	TSharedRef<SWidget> OnModifyBlockWidgetAfterMake(const TSharedRef<SMultiBoxWidget>& InMultiBoxWidget, const FMultiBlock& InMultiBlock, const TSharedRef<SWidget>& InWidget, TWeakObjectPtr<UDevMenu> InGeneratedMenu);
};

struct FDevMenuWidgetBuilderContext final : IDevMenuBuilderContext
{
	FDevMenuWidgetBuilderContext(UDevMenuWidgetBuilder& InWidgetBuilder, FMenuBuilder& InMenuBuilder, UDevMenu& InGeneratedMenu)
		: Super(InMenuBuilder, InGeneratedMenu, &InGeneratedMenu), WidgetBuilder(InWidgetBuilder) {}

	virtual FNewMenuDelegate CreatePopulateNewMenuDelegate(IDevMenuEntry& InEntry) const override
	{
		const FName MenuEntryPath = FDevMenuPaths::Combine(GetGeneratedMenu().GetEntryPath(), InEntry.GetEntryPath());
		return FNewMenuDelegate::CreateUObject(&WidgetBuilder, &UDevMenuWidgetBuilder::PopulateSubMenu, MenuEntryPath);
	};

private:
	using Super = IDevMenuBuilderContext;

	UDevMenuWidgetBuilder& WidgetBuilder;
};
