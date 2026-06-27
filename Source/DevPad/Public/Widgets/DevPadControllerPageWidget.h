// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevPadActionWidget.h"
#include "DevPadTypes.h"
#include "DevPadControllerPageWidget.generated.h"

UCLASS(BlueprintType, Blueprintable, Category = "DevHub|Pad")
class UDevPadControllerPageData : public UDevPadPageData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData DPadUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData DPadLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData DPadRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData DPadDownAction;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData FaceUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData FaceLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData FaceRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData FaceDownAction;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData LeftTriggerAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData RightTriggerAction;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData LeftThumbstickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadActionData RightThumbstickAction;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadControllerPageWidget : public UDevPadPageWidget
{
	GENERATED_BODY()

public:
	//~ Begin UDevPadContentWidget Interface
	virtual TSubclassOf<UDevPadContentData> GetExpectedDataClass() const override { return UDevPadControllerPageData::StaticClass(); }
	virtual void UpdateWidget() override;
	//~ End UDevPadContentWidget Interface

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> DPadUpAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> DPadLeftAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> DPadRightAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> DPadDownAction;


	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> FaceUpAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> FaceLeftAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> FaceRightAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> FaceDownAction;


	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> LeftTriggerAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> RightTriggerAction;


	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> LeftThumbstickAction;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadActionWidget> RightThumbstickAction;
};
