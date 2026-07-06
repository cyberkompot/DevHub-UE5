// Copyright (c) Alexandr Pereverzev.

#include "DevPadActions.h"

#include "DevPad.h"
#include "DevPadLogging.h"
#include "DevPadTypes.h"

void FDevPadActionBase::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (const UDevPad* DevPad = UDevPad::Get(WorldContextObject))
	{
		OnExecuteActionWithContext(WorldContextObject, DevPad->GetCurrentExecutionContext());
	}
	else
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad subsystem is missing. Action could not be executed"));
	}
}

void FDevPadActionOpenSubPage::OnExecuteActionWithContext(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext) const
{
	if (ExecutionContext.DevPad)
	{
		ExecutionContext.DevPad->OpenSubPage(Page.LoadSynchronous());
	}
}

void FDevPadActionToggleSubPage::OnExecuteActionWithContext(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext) const
{
	if (ExecutionContext.DevPad)
	{
		ExecutionContext.DevPad->ToggleSubPage(Page.LoadSynchronous());
	}
}

void FDevPadActionCloseSubPage::OnExecuteActionWithContext(const UObject* WorldContextObject, const FDevPadExecutionContext& ExecutionContext) const
{
	if (ExecutionContext.DevPad)
	{
		ExecutionContext.DevPad->CloseSubPage(Page.LoadSynchronous());
	}
}
