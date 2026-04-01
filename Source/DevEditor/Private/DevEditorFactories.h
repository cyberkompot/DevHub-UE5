// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/Factory.h"
#include "DevEditorFactories.generated.h"

namespace DevMenu::Editor
{
	static const FLazyName MenuCategoryName = "Debug";
}

using namespace DevMenu::Editor;

UCLASS(MinimalAPI, HideCategories = Object)
class UDevMenu_Factory final : public UFactory
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
	virtual FText GetDisplayName() const override { return INVTEXT("Dev Menu"); }
	virtual uint32 GetMenuCategories() const override { return FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().FindAdvancedAssetCategory(MenuCategoryName); }
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override { return NewObject<UDevMenu>(InParent, InName, Flags); }
	//~ End UFactory Interface
};
