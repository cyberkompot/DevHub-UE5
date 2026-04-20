// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadPanelWidget.h"

void UDevPadPanelWidget::UpdateWidget()
{
	const UDevPadPanelData& Data = GetDataOrRequestedDefault<UDevPadPanelData>();
	if (PanelHeader)
	{
		PanelHeader->SetData(Data.HeaderData);
	}
	if (PanelContent)
	{
		if (PanelContent->GetContent() != Data.PageWidget)
		{
			PanelContent->SetContent(Data.PageWidget);
		}
		if (Data.PageWidget)
		{
			Data.PageWidget->UpdateWidget();
		}
	}
}
