// Copyright (c) Alexandr Pereverzev.

#include "DevPad.h"

#include "DevCore.h"
#include "DevPadLogging.h"
#include "DevPadManager.h"
#include "DevPadRegistry.h"

UDevPad::UDevPad()
{
	PadRegistry = CreateDefaultSubobject<UDevPadRegistry>("PadRegistry", true);
	PadManager = CreateDefaultSubobject<UDevPadManager>("PadManager", true);

	PadManager->SetMenuRegistry(*PadRegistry);
}

bool UDevPad::IsPadVisible() const
{
	return PadManager->IsPadVisible();
}

void UDevPad::ShowPad(const FName InPageName) const
{
	PadManager->ShowPad(InPageName);
}

void UDevPad::TogglePad() const
{
	PadManager->TogglePad();
}

void UDevPad::HidePad() const
{
	PadManager->HidePad();
}

void UDevPad::NavigateToPreviousPage() const
{
	PadManager->NavigateToPreviousPage();
}

void UDevPad::NavigateToNextPage() const
{
	PadManager->NavigateToNextPage();
}

void UDevPad::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PadRegistry->Initialize();
}

void UDevPad::Deinitialize()
{
	PadManager->Reset();
	PadRegistry->Reset();

	Super::Deinitialize();
}

namespace DevPad::Console
{
	static FAutoConsoleCommandWithWorldAndArgs ShowPadCommand(
		TEXT("DevHub.Pad.ShowPad"),
		TEXT("Shows the DevPad, optionally opening a specified page if provided."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (const UDevPad* DevPad = UDevPad::Get(World))
			{
				if (Args.IsEmpty())
				{
					DevPad->ShowPad();
				}
				else
				{
					DevPad->ShowPad(FName(*Args[0]));
				}
			}
			else
			{
				UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad subsystem is missing. DevPad could not be shown"));
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs HideMenuCommand(
		TEXT("DevHub.Pad.HidePad"),
		TEXT("Hides the DevPad."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World)
		{
			if (const UDevPad* DevPad = UDevPad::Get(World))
			{
				DevPad->HidePad();
			}
			else
			{
				UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad subsystem is missing. DevPad could not be hidden"));
			}
		}));
} // DevPad::Console
