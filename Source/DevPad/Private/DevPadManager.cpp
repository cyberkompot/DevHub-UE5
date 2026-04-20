// Copyright (c) Alexandr Pereverzev.

#include "DevPadManager.h"

#include "DevCore.h"
#include "DevPad.h"
#include "DevPadInputController.h"
#include "DevPadLogging.h"
#include "DevPadRegistry.h"
#include "DevPadSettings.h"
#include "DevPadStackController.h"
#include "DevPadTypes.h"
#include "Widgets/DevPadPanelWidget.h"

UDevPadManager::UDevPadManager()
{
	InputController = CreateDefaultSubobject<UDevPadInputController>("InputController", true);
	StackController = CreateDefaultSubobject<UDevPadStackController>("StackController", true);
}

void UDevPadManager::Reset()
{
	HidePad();
}

void UDevPadManager::ShowPad(const FName InPageName)
{
	if (IsPadVisible()) { return; }

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad showing: Page = %s"), *InPageName.ToString());

	if (!CreateWidget()) { return; }

	InputController->BindInput(FDevPadInputEvent::CreateUObject(this, &ThisClass::ExecutePadInput));

	StackController->SetCommonPage(PadRegistry->GetCommonPage());
	StackController->PushToStack(PadRegistry->GetFirstPage());

	PopulateWidget();
}

void UDevPadManager::HidePad()
{
	if (!IsPadVisible()) { return; }

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad hiding"));

	InputController->Reset();
	StackController->Reset();
	DestroyWidget();
}

void UDevPadManager::TogglePad()
{
	if (IsPadVisible())
	{
		HidePad();
	}
	else
	{
		ShowPad(NAME_None);
	}
}

void UDevPadManager::NavigateToPreviousPage() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	if (const UDevPadPage* TopPage = StackController->GetTopPage())
	{
		if (StackController->IsMainPage(TopPage))
		{
			if (UDevPadPage* NavigationPage = PadRegistry->GetPreviousPage(TopPage, true); NavigationPage && NavigationPage != TopPage)
			{
				StackController->PopFromStack();
				StackController->PushToStack(NavigationPage);
				UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to previous page: Page = %s"), *PageToLog(NavigationPage));
				PopulateWidget();
				return;
			}
		}
		else
		{
			StackController->PopFromStack();
			UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to parent page: Page = %s"), *PageToLog(StackController->GetTopPage()));
			PopulateWidget();
			return;
		}
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no next previous available"));
}

void UDevPadManager::NavigateToNextPage() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	if (const UDevPadPage* TopPage = StackController->GetTopPage())
	{
		if (UDevPadPage* NavigationPage = PadRegistry->GetNextPage(TopPage, true); NavigationPage && NavigationPage != TopPage)
		{
			StackController->PopFromStack();
			StackController->PushToStack(NavigationPage);
			UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to next page: Page = %s"), *PageToLog(NavigationPage));
			PopulateWidget();
			return;
		}
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no next page available"));
}

void UDevPadManager::BeginDestroy()
{
	Reset();
	Super::BeginDestroy();
}

void UDevPadManager::ExecutePadInput(const EDevPadInput InPadInput)
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget is missing. DevPad input cannot be executed: PadInput = %s"), *EnumToLog(InPadInput));
		return;
	}

	const UWorld* WorldContextObject = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	FDevPadExecutionContext ExecutionContext;
	ExecutionContext.DevPad = UDevPad::Get(WorldContextObject);
	ExecutionContext.PadInput = InPadInput;
	ExecutionContext.PadStack = StackController;

	EDevPadInputExecution Execution = EDevPadInputExecution::Continue;

	if (Execution == EDevPadInputExecution::Continue)
	{
		Execution = ExecutePageInput(WorldContextObject, ExecutionContext);
	}
	if (Execution == EDevPadInputExecution::Continue)
	{
		Execution = ExecuteDefaultInput(WorldContextObject, ExecutionContext);
	}

	if (Execution == EDevPadInputExecution::Break)
	{
		InputController->ConsumeCurrentInputEvent();
	}
}

EDevPadInputExecution UDevPadManager::ExecutePageInput(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext)
{
	EDevPadInputExecution Execution = EDevPadInputExecution::Continue;

	const TArray<UDevPadPage*>& Pages = ExecutionContext.PadStack->GetAllPages();
	for (int32 i = Pages.Num() - 1; i >= 0; --i)
	{
		if (const UDevPadPage* Page = Pages[i])
		{
			Execution = Page->ExecutePageInput(WorldContextObject, ExecutionContext);
			if (Execution == EDevPadInputExecution::Break)
			{
				UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad page action executed: PadInput = %s, Page = %s"), *EnumToLog(ExecutionContext.PadInput), *PageToLog(Page));
				break;
			}
		}
	}

	return Execution;
}

EDevPadInputExecution UDevPadManager::ExecuteDefaultInput(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext)
{
	switch (ExecutionContext.PadInput)
	{
		case EDevPadInput::LeftShoulder:
			NavigateToPreviousPage();
			return EDevPadInputExecution::Break;

		case EDevPadInput::RightShoulder:
			NavigateToNextPage();
			return EDevPadInputExecution::Break;

		case EDevPadInput::LeftPlusRightShoulders:
			HidePad();
			return EDevPadInputExecution::Break;

		default:
			UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("No DevPad action associated with input: PadInput = %s"), *EnumToLog(ExecutionContext.PadInput));
			return EDevPadInputExecution::Break;
	}
}

bool UDevPadManager::CreateWidget()
{
	const UDevPadSettings* Settings = GetDefault<UDevPadSettings>();
	if (!Settings)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad settings are missing. DevPad could not be constructed and shown"));
		return false;
	}

	const TSubclassOf<UDevPadPanelWidget> PadWidgetClass = Settings->PadWidgetClass.LoadSynchronous();
	if (!PadWidgetClass)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget class or asset is missing. DevPad could not be constructed and shown"));
		return false;
	}

	PadWidget = Cast<UDevPadPanelWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), PadWidgetClass, "DevPad"));
	if (!PadWidget)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget creation failed. DevPad could not be shown"));
		return false;
	}

	PadWidget->AddToViewport();
	return true;
}

void UDevPadManager::PopulateWidget() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget is missing. DevPad could not be populated"));
		return;
	}

	UDevPadPanelData* Data = PadWidget->GetDataOrCreate<UDevPadPanelData>();
	if (!Data)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget data is missing. DevPad could not be populated"));
		return;
	}

	const UObject* WorldContextObject = GetWorld();
	const UDevPadPage* TopPage = StackController->GetTopPage();

	PopulateWidgetHeader(WorldContextObject, TopPage, Data);
	PopulateWidgetContent(WorldContextObject, TopPage, Data);

	PadWidget->UpdateWidget();
}

void UDevPadManager::PopulateWidgetHeader(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const
{
	// Header.
	if (!TopPage)
	{
		Data->HeaderData = FDevPadHeaderData();
		Data->HeaderData.PreviousPageNavigation.IsVisible = false;
		Data->HeaderData.NextPageNavigation.IsVisible = false;
		return;
	}

	auto HeaderNavigationData = [this, TopPage](const EUINavigation InNavigationType, FDevPadHeaderNavigationData& OutHeaderNavigationData)
	{
		if (StackController->IsCommonPage(TopPage))
		{
			if (InNavigationType == EUINavigation::Previous)
			{
				OutHeaderNavigationData.IsVisible = true;
				OutHeaderNavigationData.Title = INVTEXT("↑ Back");
				return;
			}
		}

		if (StackController->IsMainPage(TopPage))
		{
			const UDevPadPage* NavigationPage = (InNavigationType == EUINavigation::Previous)
				? PadRegistry->GetPreviousPage(TopPage, true)
				: PadRegistry->GetNextPage(TopPage, true);
			if (NavigationPage)
			{
				OutHeaderNavigationData.IsVisible = true;
				OutHeaderNavigationData.Title = (InNavigationType == EUINavigation::Previous)
					? FText::FromString(TEXT("← ") + NavigationPage->GetPageTitle().Get().ToString())
					: FText::FromString(NavigationPage->GetPageTitle().Get().ToString() + TEXT(" →"));
				return;
			}
		}

		OutHeaderNavigationData = FDevPadHeaderNavigationData();
		OutHeaderNavigationData.IsVisible = false;
	};

	Data->HeaderData.Title = TopPage->GetPageTitle().Get();
	HeaderNavigationData(EUINavigation::Previous, Data->HeaderData.PreviousPageNavigation);
	HeaderNavigationData(EUINavigation::Next, Data->HeaderData.NextPageNavigation);
}

void UDevPadManager::PopulateWidgetContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const
{
	if (!TopPage)
	{
		Data->PageWidget = nullptr;
		return;
	}

	const TSubclassOf<UDevPadPageWidget> CurrentPageWidgetClass = (Data->PageWidget) ? Data->PageWidget->GetClass() : nullptr;
	const TSubclassOf<UDevPadPageWidget> TopPageWidgetClass = TopPage->GetPageWidgetClass(WorldContextObject).LoadSynchronous();
	UE_CLOG_FUNCTION(!TopPageWidgetClass, LogDevPad, Warning, TEXT("DevPad page widget class is missing. DevPad page could not be constructed and shown: Page = %s"), *PageToLog(TopPage));

	if (CurrentPageWidgetClass != TopPageWidgetClass)
	{
		if (Data->PageWidget)
		{
			Data->PageWidget = nullptr;
		}
		if (TopPageWidgetClass)
		{
			Data->PageWidget = Cast<UDevPadPageWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), TopPageWidgetClass, "DevPadPage"));
			UE_CLOG_FUNCTION(!Data->PageWidget, LogDevPad, Warning, TEXT("DevPad page widget creation failed. DevPad page could not be constructed and shown: Page = %s, WidgetClass = %s"), *PageToLog(TopPage), *GetNameSafe(TopPageWidgetClass));
		}
	}

	if (Data->PageWidget)
	{
		FDevPadWidgetContext PageWidgetContext;
		PageWidgetContext.DevPad = UDevPad::Get(WorldContextObject);
		PageWidgetContext.PadStack = StackController;
		PageWidgetContext.PageData = Data->PageWidget->GetDataOrCreate<UDevPadPageData>();
		PageWidgetContext.PageWidget = Data->PageWidget;
		TopPage->PopulatePageWidget(WorldContextObject, PageWidgetContext);
	}
}

void UDevPadManager::DestroyWidget()
{
	if (PadWidget)
	{
		PadWidget->RemoveFromParent();
		PadWidget->MarkAsGarbage();
		PadWidget = nullptr;
	}
}
