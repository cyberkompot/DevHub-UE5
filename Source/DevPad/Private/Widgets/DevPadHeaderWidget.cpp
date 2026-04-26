// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadHeaderWidget.h"

const FText UDevPadHeaderWidget::DefaultTitle = INVTEXT("DevPad");

FDevPadHeaderData UDevPadHeaderWidget::GetData() const
{
	FDevPadHeaderData Data;
	Data.IsVisible = IsVisible();
	Data.Title = GetTitle();
	if (PreviousPageButton) { Data.PreviousPage = PreviousPageButton->GetData(); }
	if (NextPageButton) { Data.NextPage = NextPageButton->GetData(); }
	return Data;
}

void UDevPadHeaderWidget::SetData(const FDevPadHeaderData& InData)
{
	SetVisibility((InData.IsVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SetTitle((!InData.Title.IsEmpty()) ? InData.Title : DefaultTitle);
	if (PreviousPageButton) { PreviousPageButton->SetData(InData.PreviousPage); }
	if (NextPageButton) { NextPageButton->SetData(InData.NextPage); }
}
