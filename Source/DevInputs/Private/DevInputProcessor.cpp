// Copyright (c) Alexandr Pereverzev.

#include "DevInputProcessor.h"

#include "DevCore.h"
#include "DevInputLogging.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerInput.h"
#include "Input/Events.h"

using namespace DevInput::Logging;

void FDevInputProcessor::Dispose()
{
	OnInputTypeChanged.Clear();
	OnInputEvent.Clear();
}

void FDevInputProcessor::EmulateKeyPress(const FKey& InKey) const
{
	const FInputKeyManager& InputKeyManager = FInputKeyManager::Get();
	const uint32* KeyCodePtr;
	const uint32* CharacterPtr;
	InputKeyManager.GetCodesFromKey(InKey, KeyCodePtr, CharacterPtr);
	const uint32 KeyCode = (KeyCodePtr) ? *KeyCodePtr : 0;
	const uint32 Character = (CharacterPtr) ? *CharacterPtr : 0;

	UE_LOG_FUNCTION(LogDevInputs, Log, TEXT("Key press emulated: Key = %s, KeyCode = %u, Character = %u"), *InKey.ToString(), KeyCode, Character);

	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.OnKeyDown(KeyCode, Character, false);
	SlateApp.OnKeyUp(KeyCode, Character, false);
}

EDevInputType FDevInputProcessor::GetInputType(const FKey& InKey) const
{
	if (InKey.IsTouch() || InKey.IsGesture())
	{
		return EDevInputType::Touch;
	}
	if (InKey.IsGamepadKey() || InKey.IsAnalog())
	{
		return EDevInputType::Gamepad;
	}
	if (InKey.IsMouseButton())
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

	OnInputEvent.Broadcast(Params);

	return false;
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

	OnInputEvent.Broadcast(Params);
	
	return false;
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

	OnInputEvent.Broadcast(Params);

	return false;
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

	OnInputEvent.Broadcast(Params);

	return false;
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
