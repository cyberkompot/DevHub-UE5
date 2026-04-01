// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "AssetTypeActions_Base.h"
#include "DevMenuTypes.h"

class FAssetTypeActions_DevMenu final : public FAssetTypeActions_Base
{
public:
	//~ Begin IAssetTypeActions Interface
	virtual FText GetName() const override { return INVTEXT("Dev Menu"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 156, 0); };
	virtual UClass* GetSupportedClass() const override { return UDevMenu::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	//~ End IAssetTypeActions Interface
};
