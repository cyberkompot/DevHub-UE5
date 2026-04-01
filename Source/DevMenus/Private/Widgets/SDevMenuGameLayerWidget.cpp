// Copyright (c) Alexandr Pereverzev.

#include "SDevMenuGameLayerWidget.h"

#include "Widgets/Layout/SDPIScaler.h"

namespace DevMenu::Setting
{
	static float GMenuScale = 1.0f;
	static FAutoConsoleVariableRef CVarMenuScale(TEXT("DevHub.Menu.Settings.Scale"), GMenuScale, TEXT("Dev Menu UI Scale"));
}

using namespace DevMenu::Setting;

SLATE_IMPLEMENT_WIDGET(SDevMenuGameLayerWidget)

void SDevMenuGameLayerWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SDevMenuGameLayerWidget::Construct(const FArguments& InArgs)
{
	WorldType = InArgs._WorldType;

	ChildSlot
	[
		SAssignNew(DPIScaler, SDPIScaler)
		.DPIScale(this, &SDevMenuGameLayerWidget::GetDPIScale)
		[
			SAssignNew(Overlay, SOverlay)
		]
	];
}

float SDevMenuGameLayerWidget::GetDPIScale() const
{
	float Scale = GMenuScale;

#if WITH_EDITOR
	if (WorldType == EWorldType::Editor || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview || WorldType == EWorldType::Inactive)
	{
		if (const TSharedPtr<SDPIScaler> ParentDPIScaler = GetParentDPIScaler())
		{
			const FChildren* ParentDPIScalerChildren = ParentDPIScaler->GetChildren();
			const TSharedPtr<const SWidget> ParentDPIScalerContent = (ParentDPIScalerChildren && ParentDPIScalerChildren->NumSlot() != 0) ? ParentDPIScalerChildren->GetChildAt(0) : TSharedPtr<const SWidget>();
			const FVector2D ScaledSizeVector = ParentDPIScaler->ComputeDesiredSize(1.0f);
			const FVector2D OriginalSizeVector = (ParentDPIScalerContent) ? ParentDPIScalerContent->GetDesiredSize() : FVector2D::Zero();
			const double ParentScale = (OriginalSizeVector.X != 0.0)
				? (ScaledSizeVector.X / OriginalSizeVector.X)
				: (OriginalSizeVector.Y != 0.0)
					? (ScaledSizeVector.Y / OriginalSizeVector.Y)
					: 1.0;
			if (ParentScale != 0.0)
			{
				Scale *= 1.0 / ParentScale;
			}
		}
	}
#endif // WITH_EDITOR

	return Scale;
}

TSharedPtr<SDPIScaler> SDevMenuGameLayerWidget::GetParentDPIScaler() const
{
	const TSharedPtr<SWidget> ParentWidget = GetParentWidget();
	if (CachedParentWidget != ParentWidget)
	{
		CachedParentWidget = ParentWidget;
		CachedDPIScalerWidget = nullptr;
	}

	TSharedPtr<SDPIScaler> ParentDPIScaler = (CachedDPIScalerWidget.IsValid()) ? CachedDPIScalerWidget.Pin() : nullptr;
	if (!ParentDPIScaler)
	{
		const FName DPIScalerType = DPIScaler->GetType();
		TSharedPtr<SWidget> TestWidget = ParentWidget;
		while (TestWidget.IsValid())
		{
			if (TestWidget->GetType() == DPIScalerType)
			{
				ParentDPIScaler = StaticCastSharedPtr<SDPIScaler>(TestWidget);
				CachedDPIScalerWidget = ParentDPIScaler;
				break;
			}
			TestWidget = TestWidget->GetParentWidget();
		};
	}

	return ParentDPIScaler;
}
