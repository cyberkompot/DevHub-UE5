// Copyright (c) Alexandr Pereverzev.

#include "DevPadActions.h"

#include "DevPad.h"
#include "DevPadLogging.h"

void FDevPadActionBase::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (UDevPad* DevPad = UDevPad::Get(WorldContextObject))
	{
		OnExecuteAction(WorldContextObject, DevPad);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DedPad subsystem is missing. Action could not be executed"));
	}
}

void FDevPadActionOpenSubPage::OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const
{
	DevPad->OpenSubPage(Page.LoadSynchronous());
}

void FDevPadActionCloseSubPage::OnExecuteAction(const UObject* WorldContextObject, UDevPad* DevPad) const
{
	DevPad->CloseSubPage(Page.LoadSynchronous());
}
