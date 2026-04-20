// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "AssetToolsModule.h"
#include "DevEditorAssetTypeActions.h"
#include "IAssetTools.h"
#include "Factories/Factory.h"
#include "DevEditorFactories.generated.h"

namespace DevMenu::Editor
{
	static const FLazyName MenuCategoryName = "Debug";
}

using namespace DevMenu::Editor;

UCLASS(Abstract)
class UDevEditorFactoryBase : public UFactory
{
	GENERATED_BODY()

public:
	//~ Begin UFactory Interface
	virtual uint32 GetMenuCategories() const override { return FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().FindAdvancedAssetCategory(MenuCategoryName); }
	//~ End UFactory Interface
};


UCLASS(MinimalAPI, HideCategories = Object)
class UDevMenu_Factory final : public UDevEditorFactoryBase
{
	GENERATED_BODY()

public:
	explicit UDevMenu_Factory(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SupportedClass = UDevMenu::StaticClass();
		bCreateNew = true;
		bEditAfterNew = true;
	}

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override { return NewObject<UDevMenu>(InParent, InName, Flags); }
	virtual FText GetDisplayName() const override { return FAssetTypeActions_DevMenu::Name; }
	//~ End UFactory Interface
};

UCLASS(MinimalAPI, HideCategories = Object)
class UDevPadPage_Factory final : public UDevEditorFactoryBase
{
	GENERATED_BODY()

public:
	explicit UDevPadPage_Factory(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SupportedClass = UDevPadPage::StaticClass();
		bCreateNew = true;
		bEditAfterNew = true;
	}

	UPROPERTY(EditAnywhere, Category = "DebHub|Pad")
	TSubclassOf<UDevPadPage> PadPageClass;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override { return FAssetTypeActions_DevPadPage::Name; }
	//~ End UFactory Interface
};
