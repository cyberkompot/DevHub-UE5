#pragma once

#include "DevPadTypes.h"
#include "Components/TextBlock.h"
#include "DevPadActionWidget.generated.h"

USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Pad")
struct FDevPadActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	bool IsVisible = true;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadActionWidget : public UDevPadWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FText GetLabel() const { return (LabelText) ? LabelText->GetText() : FText(); }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetLabel(const FText& Text) const { if (LabelText) { LabelText->SetText(Text); } }

	void SetLabel(FText&& Text) const { if (LabelText) { LabelText->SetText(MoveTempIfPossible(Text)); } }

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FDevPadActionData GetData() const
	{
		FDevPadActionData Data;
		Data.IsVisible = IsVisible();
		Data.Label = GetLabel();
		return Data;
	}

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetData(const FDevPadActionData& InData)
	{
		SetVisibility((InData.IsVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		SetLabel(InData.Label);
	}

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UTextBlock> LabelText;
};
