// Copyright (c) Alexandr Pereverzev.

#include "Pages/DevPadControllerPage.h"

#include "DevActionTypes.h"
#include "DevPadLogging.h"

TSoftClassPtr<UDevPadInfoWidget> UDevPadControllerPage::GetInfoWidgetClass(const UObject* WorldContextObject) const
{
	return (!InfoWidgetClass.IsNull())
		? TSoftClassPtr<UDevPadInfoWidget>(InfoWidgetClass.ToSoftObjectPath())
		: Super::GetInfoWidgetClass(WorldContextObject);
}

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

	const FDevPadControllerPageActionView ActionView = GetPageActionInPage(this, PageExecutionContext.PadInput);
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

	auto PopulatePageActionData = [WorldContextObject](const FDevPadControllerPageActionView& InActionView, FDevPadActionData& OutActionData)
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
	
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::DPadUp), Data->DPadUpAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::DPadLeft), Data->DPadLeftAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::DPadRight), Data->DPadRightAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::DPadDown), Data->DPadDownAction);

	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::FaceUp), Data->FaceUpAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::FaceLeft), Data->FaceLeftAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::FaceRight), Data->FaceRightAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::FaceDown), Data->FaceDownAction);

	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::LeftTrigger), Data->LeftTriggerAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::RightTrigger), Data->RightTriggerAction);

	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::LeftThumbstick), Data->LeftThumbstickAction);
	PopulatePageActionData(GetPageActionInStack(PadStack, EDevPadInput::RightThumbstick), Data->RightThumbstickAction);
}

FDevPadControllerPageActionView UDevPadControllerPage::GetPageActionInPage(const UDevPadControllerPage* InPadPage, const EDevPadInput InPadInput) const
{
	if (!InPadPage) { return FDevPadControllerPageActionView(); }

	switch (InPadInput)
	{
		case EDevPadInput::DPadUp: return FDevPadControllerPageActionView(*InPadPage, InPadPage->DPadUpAction);
		case EDevPadInput::DPadRight: return FDevPadControllerPageActionView(*InPadPage, InPadPage->DPadRightAction);
		case EDevPadInput::DPadLeft: return FDevPadControllerPageActionView(*InPadPage, InPadPage->DPadLeftAction);
		case EDevPadInput::DPadDown: return FDevPadControllerPageActionView(*InPadPage, InPadPage->DPadDownAction);

		case EDevPadInput::FaceUp: return FDevPadControllerPageActionView(*InPadPage, InPadPage->FaceUpAction);
		case EDevPadInput::FaceRight: return FDevPadControllerPageActionView(*InPadPage, InPadPage->FaceRightAction);
		case EDevPadInput::FaceLeft: return FDevPadControllerPageActionView(*InPadPage, InPadPage->FaceLeftAction);
		case EDevPadInput::FaceDown: return FDevPadControllerPageActionView(*InPadPage, InPadPage->FaceDownAction);

		case EDevPadInput::LeftTrigger: return FDevPadControllerPageActionView(*InPadPage, InPadPage->LeftTriggerAction);
		case EDevPadInput::RightTrigger: return FDevPadControllerPageActionView(*InPadPage, InPadPage->RightTriggerAction);

		case EDevPadInput::LeftThumbstick: return FDevPadControllerPageActionView(*InPadPage, InPadPage->LeftThumbstickAction);
		case EDevPadInput::RightThumbstick: return FDevPadControllerPageActionView(*InPadPage, InPadPage->RightThumbstickAction);

		default: return FDevPadControllerPageActionView();
	}
}

FDevPadControllerPageActionView UDevPadControllerPage::GetPageActionInStack(const UDevPadStack* InPadStack, const EDevPadInput InPadInput) const
{
	if (!InPadStack) { return FDevPadControllerPageActionView(); }

	// Iterate through Stack and Common pages until reaching the first page of a different type.
	const TArray<UDevPadPage*>& StackPages = InPadStack->GetAllPages();
	for (int32 i = StackPages.Num() - 1; i >= 0; --i)
	{
		const UDevPadControllerPage* ControllerPage = Cast<UDevPadControllerPage>(StackPages[i]);
		if (!ControllerPage) { return FDevPadControllerPageActionView(); }

		if (const FDevPadControllerPageActionView ActionView = GetPageActionInPage(ControllerPage, InPadInput); ActionView.IsValid())
		{
			return ActionView;
		}
	}

	return FDevPadControllerPageActionView();
}
