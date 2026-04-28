// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"
#include "Framework/Application/IInputProcessor.h"

struct FInputKeyParams;
struct FKey;

/**
 * We use our own InputProcessor because in "UI Only" input mode, UGameViewportClient::OnInputKey is not triggered.
 * This approach allows us to handle input shortcuts even when the game viewport is not focused,
 * such as during PIE mode. However, this introduces an issue in PIE, where multiple game systems and viewports exist,
 * each receiving input simultaneously. This conflict needs to be properly addressed in the future.
 */
class FDevInputProcessor final : public TSharedFromThis<FDevInputProcessor>, public IInputProcessor
{
public:
	EDevInputType CurrentInputType = EDevInputType::Undefined;

	FDevInputTypeChanged OnInputTypeChanged;
	FDevInputEvent OnInputEvent;

	void Reset();

	void ConsumeInputEvent();
	void EmulateKeyPress(const FKey& Key) const;

private:
	const FInputKeyParams* CurrentInputEvent = nullptr;
	bool bConsumeInputEvent = false;
	TSet<FKey> ConsumedDownKeys;

	EDevInputType GetInputType(const FKey& Key) const;
	void SetCurrentInputType(const EDevInputType InInputType);

	bool ProcessDownEvent(const FInputKeyParams& Params);
	bool ProcessUpEvent(const FInputKeyParams& Params);
	bool ProcessUnpairedEvent(const FInputKeyParams& Params);

	//~ Begin IInputProcessor interface.
	virtual const TCHAR* GetDebugName() const override { return TEXT("DevInput Processor"); }
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override;
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {};
	//~ End IInputProcessor interface.
};
