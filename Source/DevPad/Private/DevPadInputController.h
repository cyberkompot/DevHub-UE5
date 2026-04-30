// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.h"
#include "DevInputTypes.h"
#include "DevPadTypes.h"
#include "InputCoreTypes.h"
#include "DevPadInputController.generated.h"

struct FDevInputShortcut;
class UDevPadPanelWidget;

DECLARE_DELEGATE_OneParam(FDevPadInputEvent, const EDevPadInput);

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevPadInputController final : public UDevCoreResettable
{
	GENERATED_BODY()

public:
	UDevPadInputController();

	virtual void Reset() override;

public:
	void BindInput(FDevPadInputEvent&& InDelegate);
	void UnbindInput();

	FORCEINLINE bool IsInputPaused() const { return bInputPaused; }
	FORCEINLINE void ResumeInput() { bInputPaused = false; }
	FORCEINLINE void PauseInput() { bInputPaused = true; }

	void ConsumeInputEvent() const;

private:
	static const TArray<TKeyValuePair<EDevPadInput, FDevInputShortcut>> PadInputShortcuts;

	TArray<FDevInputShortcutDelegateBinding*> Bindings;
	TSet<FKey> InputEventKeysToAutoConsume;
	bool bInputPaused = false;
	FDevPadInputEvent OnPadInputEvent;

	void OnInputEvent(const FDevInputKeyEventArgs& InKeyParams);
	void OnInputShortcutEvent(const EDevPadInput InPadInput);
};
