// Copyright (c) Alexandr Pereverzev.

#include "DevPadActions.h"

#include "DevPad.h"
#include "DevPadLogging.h"
#include "DevPadTypes.h"

void FDevPadActionBase::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (UDevPad* DevPad = UDevPad::Get(WorldContextObject))
	{
		OnExecuteActionWithSubsystem(WorldContextObject, DevPad);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad subsystem is missing. Action could not be executed"));
	}
}

void FDevPadActionOpenSubPage::OnExecuteActionWithSubsystem(const UObject* WorldContextObject, UDevPad* DevPad) const
{
	DevPad->OpenSubPage(Page.LoadSynchronous());
}

void FDevPadActionToggleSubPage::OnExecuteActionWithSubsystem(const UObject* WorldContextObject, UDevPad* DevPad) const
{
	DevPad->ToggleSubPage(Page.LoadSynchronous());
}

void FDevPadActionCloseSubPage::OnExecuteActionWithSubsystem(const UObject* WorldContextObject, UDevPad* DevPad) const
{
	DevPad->CloseSubPage(Page.LoadSynchronous());
}
