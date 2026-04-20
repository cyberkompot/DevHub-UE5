// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "AssetTypeActions_Base.h"
#include "DevMenuTypes.h"
#include "DevPadTypes.h"

class FDevEditorAssetTypeActionsBase : public FAssetTypeActions_Base
{
public:
	//~ Begin IAssetTypeActions Interface
	virtual FColor GetTypeColor() const override { return FColor(255, 156, 0); };
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	//~ End IAssetTypeActions Interface
};

class FAssetTypeActions_DevMenu final : public FDevEditorAssetTypeActionsBase
{
public:
	inline static const FText Name = INVTEXT("DevHub Menu");

	//~ Begin IAssetTypeActions Interface
	virtual FText GetName() const override { return Name; }
	virtual UClass* GetSupportedClass() const override { return UDevMenu::StaticClass(); }
	//~ End IAssetTypeActions Interface
};

class FAssetTypeActions_DevPadPage final : public FDevEditorAssetTypeActionsBase
{
public:
	inline static const FText Name = INVTEXT("DevHub Pad Page");

	//~ Begin IAssetTypeActions Interface
	virtual FText GetName() const override { return Name; }
	virtual UClass* GetSupportedClass() const override { return UDevPadPage::StaticClass(); }
	//~ End IAssetTypeActions Interface
};
