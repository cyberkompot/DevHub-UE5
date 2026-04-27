// Copyright (c) Alexandr Pereverzev.

#include "DevInputProcessor.h"

#include "DevCore.h"
#include "DevInputLogging.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerInput.h"
#include "Input/Events.h"

using namespace DevInput::Logging;

void FDevInputProcessor::Reset()
{
	OnInputTypeChanged.Clear();
	OnInputEvent.Clear();
	ConsumedDownKeys.Reset();
}

void FDevInputProcessor::ConsumeInputEvent()
{
	if (CurrentInputEvent)
	{
		bConsumeInputEvent = true;

		const FInputKeyManager& InputKeyManager = FInputKeyManager::Get();
		const FKey& Key = CurrentInputEvent->Key;
		const uint32* KeyCodePtr;
		const uint32* CharacterPtr;
		InputKeyManager.GetCodesFromKey(Key, KeyCodePtr, CharacterPtr);
		const uint32 KeyCode = (KeyCodePtr) ? *KeyCodePtr : 0;
		const uint32 Character = (CharacterPtr) ? *CharacterPtr : 0;

		UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Input event consumed: Key = %s, KeyCode = %u, Character = %u, Event = %s"), *Key.ToString(), KeyCode, Character, *EnumToString(CurrentInputEvent->Event));
	}
	else
	{
		UE_LOG_FUNCTION(LogDevInputs, Warning, TEXT("Attempt to consume an input event outside of its handler has no effect"));
	}
}

void FDevInputProcessor::EmulateKeyPress(const FKey& Key) const
{
	const FInputKeyManager& InputKeyManager = FInputKeyManager::Get();
	const uint32* KeyCodePtr;
	const uint32* CharacterPtr;
	InputKeyManager.GetCodesFromKey(Key, KeyCodePtr, CharacterPtr);
	const uint32 KeyCode = (KeyCodePtr) ? *KeyCodePtr : 0;
	const uint32 Character = (CharacterPtr) ? *CharacterPtr : 0;

	UE_LOG_FUNCTION(LogDevInputs, Log, TEXT("Key press emulated: Key = %s, KeyCode = %u, Character = %u"), *Key.ToString(), KeyCode, Character);

	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.OnKeyDown(KeyCode, Character, false);
	SlateApp.OnKeyUp(KeyCode, Character, false);
}

EDevInputType FDevInputProcessor::GetInputType(const FKey& Key) const
{
	if (Key.IsTouch() || Key.IsGesture())
	{
		return EDevInputType::Touch;
	}
	if (Key.IsGamepadKey() || Key.IsAnalog())
	{
		return EDevInputType::Gamepad;
	}
	if (Key.IsMouseButton())
	{
		return EDevInputType::Mouse;
	}
	// Keyboard remained.
	{
		return EDevInputType::Keyboard;
	}
}

void FDevInputProcessor::SetCurrentInputType(const EDevInputType InInputType)
{
	if (CurrentInputType != InInputType)
	{
		UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Input type changed: Type = %s"), *EnumToString(InInputType));
		
		CurrentInputType = InInputType;
		OnInputTypeChanged.Broadcast(InInputType);
	}
}

bool FDevInputProcessor::ProcessDownEvent(const FInputKeyParams& Params)
{
	CurrentInputEvent = &Params;
	bConsumeInputEvent = false;
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	if (bConsumeInputEvent)
	{
		ConsumedDownKeys.Add(Params.Key);
	}
	else
	{
		ConsumedDownKeys.Remove(Params.Key);
	}
	return bConsumeInputEvent;
}

bool FDevInputProcessor::ProcessUpEvent(const FInputKeyParams& Params)
{
	CurrentInputEvent = &Params;
	bConsumeInputEvent = ConsumedDownKeys.Contains(Params.Key);;
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	ConsumedDownKeys.Remove(Params.Key);
	return bConsumeInputEvent;
}

bool FDevInputProcessor::ProcessUnpairedEvent(const FInputKeyParams& Params)
{
	CurrentInputEvent = &Params;
	bConsumeInputEvent = false;
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	return bConsumeInputEvent;
}

bool FDevInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Key down: Key = %s, KeyCode = %u, Character = %u"), *InKeyEvent.GetKey().ToString(), InKeyEvent.GetKeyCode(), InKeyEvent.GetCharacter());

	SetCurrentInputType(GetInputType(InKeyEvent.GetKey()));

	FInputKeyParams Params = {};
	Params.Key = InKeyEvent.GetKey();
	Params.Event = IE_Pressed;
	Params.Delta.X = 1.0;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = Params.Key.IsAnalog() ? 1 : 0;
#if UE_COMPATIBILITY_SUPPORTED_KEY_EVENT_GET_INPUT_DEVICE_ID
	Params.InputDevice = InKeyEvent.GetInputDeviceId();
#endif // UE_COMPATIBILITY_KEY_EVENT_GET_INPUT_DEVICE_ID

	return ProcessDownEvent(Params);
}

bool FDevInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Key up: Key = %s, KeyCode = %u, Character = %u"), *InKeyEvent.GetKey().ToString(), InKeyEvent.GetKeyCode(), InKeyEvent.GetCharacter());

	FInputKeyParams Params = {};
	Params.Key = InKeyEvent.GetKey();
	Params.Event = IE_Released;
	Params.Delta.X = 0.0;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = Params.Key.IsAnalog() ? 1 : 0;
#if UE_COMPATIBILITY_SUPPORTED_KEY_EVENT_GET_INPUT_DEVICE_ID
	Params.InputDevice = InKeyEvent.GetInputDeviceId();
#endif // UE_COMPATIBILITY_KEY_EVENT_GET_INPUT_DEVICE_ID

	return ProcessUpEvent(Params);
}

bool FDevInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	// Note: We avoid setting InputType based on analog events, as they may be affected by stick drift.
	// The switch to Gamepad type will occur in HandleKeyDownEvent() when the analog input is translated into a key-down event.

	return false;
}

bool FDevInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	SetCurrentInputType(EDevInputType::Mouse);

	return false;
}

bool FDevInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Mouse button down: Button = %s"), *MouseEvent.GetEffectingButton().ToString());

	SetCurrentInputType(EDevInputType::Mouse);

	FInputKeyParams Params = {};
	Params.Key = MouseEvent.GetEffectingButton();
	Params.Event = IE_Pressed;
	Params.Delta.X = 1.0;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = 0;
#if UE_COMPATIBILITY_SUPPORTED_KEY_EVENT_GET_INPUT_DEVICE_ID
	Params.InputDevice = MouseEvent.GetInputDeviceId();
#endif // UE_COMPATIBILITY_KEY_EVENT_GET_INPUT_DEVICE_ID

	return ProcessDownEvent(Params);
}

bool FDevInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Mouse button up: Button = %s"), *MouseEvent.GetEffectingButton().ToString());

	FInputKeyParams Params = {};
	Params.Key = MouseEvent.GetEffectingButton();
	Params.Event = IE_Released;
	Params.Delta.X = 0.0;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = 0;
#if UE_COMPATIBILITY_SUPPORTED_KEY_EVENT_GET_INPUT_DEVICE_ID
	Params.InputDevice = MouseEvent.GetInputDeviceId();
#endif // UE_COMPATIBILITY_KEY_EVENT_GET_INPUT_DEVICE_ID

	return ProcessUpEvent(Params);
}

bool FDevInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	SetCurrentInputType(EDevInputType::Mouse);

	return false;
}

bool FDevInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	SetCurrentInputType(EDevInputType::Mouse);

	return false;
}

bool FDevInputProcessor::HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	SetCurrentInputType(EDevInputType::Gamepad);

	return false;
}
