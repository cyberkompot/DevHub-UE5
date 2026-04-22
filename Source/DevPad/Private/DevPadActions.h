// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "DevPadActions.generated.h"

class UDevPad;
class UDevPadPage;

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", meta = (Hidden))
struct FDevPadActionBase : public FDevActionBase
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const {};
	//~ End FDevAction interface.
};

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "[DevPad] Open sub-page")
struct FDevPadActionOpenSubPage : public FDevPadActionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevAction", meta = (DisplayAfter = "Label"))
	TSoftObjectPtr<UDevPadPage> Page;

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const override;
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return !Page.IsNull();  }
	//~ End FDevAction interface.
};

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "[DevPad] Open sub-page")
struct FDevPadActionCloseSubPage : public FDevPadActionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevAction", meta = (DisplayAfter = "Label"))
	TSoftObjectPtr<UDevPadPage> Page;

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const override;
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return !Page.IsNull();  }
	//~ End FDevAction interface.
};
