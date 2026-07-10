// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadLayoutWidget.h"

#include "DevPadTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScaleBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	FSlateChildSize MakeSize(const float InValue)
	{
		FSlateChildSize ChildSize(ESlateSizeRule::Fill);
		ChildSize.Value = InValue;
		return ChildSize;
	};
}

void UDevPadLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), "Canvas");
	WidgetTree->RootWidget = Canvas;

	VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), "VBox");
	HBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), "HBox");
	ScaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), "ScaleBox");
	HSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), "HSpacer");
	VSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), "VSpacer");

	ScaleBox->SetStretch(EStretch::ScaleToFitX);

	VBoxSlot = Canvas->AddChildToCanvas(VBox);
	VBoxSlot->SetAutoSize(false);
	VBoxSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	VBoxSlot->SetOffsets(FMargin(10.f));

	ApplyAlignment();
}

void UDevPadLayoutWidget::SetAlignment(const EDevPadAlignment InAlignment)
{
	if (Alignment == InAlignment) { return; }
	Alignment = InAlignment;
	ApplyAlignment();
}

void UDevPadLayoutWidget::SetContent(UWidget* InWidget) const
{
	if (ScaleBox->GetContent() == InWidget) { return; }
	ScaleBox->SetContent(InWidget);
}

void UDevPadLayoutWidget::SetScale(const float InScale)
{
	if (FMath::IsNearlyEqual(Scale, InScale)) { return; }
	Scale = FMath::Clamp(InScale, 1.f, 2.f);
	ApplyScale();
}

float UDevPadLayoutWidget::GetContentWidthFraction() const
{
	return FMath::Lerp(0.25f, 0.5f, Scale - 1.f);
}

void UDevPadLayoutWidget::ApplyAlignment() const
{
	const float Size = GetContentWidthFraction();

	const bool bIsRight = (Alignment == EDevPadAlignment::TopRight || Alignment == EDevPadAlignment::BottomRight);
	const bool bIsBottom = (Alignment == EDevPadAlignment::BottomLeft || Alignment == EDevPadAlignment::BottomRight);

	HBox->ClearChildren();
	if (bIsRight)
	{
		HBox->AddChildToHorizontalBox(HSpacer)->SetSize(MakeSize(1.f - Size));
		HBox->AddChildToHorizontalBox(ScaleBox)->SetSize(MakeSize(Size));
	}
	else
	{
		HBox->AddChildToHorizontalBox(ScaleBox)->SetSize(MakeSize(Size));
		HBox->AddChildToHorizontalBox(HSpacer)->SetSize(MakeSize(1.f - Size));
	}

	VBox->ClearChildren();
	if (bIsBottom)
	{
		VBox->AddChildToVerticalBox(VSpacer)->SetSize(MakeSize(1.f));
		VBox->AddChildToVerticalBox(HBox)->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
	else
	{
		VBox->AddChildToVerticalBox(HBox)->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		VBox->AddChildToVerticalBox(VSpacer)->SetSize(MakeSize(1.f));
	}
}

void UDevPadLayoutWidget::ApplyScale() const
{
	const float Size = GetContentWidthFraction();

	if (UHorizontalBoxSlot* ScaleBoxSlot = Cast<UHorizontalBoxSlot>(ScaleBox->Slot))
	{
		ScaleBoxSlot->SetSize(MakeSize(Size));
	}
	if (UHorizontalBoxSlot* HSpacerSlot = Cast<UHorizontalBoxSlot>(HSpacer->Slot))
	{
		HSpacerSlot->SetSize(MakeSize(1.f - Size));
	}
}
