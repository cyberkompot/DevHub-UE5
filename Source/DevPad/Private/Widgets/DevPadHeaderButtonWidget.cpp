// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadHeaderButtonWidget.h"

const FText UDevPadHeaderButtonWidget::DefaultTitle = INVTEXT("DevPad");

FDevPadHeaderButtonData UDevPadHeaderButtonWidget::GetData() const
{
	FDevPadHeaderButtonData Data;
	Data.IsVisible = IsVisible();
	Data.Title = GetLabel();
	return Data;
}

void UDevPadHeaderButtonWidget::SetData(const FDevPadHeaderButtonData& InData)
{
	SetVisibility((InData.IsVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SetLabel((!InData.Title.IsEmpty()) ? InData.Title : DefaultTitle);
}
