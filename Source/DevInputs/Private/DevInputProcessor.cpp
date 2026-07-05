// Copyright (c) Alexandr Pereverzev.

#include "DevInputProcessor.h"

#include "DevCore.h"
#include "DevInputLogging.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerInput.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#if UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
#include "InputKeyEventArgs.h"
#endif
#include "Input/Events.h"

using namespace DevInput::Logging;

FDevInputProcessor::FDevInputProcessor()
{
	CurrentGamepadName = GetGamepadNameByPlatform();
}

void FDevInputProcessor::Reset()
{
	OnInputTypeChanged.Clear();
	OnInputGamepadChanged.Clear();
	OnInputEvent.Clear();
	ConsumedDownKeys.Reset();
	CurrentGamepadName = GetGamepadNameByPlatform();
	LastGamepadInputDeviceName = NAME_None;
	LastGamepadHardwareDeviceIdentifier.Reset();
}

void FDevInputProcessor::ConsumeInputEvent()
{
	if (CurrentInputEvent)
	{
		const FInputKeyManager& InputKeyManager = FInputKeyManager::Get();
		const FKey& Key = CurrentInputEvent->Key;
		const uint32* KeyCodePtr;
		const uint32* CharacterPtr;
		InputKeyManager.GetCodesFromKey(Key, KeyCodePtr, CharacterPtr);
		const uint32 KeyCode = (KeyCodePtr) ? *KeyCodePtr : 0;
		const uint32 Character = (CharacterPtr) ? *CharacterPtr : 0;
		UE_CLOG_FUNCTION(!bConsumeCurrentInputEvent, LogDevInputs, Verbose, TEXT("Input event consumed: Key = %s, KeyCode = %u, Character = %u, Event = %s"), *Key.ToString(), KeyCode, Character, *EnumToString(CurrentInputEvent->Event));

		bConsumeCurrentInputEvent = true;
	}
	else
	{
		UE_LOG_FUNCTION(LogDevInputs, Warning, TEXT("Attempt to consume an input event outside of its execution has no effect"));
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

FName FDevInputProcessor::GetGamepadNameByPlatform() const
{
#if defined(PLATFORM_PS4) && PLATFORM_PS4
	return EDevInputGamepadNames::PS4;
#elif defined(PLATFORM_PS5) && PLATFORM_PS5
	return EDevInputGamepadNames::PS5;
#elif (defined(PLATFORM_XSX) && PLATFORM_XSX) || (defined(PLATFORM_XBOXONEGDK) && PLATFORM_XBOXONEGDK)
	return EDevInputGamepadNames::Xbox;
#else
	return EDevInputGamepadNames::Generic;
#endif
}

FName FDevInputProcessor::GetGamepadNameByHardware(const FName InInputDeviceName, const FString& InHardwareDeviceIdentifier) const
{
	// PlayStation.
	if (InHardwareDeviceIdentifier.Contains(TEXT("DualSense")))
	{
		return EDevInputGamepadNames::PS5;
	}
	if (InHardwareDeviceIdentifier.Contains(TEXT("DualShock")))
	{
		return EDevInputGamepadNames::PS4;
	}
	if (InHardwareDeviceIdentifier.Contains(TEXT("VID_054C"))) // SONY vendor ID.
	{
		const bool bIsDualSense = InHardwareDeviceIdentifier.Contains(TEXT("PID_0CE6")) // DualSense.
			|| InHardwareDeviceIdentifier.Contains(TEXT("PID_0DF2")); // DualSense Edge.
		return (bIsDualSense) ? EDevInputGamepadNames::PS5 : EDevInputGamepadNames::PS4;
	}

	// Xbox.
	if ((InInputDeviceName == "XInputInterface") || InHardwareDeviceIdentifier.Contains(TEXT("Xbox")))
	{
		return EDevInputGamepadNames::Xbox;
	}

	// Steam.
	if ((InInputDeviceName == "SteamController") || InHardwareDeviceIdentifier.Contains(TEXT("VID_28DE"))) // Valve vendor ID.
	{
		return EDevInputGamepadNames::Steam;
	}

	return GetGamepadNameByPlatform();
}

void FDevInputProcessor::SetCurrentGamepadName(const FName InGamepadName)
{
	if (CurrentGamepadName != InGamepadName)
	{
		UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Gamepad changed: Gamepad = %s"), *InGamepadName.ToString());

		CurrentGamepadName = InGamepadName;
		OnInputGamepadChanged.Broadcast(InGamepadName);
	}
}

void FDevInputProcessor::RefreshCurrentGamepadName()
{
#if PLATFORM_DESKTOP
	if (const FInputDeviceScope* DeviceScope = FInputDeviceScope::GetCurrent())
	{
		if ((DeviceScope->InputDeviceName != LastGamepadInputDeviceName) || (DeviceScope->HardwareDeviceIdentifier != LastGamepadHardwareDeviceIdentifier))
		{
			LastGamepadInputDeviceName = DeviceScope->InputDeviceName;
			LastGamepadHardwareDeviceIdentifier = DeviceScope->HardwareDeviceIdentifier;
			SetCurrentGamepadName(GetGamepadNameByHardware(DeviceScope->InputDeviceName, DeviceScope->HardwareDeviceIdentifier));
		}
	}
#else
	// Consoles only support their native gamepads. This value is initialized in the constructor.
#endif
}

bool FDevInputProcessor::ProcessDownEvent(const FDevInputKeyEventArgs& Params)
{
	CurrentInputEvent = &Params;
	bConsumeCurrentInputEvent = false;
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeCurrentInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	if (bConsumeCurrentInputEvent)
	{
		ConsumedDownKeys.Add(Params.Key);
	}
	else
	{
		ConsumedDownKeys.Remove(Params.Key);
	}
	return bConsumeCurrentInputEvent;
}

bool FDevInputProcessor::ProcessUpEvent(const FDevInputKeyEventArgs& Params)
{
	CurrentInputEvent = &Params;
	bConsumeCurrentInputEvent = ConsumedDownKeys.Contains(Params.Key);
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeCurrentInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	ConsumedDownKeys.Remove(Params.Key);
	return bConsumeCurrentInputEvent;
}

bool FDevInputProcessor::ProcessUnpairedEvent(const FDevInputKeyEventArgs& Params)
{
	CurrentInputEvent = &Params;
	bConsumeCurrentInputEvent = false;
	ON_SCOPE_EXIT
	{
		CurrentInputEvent = nullptr;
		bConsumeCurrentInputEvent = false;
	};

	OnInputEvent.Broadcast(Params);

	return bConsumeCurrentInputEvent;
}

bool FDevInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
#if WITH_EDITOR
	if (GIntraFrameDebuggingGameThread) { return false; }
#endif

	UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Key down: Key = %s, KeyCode = %u, Character = %u"), *InKeyEvent.GetKey().ToString(), InKeyEvent.GetKeyCode(), InKeyEvent.GetCharacter());

	const EDevInputType InputType = GetInputType(InKeyEvent.GetKey());

	SetCurrentInputType(InputType);
	if (InputType == EDevInputType::Gamepad)
	{
		RefreshCurrentGamepadName();
	}

	FDevInputKeyEventArgs Params = {};
	Params.Key = InKeyEvent.GetKey();
	Params.Event = IE_Pressed;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = Params.Key.IsAnalog() ? 1 : 0;
#if UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
	Params.AmountDepressed = 1.0f;
#else
	Params.Delta.X = 1.0;
#endif // UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
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

	FDevInputKeyEventArgs Params = {};
	Params.Key = InKeyEvent.GetKey();
	Params.Event = IE_Released;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = Params.Key.IsAnalog() ? 1 : 0;
#if UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
	Params.AmountDepressed = 0.0f;
#else
	Params.Delta.X = 0.0;
#endif // UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
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

	FDevInputKeyEventArgs Params = {};
	Params.Key = MouseEvent.GetEffectingButton();
	Params.Event = IE_Pressed;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = 0;
#if UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
	Params.AmountDepressed = 1.0f;
#else
	Params.Delta.X = 1.0;
#endif // UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
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

	FDevInputKeyEventArgs Params = {};
	Params.Key = MouseEvent.GetEffectingButton();
	Params.Event = IE_Released;
	Params.DeltaTime = SlateApp.GetDeltaTime();
	Params.NumSamples = 0;
#if UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
	Params.AmountDepressed = 0.0f;
#else
	Params.Delta.X = 0.0;
#endif // UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS
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
	RefreshCurrentGamepadName();

	return false;
}
