// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Containers/Queue.h"
#include "Containers/RingBuffer.h"
#include "Logging/LogVerbosity.h"
#include "Styling/SlateColor.h"
#include "Templates/SharedPointer.h"
#include "Types/SlateEnums.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"

class SDevMenuConsoleResponseWidget final : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SDevMenuConsoleResponseWidget, SCompoundWidget)

public:
	SLATE_BEGIN_ARGS(SDevMenuConsoleResponseWidget) {}
		SLATE_ARGUMENT_DEFAULT(int32, LogsCapacity) { 4 };
		SLATE_ARGUMENT_DEFAULT(float, LogsExpiration) { 5.f }; // Seconds.
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FORCEINLINE void PushConsoleResponse(const ELogVerbosity::Type InVerbosity, FText&& InText) { LogsLinesQueue.Enqueue(FLogLine(InVerbosity, Forward<FText>(InText))); }

private:
	struct FLogLine
	{
		FLogLine() = default;
		FLogLine(const ELogVerbosity::Type InVerbosity, FText&& InText)
			: Verbosity(InVerbosity), Text(MoveTempIfPossible(InText)) {}

		ELogVerbosity::Type Verbosity = ELogVerbosity::Log;
		FText Text;
		double ExpirationTime = 0.0f;
	};

	TSharedPtr<SVerticalBox> VBox;

	int32 LogsCapacity = 0;
	float LogsExpiration = 0.f; // Seconds.

	TRingBuffer<FLogLine> LogsLines;
	TQueue<FLogLine, EQueueMode::Mpsc> LogsLinesQueue;

	FSlateColor GetColorForVerbosity(ELogVerbosity::Type InVerbosity) const;

	bool UpdateLogLines(double InCurrentTime, float InDeltaTime);
	EActiveTimerReturnType UpdateWidget(const double InCurrentTime, const float InDeltaTime);
};