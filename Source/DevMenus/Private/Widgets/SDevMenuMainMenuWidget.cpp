// Copyright (c) Alexandr Pereverzev.

#include "SDevMenuMainMenuWidget.h"

#include "DevMenuTypes.h"
#include "Widgets/Layout/SBorder.h"

SLATE_IMPLEMENT_WIDGET(SDevMenuMainMenuWidget)

void SDevMenuMainMenuWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SDevMenuMainMenuWidget::Construct(const FArguments& InArgs)
{
	GeneratedMenu = InArgs._GeneratedMenu;
	MenuPath = InArgs._MenuPath;

	const FWindowStyle* Style = &FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window");

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		.Padding(0.0f)
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(&Style->BackgroundBrush)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				InArgs._Content.Widget
			]
		]
	];
}
