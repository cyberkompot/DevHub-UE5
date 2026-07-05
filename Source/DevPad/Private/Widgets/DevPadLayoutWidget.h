// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Blueprint/UserWidget.h"
#include "DevPadTypes.h"
#include "DevPadLayoutWidget.generated.h"

class UCanvasPanelSlot;
class UDevPadPanelWidget;
class UHorizontalBox;
class UScaleBox;
class USpacer;
class UVerticalBox;

UCLASS(NotBlueprintType, NotBlueprintable, Category = "DevHub|Pad", meta = (Hidden))
class UDevPadLayoutWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAlignment(const EDevPadAlignment InAlignment);
	void SetContent(UWidget* InWidget) const;
	void SetScale(const float InScale);

protected:
	//~ Begin UUserWidget interface.
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget interface.

private:
	EDevPadAlignment Alignment = EDevPadAlignment::BottomRight;
	float Scale = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> VBox;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HBox;

	UPROPERTY(Transient)
	TObjectPtr<UScaleBox> ScaleBox;

	UPROPERTY(Transient)
	TObjectPtr<USpacer> HSpacer;

	UPROPERTY(Transient)
	TObjectPtr<USpacer> VSpacer;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanelSlot> VBoxSlot;

	float GetContentWidthFraction() const;

	void ApplyAlignment() const;
	void ApplyScale() const;
};
