// Copyright (c) Alexandr Pereverzev.

#include "Widgets/DevPadControllerPageWidget.h"

void UDevPadControllerPageWidget::UpdateWidget()
{
	Super::UpdateWidget();

	auto UpdateActionWidget = [](UDevPadActionWidget* InActionWidget, const FDevPadActionData& InActionData)
	{
		if (InActionWidget)
		{
			InActionWidget->SetData(InActionData);
		}
	};

	const UDevPadControllerPageData& Data = GetDataOrRequestedDefault<UDevPadControllerPageData>();

	UpdateActionWidget(DPadUpAction, Data.DPadUpAction);
	UpdateActionWidget(DPadLeftAction, Data.DPadLeftAction);
	UpdateActionWidget(DPadRightAction, Data.DPadRightAction);
	UpdateActionWidget(DPadDownAction, Data.DPadDownAction);

	UpdateActionWidget(FaceUpAction, Data.FaceUpAction);
	UpdateActionWidget(FaceLeftAction, Data.FaceLeftAction);
	UpdateActionWidget(FaceRightAction, Data.FaceRightAction);
	UpdateActionWidget(FaceDownAction, Data.FaceDownAction);

	UpdateActionWidget(LeftTriggerAction, Data.LeftTriggerAction);
	UpdateActionWidget(RightTriggerAction, Data.RightTriggerAction);

	UpdateActionWidget(LeftThumbstickAction, Data.LeftThumbstickAction);
	UpdateActionWidget(RightThumbstickAction, Data.RightThumbstickAction);

	UpdateActionWidget(LeftPlusRightTriggersAction, Data.LeftPlusRightTriggersAction);
	UpdateActionWidget(LeftPlusRightThumbsticksAction, Data.LeftPlusRightThumbsticksAction);
}
