// Copyright (c) Alexandr Pereverzev.

#include "DevPadInputController.h"

#include "DevInputs.h"
#include "DevInputTypes.h"
#include "DevPadLogging.h"

const TArray<TKeyValuePair<EDevPadInput, FDevInputShortcut>> UDevPadInputController::PadInputShortcuts
{
	{ EDevPadInput::DPadUp, "D-pad Up" },
	{ EDevPadInput::DPadLeft, "D-pad Left" },
	{ EDevPadInput::DPadRight, "D-pad Right" },
	{ EDevPadInput::DPadDown, "D-pad Down" },

	{ EDevPadInput::FaceUp, "Face Up" },
	{ EDevPadInput::FaceLeft, "Face Left" },
	{ EDevPadInput::FaceRight, "Face Right" },
	{ EDevPadInput::FaceDown, "Face Down" },

	{ EDevPadInput::LeftShoulder, "Left Shoulder" },
	{ EDevPadInput::RightShoulder, "Right Shoulder" },

	{ EDevPadInput::LeftTrigger, "Left Trigger" },
	{ EDevPadInput::RightTrigger, "Right Trigger" },

	{ EDevPadInput::LeftThumbstick, "Left Thumbstick" },
	{ EDevPadInput::RightThumbstick, "Right Thumbstick" },

	{ EDevPadInput::LeftPlusRightShoulders, "Left Shoulder + Right Shoulder" },
	{ EDevPadInput::LeftPlusRightTriggers, "Left Trigger + Right Trigger" },
	{ EDevPadInput::LeftPlusRightThumbsticks, "Left Thumbstick + Right Thumbstick" },
};

void UDevPadInputController::Reset()
{
	UnbindInput();
}

void UDevPadInputController::BindInput(FDevPadInputEvent&& InDelegate)
{
	if (Bindings.IsEmpty())
	{
		if (const UDevInputs* DevInputs = UDevInputs::Get(this))
		{
			Bindings.Reserve(PadInputShortcuts.Num());
			for (const TKeyValuePair<EDevPadInput, FDevInputShortcut>& KV : PadInputShortcuts)
			{
				const EDevPadInput& PadInput = KV.Key;
				const FDevInputShortcut& PadInputShortcut = KV.Value;

				FDevInputShortcutDelegateBinding& Binding = DevInputs->BindShortcut(PadInputShortcut);
				Binding.Delegate.BindDelegate(this, &ThisClass::ExecutePadInput, PadInput);

				Bindings.Emplace(&Binding);
			}
		}

		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad input bound"));
	}

	OnInputEvent = MoveTempIfPossible(InDelegate);
}

void UDevPadInputController::UnbindInput()
{
	OnInputEvent.Unbind();

	if (!Bindings.IsEmpty())
	{
		if (const UDevInputs* DevInputs = UDevInputs::Get(this))
		{
			for (const FDevInputShortcutDelegateBinding* Binding : Bindings)
			{
				(void)DevInputs->RemoveBinding(*Binding);
			}
		}
		Bindings.Reset();

		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad input unbound"));
	}
}

void UDevPadInputController::ConsumeCurrentInputEvent() const
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->ConsumeCurrentInputEvent();
	}
}

void UDevPadInputController::ExecutePadInput(const EDevPadInput InPadInput)
{
	UE_LOG_FUNCTION(LogDevPad, VeryVerbose, TEXT("DevPad input triggered: PadInput = %s"), *EnumToLog(InPadInput));
	OnInputEvent.ExecuteIfBound(InPadInput);
}
