// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreDisposable.h"
#include "DevPadInputController.h"
#include "DevPadTypes.h"
#include "Widgets/DevPadPanelWidget.h"
#include "DevPadManager.generated.h"

class UDevPadSettings;
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

	void Initialize();
	virtual void Reset() override;

	FORCEINLINE void SetPadRegistry(UDevPadRegistry& InPadRegistry) { PadRegistry = &InPadRegistry; }

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

	FORCEINLINE const FDevPadExecutionContext& GetCurrentExecutionContext() const { return CurrentExecutionContext; }

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

	UPROPERTY(Transient)
	FDevPadExecutionContext CurrentExecutionContext;

	FORCEINLINE bool IsPadPaused() const { return InputController->IsInputPaused(); }
	void ResumePad() const;
	void PausePad() const;

	void ExecutePadInput(const EDevPadInput InPadInput);
	EDevPadInputExecution ExecutePageInput(const UObject* WorldContextObject);
	EDevPadInputExecution ExecuteDefaultInput(const UObject* WorldContextObject);

	bool CreateWidget();
	void DestroyWidget();
	void RefreshWidget() const;

	void PopulateWidget() const;
	void PopulateWidgetHeader(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;
	void PopulateWidgetInfoContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;
	void PopulateWidgetPageContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const;

	void OnConsoleActivationStateChanged(bool bActive);
	void OnSettingsChanged(const UDevPadSettings* Settings) const;
};
