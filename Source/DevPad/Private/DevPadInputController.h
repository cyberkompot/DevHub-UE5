// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.h"
#include "DevInputTypes.h"
#include "DevPadTypes.h"
#include "DevPadInputController.generated.h"

struct FDevInputShortcut;
struct FStreamableHandle;
class UDevPadPanelWidget;

DECLARE_DELEGATE_OneParam(FDevPadInputEvent, const EDevPadInput);

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevPadInputController final : public UDevCoreResettable
{
	GENERATED_BODY()

public:
	virtual void Reset() override;

public:
	void BindInput(FDevPadInputEvent&& InDelegate);
	void UnbindInput();

	void ConsumeCurrentInputEvent() const;

private:
	static const TArray<TKeyValuePair<EDevPadInput, FDevInputShortcut>> PadInputShortcuts;

	TArray<FDevInputShortcutDelegateBinding*> Bindings;
	FDevPadInputEvent OnInputEvent;

	void ExecutePadInput(const EDevPadInput InPadInput);
};
