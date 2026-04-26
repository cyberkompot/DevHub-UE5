// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevPadTypes.h"
#include "Components/TextBlock.h"
#include "DevPadHeaderButtonWidget.generated.h"

USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Pad")
struct FDevPadHeaderButtonData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	bool IsVisible = true;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadHeaderButtonWidget : public UDevPadWidget
{
	GENERATED_BODY()

public:
	static const FText DefaultTitle;

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FText GetLabel() const { return (LabelText) ? LabelText->GetText() : FText(); }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetLabel(const FText& Text) const { if (LabelText) { LabelText->SetText(Text); } }

	void SetLabel(FText&& Text) const { if (LabelText) { LabelText->SetText(MoveTempIfPossible(Text)); } }

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FDevPadHeaderButtonData GetData() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetData(const FDevPadHeaderButtonData& InData);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;
};
