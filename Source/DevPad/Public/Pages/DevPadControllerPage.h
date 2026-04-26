// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreCompatibility.h"
#include UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH
#include UE_COMPATIBILITY_INCLUDE_STRUCT_VIEW_PATH

#include "DevPadTypes.h"
#include "Widgets/DevPadControllerPageWidget.h"
#include "DevPadControllerPage.generated.h"

class UDevPadControllerPage;

struct FDevPadControllerPageConstActionView
{
	FDevPadControllerPageConstActionView() = default;
	FDevPadControllerPageConstActionView(const UDevPadControllerPage& InPage, const FInstancedStruct& InInstancedStruct)
		: Page(&InPage), StructView(InInstancedStruct) {}

	FORCEINLINE const UDevPadControllerPage* GetPagePtr() const { return Page; }
	FORCEINLINE const UScriptStruct* GetScriptStruct() const { 	return StructView.GetScriptStruct(); }
	FORCEINLINE const FDevAction* GetActionPtr() const { return StructView.GetPtr<const FDevAction>(); }
	FORCEINLINE bool IsValid() const { return (Page && StructView.GetPtr<const FDevAction>()); }

private:
	const UDevPadControllerPage* Page = nullptr;
	FConstStructView StructView;
};

/**
 * DevHub Pad Page allows to associate actions with game controller buttons.
 * @see https://github.com/cyberkompot/DevHub-UE5
 */
UCLASS(BlueprintType, Blueprintable, DisplayName = "Controller Page", Category = "DevHub|Pad", EditInlineNew, CollapseCategories, Meta = (LoadBehavior = "LazyOnDemand"))
class DEVPAD_API UDevPadControllerPage : public UDevPadPageBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "DevPad")
	TSoftClassPtr<UDevPadInfoWidget> InfoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "DevPad")
	TSoftClassPtr<UDevPadControllerPageWidget> PageWidgetClass;


	UPROPERTY(EditDefaultsOnly, DisplayName = "D-Pad Up Action", Category = "D-Pad Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct DPadUpAction;

	UPROPERTY(EditDefaultsOnly, DisplayName = "D-Pad Left Action", Category = "D-Pad Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct DPadLeftAction;

	UPROPERTY(EditDefaultsOnly, DisplayName = "D-Pad Right Action", Category = "D-Pad Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct DPadRightAction;

	UPROPERTY(EditDefaultsOnly, DisplayName = "D-Pad Down Action", Category = "D-Pad Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct DPadDownAction;


	UPROPERTY(EditDefaultsOnly, Category = "Face Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct FaceUpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Face Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct FaceLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Face Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct FaceRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Face Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct FaceDownAction;


	UPROPERTY(EditDefaultsOnly, Category = "Trigger Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct LeftTriggerAction;

	UPROPERTY(EditDefaultsOnly, Category = "Trigger Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct RightTriggerAction;


	UPROPERTY(EditDefaultsOnly, Category = "Thumbstick Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct LeftThumbstickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Thumbstick Buttons", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct RightThumbstickAction;


	UPROPERTY(EditDefaultsOnly, DisplayName = "Left + Right Triggers Action", Category = "Button Shortcuts", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct LeftPlusRightTriggersAction;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Left + Right Thumbsticks Action", Category = "Button Shortcuts", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	FInstancedStruct LeftPlusRightThumbsticksAction;


	//~ Begin IDevPadPage interface.
	virtual TSoftClassPtr<UDevPadInfoWidget> GetInfoWidgetClass(const UObject* WorldContextObject) const override;
	virtual TSoftClassPtr<UDevPadPageWidget> GetPageWidgetClass(const UObject* WorldContextObject) const override;
	virtual EDevPadInputExecution ExecutePageInput(const UObject* WorldContextObject, const FDevPadExecutionContext& PageExecutionContext) const override;
	virtual void PopulatePageWidget(const UObject* WorldContextObject, const FDevPadWidgetContext& PageWidgetContext) const override;
	//~ End IDevPadPage interface.

protected:
	FDevPadControllerPageConstActionView GetPageAction(const UObject* WorldContextObject, const UDevPadStack* InPadStack, const EDevPadInput InPadInput) const;
};
