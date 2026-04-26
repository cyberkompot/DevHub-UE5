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

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", meta = (Hidden))
struct FDevPadActionSubPageBase : public FDevPadActionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevAction", meta = (DisplayAfter = "Label", DisplayThumbnail = false))
	TSoftObjectPtr<UDevPadPage> Page;

protected:
	//~ Begin FDevAction interface.
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return !Page.IsNull();  }
	//~ End FDevAction interface.
};

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "[DevPad] Open sub-page")
struct FDevPadActionOpenSubPage : public FDevPadActionSubPageBase
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const override;
	//~ End FDevAction interface.
};

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "[DevPad] Toggle sub-page")
struct FDevPadActionToggleSubPage : public FDevPadActionSubPageBase
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const override;
	//~ End FDevAction interface.
};

USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "[DevPad] Close sub-page")
struct FDevPadActionCloseSubPage : public FDevPadActionSubPageBase
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual void OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const override;
	//~ End FDevAction interface.
};
