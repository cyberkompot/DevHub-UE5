// Copyright (c) Alexandr Pereverzev.

#include "DevPadInputController.h"

#include "DevInputs.h"
#include "DevInputTypes.h"
#include "DevPadLogging.h"

const TArray<TKeyValuePair<EDevPadInput, FDevInputShortcut>> UDevPadInputController::PadInputShortcuts
{
	{ EDevPadInput::DPadUp, "D-pad Up | Num 8" },
	{ EDevPadInput::DPadLeft, "D-pad Left | Num 7" },
	{ EDevPadInput::DPadRight, "D-pad Right | Num 4" },
	{ EDevPadInput::DPadDown, "D-pad Down | Num 1" },

	{ EDevPadInput::FaceUp, "Face Up | Num 9" },
	{ EDevPadInput::FaceLeft, "Face Left | Num 6" },
	{ EDevPadInput::FaceRight, "Face Right | Num 3" },
	{ EDevPadInput::FaceDown, "Face Down | Num 2" },

	{ EDevPadInput::LeftShoulder, "Left Shoulder | Num /" },
	{ EDevPadInput::RightShoulder, "Right Shoulder | Num *" },

	{ EDevPadInput::LeftTrigger, "Left Trigger | Num -" },
	{ EDevPadInput::RightTrigger, "Right Trigger | Num +" },

	{ EDevPadInput::LeftThumbstick, "Left Thumbstick | Num 0" },
	{ EDevPadInput::RightThumbstick, "Right Thumbstick | Num ." },

	{ EDevPadInput::LeftPlusRightShoulders, "Left Shoulder + Right Shoulder | Num / + Num *" },
	{ EDevPadInput::LeftPlusRightTriggers, "Left Trigger + Right Trigger | Num + + Enter" },
	{ EDevPadInput::LeftPlusRightThumbsticks, "Left Thumbstick + Right Thumbstick | Num 0 + Num ." },
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
