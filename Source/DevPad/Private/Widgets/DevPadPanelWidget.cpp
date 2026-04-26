// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadPanelWidget.h"

void UDevPadPanelWidget::UpdateWidget()
{
	const UDevPadPanelData& Data = GetDataOrRequestedDefault<UDevPadPanelData>();
	if (PanelHeader)
	{
		PanelHeader->SetData(Data.HeaderData);
	}
	if (PanelInfoContent)
	{
		if (PanelInfoContent->GetContent() != Data.InfoWidget)
		{
			PanelInfoContent->SetContent(Data.InfoWidget);
		}
		if (Data.InfoWidget)
		{
			Data.InfoWidget->UpdateWidget();
		}
	}
	if (PanelPageContent)
	{
		if (PanelPageContent->GetContent() != Data.PageWidget)
		{
			PanelPageContent->SetContent(Data.PageWidget);
		}
		if (Data.PageWidget)
		{
			Data.PageWidget->UpdateWidget();
		}
	}
}
