// Copyright (c) Alexandr Pereverzev.

#include "Pages/DevPadControllerPage.h"

#include "DevActionTypes.h"
#include "DevPadLogging.h"

TSoftClassPtr<UDevPadPageWidget> UDevPadControllerPage::GetPageWidgetClass(const UObject* WorldContextObject) const
{
	return (!PageWidgetClass.IsNull())
		? TSoftClassPtr<UDevPadPageWidget>(PageWidgetClass.ToSoftObjectPath())
		: Super::GetPageWidgetClass(WorldContextObject);
}

EDevPadInputExecution UDevPadControllerPage::ExecutePageInput(const UObject* WorldContextObject, const FDevPadExecutionContext& PageExecutionContext) const
{
	const UDevPadStack* PadStack = PageExecutionContext.GetPadStack();
	if (!PadStack)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad pages stack is missing. DevPad input cannot be executed: PadInput = %s, Page = %s"), *EnumToLog(PageExecutionContext.PadInput), *PageToLog(this));
		return EDevPadInputExecution::Continue;
	}

	const FDevPadControllerPageConstActionView ActionView = GetPageAction(WorldContextObject, PadStack, PageExecutionContext.PadInput);
	const FDevAction* Action = ActionView.GetActionPtr();
	if (!Action)
	{
		return EDevPadInputExecution::Continue;
	}

	UE_LOG_FUNCTION(LogDevPad, Log, TEXT("DevPad page action executing: PadInput = %s, Page = %s, Action = %s"), *EnumToLog(PageExecutionContext.PadInput), *PageToLog(this), *ControllerPageActionViewToLog(WorldContextObject, ActionView));
	Action->ExecuteAction(WorldContextObject);
	return EDevPadInputExecution::Break;
}

void UDevPadControllerPage::PopulatePageWidget(const UObject* WorldContextObject, const FDevPadWidgetContext& PageWidgetContext) const
{
	Super::PopulatePageWidget(WorldContextObject, PageWidgetContext);

	const UDevPadStack* PadStack = PageWidgetContext.GetPadStack();
	if (!PadStack)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad pages stack is missing. Page widget cannot be populated: Page = %s"), *PageToLog(this));
		return;
	}

	UDevPadControllerPageData* Data = PageWidgetContext.GetPageData<UDevPadControllerPageData>();
	if (!Data)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad page data is missing. Page widget cannot be populated: Page = %s"), *PageToLog(this));
		return;
	}

	auto PopulatePageActionData = [WorldContextObject](const FDevPadControllerPageConstActionView& InActionView, FDevPadActionData& OutActionData)
	{
		if (const FDevAction* Action = InActionView.GetActionPtr(); Action && Action->GetActionVisibility(WorldContextObject))
		{
			OutActionData.Label = Action->GetActionLabel(WorldContextObject).Get();
			OutActionData.IsVisible = true;
		}
		else
		{
			OutActionData = FDevPadActionData();
		}
	};
	
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::DPadUp), Data->DPadUpAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::DPadLeft), Data->DPadLeftAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::DPadRight), Data->DPadRightAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::DPadDown), Data->DPadDownAction);

	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::FaceUp), Data->FaceUpAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::FaceLeft), Data->FaceLeftAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::FaceRight), Data->FaceRightAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::FaceDown), Data->FaceDownAction);

	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::LeftTrigger), Data->LeftTriggerAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::RightTrigger), Data->RightTriggerAction);

	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::LeftThumbstick), Data->LeftThumbstickAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::RightThumbstick), Data->RightThumbstickAction);

	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::LeftPlusRightTriggers), Data->LeftPlusRightTriggersAction);
	PopulatePageActionData(GetPageAction(WorldContextObject, PadStack, EDevPadInput::LeftPlusRightThumbsticks), Data->LeftPlusRightThumbsticksAction);
}

FDevPadControllerPageConstActionView UDevPadControllerPage::GetPageAction(const UObject* WorldContextObject, const UDevPadStack* InPadStack, const EDevPadInput InPadInput) const
{
	if (!InPadStack) { return FDevPadControllerPageConstActionView(); }

	auto GetActionByPadInput = [InPadInput](const UDevPadControllerPage& InControllerPage) -> FDevPadControllerPageConstActionView
	{
		switch (InPadInput)
		{
			case EDevPadInput::DPadUp: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.DPadUpAction);
			case EDevPadInput::DPadLeft: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.DPadLeftAction);
			case EDevPadInput::DPadRight: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.DPadRightAction);
			case EDevPadInput::DPadDown: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.DPadDownAction);

			case EDevPadInput::FaceUp: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.FaceUpAction);
			case EDevPadInput::FaceLeft: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.FaceLeftAction);
			case EDevPadInput::FaceRight: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.FaceRightAction);
			case EDevPadInput::FaceDown: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.FaceDownAction);

			case EDevPadInput::LeftTrigger: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.LeftTriggerAction);
			case EDevPadInput::RightTrigger: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.RightTriggerAction);

			case EDevPadInput::LeftThumbstick: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.LeftThumbstickAction);
			case EDevPadInput::RightThumbstick: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.RightThumbstickAction);

			case EDevPadInput::LeftPlusRightTriggers: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.LeftPlusRightTriggersAction);
			case EDevPadInput::LeftPlusRightThumbsticks: return FDevPadControllerPageConstActionView(InControllerPage, InControllerPage.LeftPlusRightThumbsticksAction);

			default: return FDevPadControllerPageConstActionView();
		}
	};

	// Common and Stack pages.
	const TArray<UDevPadPage*>& StackPages = InPadStack->GetAllPages();
	for (int32 i = StackPages.Num() - 1; i >= 0; --i)
	{
		const UDevPadControllerPage* ControllerPage = Cast<UDevPadControllerPage>(StackPages[i]);
		if (!ControllerPage) { return FDevPadControllerPageConstActionView(); }

		if (const FDevPadControllerPageConstActionView ActionView = GetActionByPadInput(*ControllerPage); ActionView.IsValid())
		{
			return ActionView;
		}
	}

	return FDevPadControllerPageConstActionView();
}
