// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevPadTypes.h"
#include "DevPadHeaderButtonWidget.h"
#include "Components/TextBlock.h"
#include "DevPadHeaderWidget.generated.h"

USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Pad")
struct FDevPadHeaderData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	bool IsVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadHeaderButtonData PreviousPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadHeaderButtonData NextPage;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadHeaderWidget : public UDevPadWidget
{
	GENERATED_BODY()

public:
	static const FText DefaultTitle;

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FText GetTitle() const { return (TitleText) ? TitleText->GetText() : FText(); }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetTitle(const FText& Text) const { if (TitleText) { TitleText->SetText(Text); } }

	void SetTitle(FText&& Text) const { if (TitleText) { TitleText->SetText(MoveTempIfPossible(Text)); } }

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FDevPadHeaderData GetData() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetData(const FDevPadHeaderData& InData);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadHeaderButtonWidget> PreviousPageButton;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadHeaderButtonWidget> NextPageButton;
};
