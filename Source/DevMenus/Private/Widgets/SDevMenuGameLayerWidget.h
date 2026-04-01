// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Engine/EngineTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Templates/SharedPointer.h"

class SDPIScaler;

class SDevMenuGameLayerWidget final : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SDevMenuGameLayerWidget, SCompoundWidget)

public:
	SLATE_BEGIN_ARGS(SDevMenuGameLayerWidget) {}
		SLATE_ARGUMENT_DEFAULT(EWorldType::Type, WorldType) { EWorldType::Type::None };
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FORCEINLINE void AddWidget(const TSharedRef<SWidget>& InWidget) const { Overlay->AddSlot(INDEX_NONE)[ InWidget ]; }
	FORCEINLINE void RemoveWidget(const TSharedRef<SWidget>& InWidget) const { Overlay->RemoveSlot(InWidget); }

private:
	EWorldType::Type WorldType = EWorldType::None;

	TSharedPtr<SDPIScaler> DPIScaler;
	TSharedPtr<SOverlay> Overlay;

	mutable TWeakPtr<SDPIScaler> CachedDPIScalerWidget;
	mutable TWeakPtr<SWidget> CachedParentWidget;

	float GetDPIScale() const;
	TSharedPtr<SDPIScaler> GetParentDPIScaler() const;
};
