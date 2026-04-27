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
#include "Engine/Engine.h"
#include "Widgets/DevPadLayoutWidget.h"
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

	RefreshWidget();
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

void UDevPadManager::HidePad()
{
	if (!IsPadVisible()) { return; }

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad hiding"));

	InputController->Reset();
	StackController->Reset();
	DestroyWidget();
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
				UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to previous page: Page = %s"), *PageToLog(NavigationPage));
				StackController->PopFromStack();
				StackController->PushToStack(NavigationPage);
				RefreshWidget();
				return;
			}
		}
		else
		{
			UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to parent page: Page = %s"), *PageToLog(TopPage));
			StackController->PopFromStack();
			RefreshWidget();
			return;
		}
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no previous page available"));
}

void UDevPadManager::NavigateToNextPage() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	// Sub-pages are not in the registry, so GetNextPage() returns null when on a sub-page.
	// NavigateToPreviousPage() handles sub-pages by popping them; next has no equivalent behaviour.
	if (const UDevPadPage* TopPage = StackController->GetTopPage())
	{
		if (UDevPadPage* NavigationPage = PadRegistry->GetNextPage(TopPage, true); NavigationPage && NavigationPage != TopPage)
		{
			UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad navigating to next page: Page = %s"), *PageToLog(NavigationPage));
			StackController->PopFromStack();
			StackController->PushToStack(NavigationPage);
			RefreshWidget();
			return;
		}
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no next page available"));
}

void UDevPadManager::OpenSubPage(UDevPadPage* InPage) const
{
	if (!InPage)
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad sub-page is undefined"));
		return;
	}

	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	if (StackController->GetAllPages().Find(InPage) != INDEX_NONE)
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad page is already open: Page = %s"), *PageToLog(InPage));
		return;
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad opening sub-page: Page = %s"), *PageToLog(InPage));
	StackController->PushToStack(InPage);
	RefreshWidget();
}

void UDevPadManager::ToggleSubPage(UDevPadPage* InPage) const
{
	if (!InPage)
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad sub-page is undefined"));
		return;
	}

	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	if (StackController->GetAllPages().Find(InPage) != INDEX_NONE)
	{
		CloseSubPage(InPage);
	}
	else
	{
		OpenSubPage(InPage);
	}
}

void UDevPadManager::CloseSubPage(UDevPadPage* InPage) const
{
	if (!InPage)
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad sub-page is undefined"));
		return;
	}

	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	if (!StackController->IsSubPage(InPage))
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad sub-page is not open: Page = %s"), *PageToLog(InPage));
		return;
	}

	for (const UDevPadPage* TopPage = StackController->GetTopPage(); TopPage; TopPage = StackController->GetTopPage())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad closing sub-page: Page = %s"), *PageToLog(TopPage));
		StackController->PopFromStack();
		if (TopPage == InPage)
		{
			break;
		}
	}
	RefreshWidget();
}

void UDevPadManager::CloseCurrentSubPage() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	const UDevPadPage* TopPage = StackController->GetTopPage();
	if (!TopPage || !StackController->IsSubPage(TopPage))
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no opened sub-pages"));
		return;
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad closing sub-page: Page = %s"), *PageToLog(TopPage));
	StackController->PopFromStack();
	RefreshWidget();
}

void UDevPadManager::CloseAllSubPages() const
{
	if (!IsPadVisible())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad is not visible"));
		return;
	}

	const UDevPadPage* TopPage = StackController->GetTopPage();
	if (!TopPage || !StackController->IsSubPage(TopPage))
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad no opened sub-pages"));
		return;
	}

	for (; TopPage && StackController->IsSubPage(TopPage); TopPage = StackController->GetTopPage())
	{
		UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad closing sub-page: Page = %s"), *PageToLog(TopPage));
		StackController->PopFromStack();
	}
	RefreshWidget();
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
	const TArray<UDevPadPage*>& Pages = ExecutionContext.PadStack->GetAllPages();
	for (int32 i = Pages.Num() - 1; i >= 0; --i)
	{
		if (const UDevPadPage* Page = Pages[i])
		{
			if (const EDevPadInputExecution Execution = Page->ExecutePageInput(WorldContextObject, ExecutionContext); Execution == EDevPadInputExecution::Break)
			{
				UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad page action executed: PadInput = %s, Page = %s"), *EnumToLog(ExecutionContext.PadInput), *PageToLog(Page));
				RefreshWidget();
				return EDevPadInputExecution::Break;
			}
		}
	}

	return EDevPadInputExecution::Continue;
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
			// Default input execute always ends with the break to suppress further input handling.
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

	if (!PadLayoutWidget)
	{
		PadLayoutWidget = Cast<UDevPadLayoutWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), UDevPadLayoutWidget::StaticClass(), "DevPadLayout"));
		if (!PadLayoutWidget)
		{
			UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad layout widget creation failed. DevPad could not be shown"));
			return false;
		}
	}

	PadWidget = Cast<UDevPadPanelWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), PadWidgetClass, "DevPad"));
	if (!PadWidget)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad widget creation failed. DevPad could not be shown"));
		return false;
	}

	PadLayoutWidget->SetAlignment(Settings->PadWidgetAlignment);
	PadLayoutWidget->SetScale(Settings->PadWidgetScale);
	PadLayoutWidget->SetContent(PadWidget);
	PadLayoutWidget->AddToViewport();
	return true;
}

void UDevPadManager::DestroyWidget()
{
	if (PadLayoutWidget)
	{
		PadLayoutWidget->RemoveFromParent();
	}

	if (PadWidget)
	{
		PadWidget->RemoveFromParent();
		PadWidget->MarkAsGarbage();
		PadWidget = nullptr;
	}
}

void UDevPadManager::RefreshWidget() const
{
	PopulateWidget();
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
	PopulateWidgetInfoContent(WorldContextObject, TopPage, Data);
	PopulateWidgetPageContent(WorldContextObject, TopPage, Data);

	PadWidget->UpdateWidget();
}

void UDevPadManager::PopulateWidgetHeader(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const
{
	if (!TopPage)
	{
		Data->HeaderData = FDevPadHeaderData();
		Data->HeaderData.PreviousPage.IsVisible = false;
		Data->HeaderData.NextPage.IsVisible = false;
		return;
	}

	TStringBuilder<NAME_SIZE> TitleBuilder;
	for (const UDevPadPage* Page : StackController->GetAllPages())
	{
		if (StackController->IsCommonPage(Page)) { continue; }
		if (TitleBuilder.Len()) { TitleBuilder.Append(TEXT(" → ")); }
		TitleBuilder.Append(Page->GetPageTitle().Get().ToString());
	}

	auto PopulateHeaderButtonData = [this, TopPage](const EUINavigation InNavigationType, FDevPadHeaderButtonData& OutHeaderButtonData)
	{
		if (StackController->IsSubPage(TopPage))
		{
			if (InNavigationType == EUINavigation::Previous)
			{
				OutHeaderButtonData.IsVisible = true;
				OutHeaderButtonData.Title = INVTEXT("↑ Back");
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
				OutHeaderButtonData.IsVisible = true;
				OutHeaderButtonData.Title = (InNavigationType == EUINavigation::Previous)
					? FText::FromString(TEXT("← ") + NavigationPage->GetPageTitle().Get().ToString())
					: FText::FromString(NavigationPage->GetPageTitle().Get().ToString() + TEXT(" →"));
				return;
			}
		}

		OutHeaderButtonData = FDevPadHeaderButtonData();
		OutHeaderButtonData.IsVisible = false;
	};

	Data->HeaderData.Title = FText::FromStringView(TitleBuilder.ToView());
	PopulateHeaderButtonData(EUINavigation::Previous, Data->HeaderData.PreviousPage);
	PopulateHeaderButtonData(EUINavigation::Next, Data->HeaderData.NextPage);
}

void UDevPadManager::PopulateWidgetInfoContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const
{
	if (!TopPage)
	{
		Data->InfoWidget = nullptr;
		return;
	}

	const TSubclassOf<UDevPadInfoWidget> CurrentWidgetClass = (Data->InfoWidget) ? Data->InfoWidget->GetClass() : nullptr;
	const TSubclassOf<UDevPadInfoWidget> TopPageWidgetClass = TopPage->GetInfoWidgetClass(WorldContextObject).LoadSynchronous();

	if (CurrentWidgetClass != TopPageWidgetClass)
	{
		if (Data->InfoWidget)
		{
			Data->InfoWidget = nullptr;
		}
		if (TopPageWidgetClass)
		{
			Data->InfoWidget = Cast<UDevPadInfoWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), TopPageWidgetClass, "DevPadInfo"));
			UE_CLOG_FUNCTION(!Data->InfoWidget, LogDevPad, Warning, TEXT("DevPad page widget creation failed. DevPad page could not be fully constructed and shown: Page = %s, WidgetClass = %s"), *PageToLog(TopPage), *GetNameSafe(TopPageWidgetClass));
		}
	}
}

void UDevPadManager::PopulateWidgetPageContent(const UObject* WorldContextObject, const UDevPadPage* TopPage, UDevPadPanelData* Data) const
{
	if (!TopPage)
	{
		Data->PageWidget = nullptr;
		return;
	}

	const TSubclassOf<UDevPadPageWidget> CurrentWidgetClass = (Data->PageWidget) ? Data->PageWidget->GetClass() : nullptr;
	const TSubclassOf<UDevPadPageWidget> TopPageWidgetClass = TopPage->GetPageWidgetClass(WorldContextObject).LoadSynchronous();
	UE_CLOG_FUNCTION(!TopPageWidgetClass, LogDevPad, Warning, TEXT("DevPad page widget class is missing. DevPad page could not be fully constructed and shown: Page = %s"), *PageToLog(TopPage));

	if (CurrentWidgetClass != TopPageWidgetClass)
	{
		if (Data->PageWidget)
		{
			Data->PageWidget = nullptr;
		}
		if (TopPageWidgetClass)
		{
			Data->PageWidget = Cast<UDevPadPageWidget>(UUserWidget::CreateWidgetInstance(*GetWorld(), TopPageWidgetClass, "DevPadPage"));
			UE_CLOG_FUNCTION(!Data->PageWidget, LogDevPad, Warning, TEXT("DevPad page widget creation failed. DevPad page could not be fully constructed and shown: Page = %s, WidgetClass = %s"), *PageToLog(TopPage), *GetNameSafe(TopPageWidgetClass));
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
