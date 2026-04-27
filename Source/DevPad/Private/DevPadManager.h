// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.h"
#include "DevPadTypes.h"
#include "Widgets/DevPadPanelWidget.h"
#include "DevPadManager.generated.h"

class UDevPadInputController;
class UDevPadLayoutWidget;
class UDevPadPanelWidget;
class UDevPadRegistry;
class UDevPadStackController;

UCLASS(NotBlueprintType, NotBlueprintable)
class UDevPadManager final : public UDevCoreResettable
{
	GENERATED_BODY()

public:
	UDevPadManager();
	FORCEINLINE void SetPadRegistry(UDevPadRegistry& InPadRegistry) { PadRegistry = &InPadRegistry; }
	virtual void Reset() override;

public:
	FORCEINLINE bool IsPadVisible() const { return (!!PadWidget); }

	void ShowPad(const FName InPageName);
	void TogglePad();
	void HidePad();

	void NavigateToNextPage() const;
	void NavigateToPreviousPage() const;

	void OpenSubPage(UDevPadPage* InPage) const;
	void ToggleSubPage(UDevPadPage* InPage) const;
	void CloseSubPage(UDevPadPage* InPage) const;
	void CloseCurrentSubPage() const;
	void CloseAllSubPages() const;

public:
	//~ Begin UObject interface.
	virtual void BeginDestroy() override;
	//~ End UObject interface.

private:
	UPROPERTY(Transient)
	TObjectPtr<UDevPadInputController> InputController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDevPadStackController> StackController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDevPadRegistry> PadRegistry = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDevPadPanelWidget> PadWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDevPadLayoutWidget> PadLayoutWidget = nullptr;

	void ExecutePadInput(const EDevPadInput InPadInput);
	EDevPadInputExecution ExecutePageInput(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext);
	EDevPadInputExecution ExecuteDefaultInput(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext);

	bool CreateWidget();
	void DestroyWidget();
	void RefreshWidget() const;

	void PopulateWidget() const;
	void PopulateWidgetHeader(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;
	void PopulateWidgetInfoContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;
	void PopulateWidgetPageContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;
};
