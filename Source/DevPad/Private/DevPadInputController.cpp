// Copyright (c) Alexandr Pereverzev.

#include "DevPadInputController.h"

#include "DevInputs.h"
#include "DevInputShortcutBuilder.h"
#include "DevInputTypes.h"
#include "DevPadLogging.h"
#include "GameFramework/PlayerInput.h"

const TArray<TKeyValuePair<EDevPadInput, FDevInputShortcut>> UDevPadInputController::PadInputShortcuts
{
	{ EDevPadInput::DPadUp, "D-pad Up | Num 7" },
	{ EDevPadInput::DPadRight, "D-pad Right | Num 4" },
	{ EDevPadInput::DPadLeft, "D-pad Left | Num 1" },
	{ EDevPadInput::DPadDown, "D-pad Down | Num 0" },

	{ EDevPadInput::FaceUp, "Face Up | Num 9" },
	{ EDevPadInput::FaceRight, "Face Right | Num 6" },
	{ EDevPadInput::FaceLeft, "Face Left | Num 3" },
	{ EDevPadInput::FaceDown, "Face Down | Num ." },

	{ EDevPadInput::LeftShoulder, "Left Shoulder | Num /" },
	{ EDevPadInput::RightShoulder, "Right Shoulder | Num *" },

	{ EDevPadInput::LeftTrigger, "Left Trigger | Num -" },
	{ EDevPadInput::RightTrigger, "Right Trigger | Num +" },

	{ EDevPadInput::LeftThumbstick, "Left Thumbstick | Num 8" },
	{ EDevPadInput::RightThumbstick, "Right Thumbstick | Num 2" },

	{ EDevPadInput::LeftPlusRightShoulders, "Left Shoulder + Right Shoulder | Num / + Num *" },
	{ EDevPadInput::LeftPlusRightTriggers, "Left Trigger + Right Trigger | Num - + Num +" },
	{ EDevPadInput::LeftPlusRightThumbsticks, "Left Thumbstick + Right Thumbstick | Num 8 + Num 2" },
};

UDevPadInputController::UDevPadInputController()
{
	FDevInputSequence InputSequence;
	InputEventKeysToAutoConsume.Reserve(PadInputShortcuts.Num() * 2);
	for (const TKeyValuePair<EDevPadInput, FDevInputShortcut>& KV : PadInputShortcuts)
	{
		const FDevInputShortcut& PadInputShortcut = KV.Value;
		FDevInputShortcutBuilder::FromName(PadInputShortcut.GetName(), InputSequence);
		for (const FDevInputToken& Token : InputSequence)
		{
			if (!EDevInputTokens::IsSpecialToken(Token))
			{
				InputEventKeysToAutoConsume.Add(Token.GetName());
			}
		}
	}
}

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
				Binding.Delegate.BindDelegate(this, &ThisClass::OnInputShortcutEvent, PadInput);

				Bindings.Emplace(&Binding);
			}
			DevInputs->OnInputEvent().AddUObject(this, &ThisClass::OnInputEvent);
		}

		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad input bound"));
	}

	OnPadInputEvent = MoveTempIfPossible(InDelegate);
}

void UDevPadInputController::UnbindInput()
{
	OnPadInputEvent.Unbind();

	if (!Bindings.IsEmpty())
	{
		if (const UDevInputs* DevInputs = UDevInputs::Get(this))
		{
			for (const FDevInputShortcutDelegateBinding* Binding : Bindings)
			{
				(void)DevInputs->RemoveBinding(*Binding);
			}

			DevInputs->OnInputEvent().RemoveAll(this);
		}
		Bindings.Reset();

		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad input unbound"));
	}
}

void UDevPadInputController::ConsumeInputEvent() const
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->ConsumeInputEvent();
	}
}

void UDevPadInputController::OnInputEvent(const FDevInputKeyEventArgs& InKeyParams)
{
	if (IsInputPaused()) { return; }

	if (InputEventKeysToAutoConsume.Contains(InKeyParams.Key))
	{
		UE_LOG_FUNCTION(LogDevPad, VeryVerbose, TEXT("DevPad input key triggered. Consuming event: Key = %s, Event = %s"), *InKeyParams.Key.ToString(), *EnumToLog(InKeyParams.Event));
		if (const UDevInputs* DevInputs = UDevInputs::Get(this))
		{
			DevInputs->ConsumeInputEvent();
		}
	}
}

void UDevPadInputController::OnInputShortcutEvent(const EDevPadInput InPadInput)
{
	if (IsInputPaused()) { return; }

	UE_LOG_FUNCTION(LogDevPad, VeryVerbose, TEXT("DevPad input shortcut triggered: PadInput = %s"), *EnumToLog(InPadInput));
	OnPadInputEvent.ExecuteIfBound(InPadInput);
}
