// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadHeaderWidget.h"

const FText UDevPadHeaderWidget::DefaultTitle = INVTEXT("DevPad");

FDevPadHeaderData UDevPadHeaderWidget::GetData() const
{
	FDevPadHeaderData Data;
	Data.IsVisible = IsVisible();
	Data.Title = GetTitle();
	return Data;
}

void UDevPadHeaderWidget::SetData(const FDevPadHeaderData& InData)
{
	SetVisibility((InData.IsVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SetTitle((!InData.Title.IsEmpty()) ? InData.Title : DefaultTitle);
}
