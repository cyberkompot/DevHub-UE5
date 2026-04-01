// Copyright (c) Alexandr Pereverzev.

#include "SDevMenuConsoleResponseWidget.h"

#include "Styling/AppStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

SLATE_IMPLEMENT_WIDGET(SDevMenuConsoleResponseWidget)

void SDevMenuConsoleResponseWidget::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SDevMenuConsoleResponseWidget::Construct(const FArguments& InArgs)
{
	LogsCapacity = InArgs._LogsCapacity;
	LogsExpiration = InArgs._LogsExpiration;

	LogsLines.Reserve(LogsCapacity);

	ChildSlot
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Left)
	.Padding(FMargin(4.f))
	[
		SAssignNew(VBox, SVerticalBox)
	];

	RegisterActiveTimer(0.1f, FWidgetActiveTimerDelegate::CreateSP(SharedThis(this), &ThisClass::UpdateWidget));
}

FSlateColor SDevMenuConsoleResponseWidget::GetColorForVerbosity(const ELogVerbosity::Type InVerbosity) const
{
	switch (InVerbosity)
	{
		case ELogVerbosity::Fatal:
		case ELogVerbosity::Error:
			return FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f));
		case ELogVerbosity::Warning:
			return FSlateColor(FLinearColor(1.f, 0.7f, 0.0f));
		case ELogVerbosity::Log:
		case ELogVerbosity::Display:
			return FSlateColor::UseForeground();
		case ELogVerbosity::Verbose:
		case ELogVerbosity::VeryVerbose:
			return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
		default:
			return FSlateColor::UseForeground();
	}
}

bool SDevMenuConsoleResponseWidget::UpdateLogLines(const double InCurrentTime, const float InDeltaTime)
{
	bool bDirty = false;

	for (int i = LogsLines.Num() - 1; i >= 0; --i)
	{
		if (InCurrentTime >= LogsLines[i].ExpirationTime)
		{
			LogsLines.RemoveAt(i);
			bDirty = true;
		}
	}

	FLogLine LogLine;
	while (LogsLinesQueue.Dequeue(LogLine))
	{
		if (LogsLines.Num() == LogsCapacity) { LogsLines.PopFront(); }
		LogLine.ExpirationTime = InCurrentTime + LogsExpiration;
		LogsLines.Emplace(MoveTemp(LogLine));
		bDirty = true;
	}

	return bDirty;
}

EActiveTimerReturnType SDevMenuConsoleResponseWidget::UpdateWidget(const double InCurrentTime, const float InDeltaTime)
{
	if (UpdateLogLines(InCurrentTime, InDeltaTime))
	{
		VBox->ClearChildren();
		for (const FLogLine& LogLine : LogsLines)
		{
			VBox->AddSlot()
			.AutoHeight()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Left)
			.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::Get().GetBrush("Menu.Background"))
				.Padding(FMargin(6.f, 4.f))
				[
					SNew(STextBlock)
					.Text(LogLine.Text)
					.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("Menu.Label"))
					.ColorAndOpacity(GetColorForVerbosity(LogLine.Verbosity))
					.AutoWrapText(true)
				]
			];
		}
	}
	return EActiveTimerReturnType::Continue;
}
