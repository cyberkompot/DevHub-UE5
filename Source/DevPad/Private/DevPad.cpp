// Copyright (c) Alexandr Pereverzev.

#include "DevPad.h"

#include "DevCore.h"
#include "DevInputs.h"
#include "DevPadLogging.h"
#include "DevPadManager.h"
#include "DevPadRegistry.h"
#include "DevPadSettings.h"

UDevPad::UDevPad()
{
	PadRegistry = CreateDefaultSubobject<UDevPadRegistry>("PadRegistry", true);
	PadManager = CreateDefaultSubobject<UDevPadManager>("PadManager", true);

	PadManager->SetPadRegistry(*PadRegistry);
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

void UDevPad::OpenSubPage(UDevPadPage* InPage) const
{
	PadManager->OpenSubPage(InPage);
}

void UDevPad::ToggleSubPage(UDevPadPage* InPage) const
{
	PadManager->ToggleSubPage(InPage);
}

void UDevPad::CloseSubPage(UDevPadPage* InPage) const
{
	PadManager->CloseSubPage(InPage);
}

void UDevPad::CloseCurrentSubPage() const
{
	PadManager->CloseCurrentSubPage();
}

void UDevPad::CloseAllSubPages() const
{
	PadManager->CloseAllSubPages();
}

void UDevPad::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PadRegistry->Initialize();
	PadManager->Initialize();

	BindShortcut();
}

void UDevPad::Deinitialize()
{
	UnbindShortcut();

	PadManager->Reset();
	PadRegistry->Reset();

	Super::Deinitialize();
}

void UDevPad::BindShortcut()
{
	const UDevInputs* DevInputs = UDevInputs::Get(this);
	if (!DevInputs)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevInputs is missing. DevPad shortcuts could not be bound"));
		return;
	}

	const UDevPadSettings* Settings = GetDefault<UDevPadSettings>();
	if (!Settings)
	{
		UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("DevPad settings are missing. DevPad shortcuts could not be bound"));
		return;
	}

	if (Settings->PadShortcut.IsNone())
	{
		UE_LOG_FUNCTION(LogDevPad, Log, TEXT("DevPad shortcut is undefined"));
		return;
	}

	UE_LOG_FUNCTION(LogDevPad, Verbose, TEXT("DevPad bound to input shortcut: %s"), *Settings->PadShortcut.ToString());
	DevInputs->BindShortcut(Settings->PadShortcut).Delegate.BindDelegate(this, &ThisClass::OnShortcutTriggered);
}

void UDevPad::UnbindShortcut() const
{
	if (const UDevInputs* DevInputs = UDevInputs::Get(this))
	{
		DevInputs->ClearBindingsForObject(this);
	}
}

void UDevPad::OnShortcutTriggered()
{
	TogglePad();
}

namespace DevPad::Console
{
	template <class TSubsystemClass = UDevCoreGameInstanceSubsystem>
	struct TAutoConsoleCommandWithGameInstanceSubsystemAndArgs : private FAutoConsoleCommandWithWorldAndArgs
	{
		DECLARE_DELEGATE_ThreeParams(FDelegate, const TArray<FString>&, UWorld*, TSubsystemClass*);

		TAutoConsoleCommandWithGameInstanceSubsystemAndArgs(const TCHAR* Name, const TCHAR* Help, const FDelegate& Command, uint32 Flags = ECVF_Default)
			: FAutoConsoleCommandWithWorldAndArgs(Name, Help,
				FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([Delegate = Command](const TArray<FString>& Args, UWorld* World)
				{
					if (const UGameInstance* GameInstance = World->GetGameInstance())
					{
						if (TSubsystemClass* Subsystem = GameInstance->GetSubsystem<TSubsystemClass>())
						{
							Delegate.ExecuteIfBound(Args, World, Subsystem);
						}
						else
						{
							UE_LOG_FUNCTION(LogDevPad, Warning, TEXT("Required game instance subsystem is missing. Command could not be executed: Subsystem = %s"), *GetNameSafe(TSubsystemClass::StaticClass()));
						}
					}
				}), Flags) {}
	};

	struct FAutoConsoleCommandWithWorldDevPadAndArgs : TAutoConsoleCommandWithGameInstanceSubsystemAndArgs<UDevPad>
	{
		FAutoConsoleCommandWithWorldDevPadAndArgs(const TCHAR* Name, const TCHAR* Help, const FDelegate& Command, uint32 Flags = ECVF_Default)
			: TAutoConsoleCommandWithGameInstanceSubsystemAndArgs(Name, Help, Command, Flags) {}
	};

	FAutoConsoleCommandWithWorldDevPadAndArgs ShowPadCommand(TEXT("DevHub.Pad.ShowPad"), TEXT("Shows the DevPad, optionally opening a specified page if provided."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			if (Args.IsEmpty())
			{
				DevPad->ShowPad();
			}
			else
			{
				DevPad->ShowPad(FName(*Args[0]));
			}
		}));

	FAutoConsoleCommandWithWorldDevPadAndArgs HideMenuCommand(TEXT("DevHub.Pad.HidePad"), TEXT("Hides the DevPad."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			DevPad->HidePad();
		}));

	FAutoConsoleCommandWithWorldDevPadAndArgs NavigateToPreviousPage(TEXT("DevHub.Pad.PreviousPage"), TEXT("Navigate to previous DevPad page."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			DevPad->NavigateToPreviousPage();
		}));

	FAutoConsoleCommandWithWorldDevPadAndArgs NavigateToNextPage(TEXT("DevHub.Pad.NextPage"), TEXT("Navigate to next DevPad page."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			DevPad->NavigateToNextPage();
		}));

	FAutoConsoleCommandWithWorldDevPadAndArgs CloseTopSubPage(TEXT("DevHub.Pad.CloseCurrentSubPage"), TEXT("Close current DevPad sub-page."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			DevPad->CloseCurrentSubPage();
		}));

	FAutoConsoleCommandWithWorldDevPadAndArgs CloseAllSubPages(TEXT("DevHub.Pad.CloseAllSubPages"), TEXT("Close all DevPad sub-pages."),
		FAutoConsoleCommandWithWorldDevPadAndArgs::FDelegate::CreateLambda([](const TArray<FString>& Args, const UWorld* World, const UDevPad* DevPad)
		{
			DevPad->CloseAllSubPages();
		}));
} // DevPad::Console
