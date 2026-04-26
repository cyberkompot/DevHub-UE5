// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevPadHeaderWidget.h"
#include "DevPadTypes.h"
#include "Components/ContentWidget.h"
#include "DevPadPanelWidget.generated.h"

UCLASS(BlueprintType, Blueprintable, Category = "DevHub|Pad")
class UDevPadPanelData : public UDevPadContentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	FDevPadHeaderData HeaderData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	TObjectPtr<UDevPadInfoWidget> InfoWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevHub")
	TObjectPtr<UDevPadPageWidget> PageWidget;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadPanelWidget : public UDevPadContentWidget
{
	GENERATED_BODY()

public:
	//~ Begin UDevPadContentWidget Interface
	virtual TSubclassOf<UDevPadContentData> GetExpectedDataClass() const override { return UDevPadPanelData::StaticClass(); }
	virtual void UpdateWidget() override;
	//~ End UDevPadContentWidget Interface

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UDevPadHeaderWidget> PanelHeader;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UContentWidget> PanelInfoContent;

	UPROPERTY(BlueprintReadOnly, Category = "DevHub", meta = (BindWidget))
	TObjectPtr<UContentWidget> PanelPageContent;
};
