// Copyright (c) Alexandr Pereverzev.

#include "DevPadStackController.h"

#include "DevCoreCompatibility.h"
#include "DevPadLogging.h"

void UDevPadStackController::Reset()
{
	CommonPage = nullptr;
	Pages.Reset();
}

void UDevPadStackController::PushToStack(UDevPadPage* InPage)
{
	if (InPage)
	{
		Pages.AddUnique(InPage);
	}
}

void UDevPadStackController::PopFromStack()
{
	if (!IsStackEmpty())
	{
		Pages.Pop(DONT_ALLOW_SHRINKING);
	}
	else
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad page stack is empty. Nothing to pop from the stack"));
	}
}

void UDevPadStackController::ResetStack()
{
	Pages.Reset();
	if (CommonPage)
	{
		Pages.Add(CommonPage);
	}
}

void UDevPadStackController::SetCommonPage(UDevPadPage* InPage)
{
	if (CommonPage == InPage) { return; }

	if (CommonPage)
	{
		Pages.RemoveSingle(CommonPage);
	}

	CommonPage = InPage;

	if (CommonPage)
	{
		if (Pages.RemoveSingle(CommonPage))
		{
			UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad stack page is being used as a common page. Page was forcefully removed from the stack: Page = %s"), *PageToLog(CommonPage));
		}
		Pages.Insert(CommonPage, 0);
	}
}

void UDevPadStackController::ResetCommonPage()
{
	SetCommonPage(nullptr);
}
