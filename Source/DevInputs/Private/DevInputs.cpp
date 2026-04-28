// Copyright (c) Alexandr Pereverzev.

#include "DevInputs.h"

#include "DevInputManager.h"
#include "DevInputLogging.h"
#include "DevInputMode.h"
#include "DevInputProcessor.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

class FDevInputsModule;
using namespace DevInput::Logging;


UDevInputs::UDevInputs()
{
	InputManager = MakePimpl<FDevInputManager>();
	InputMode = MakePimpl<FDevInputMode>();
	InputProcessor = MakeShared<FDevInputProcessor>();

	InputManager->SetWorldContextObject(this);
}

UDevInputs* UDevInputs::Get(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<ThisClass>();
		}
	}
	return nullptr;
}

void UDevInputs::Initialize(FSubsystemCollectionBase& Collection)
{
	UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Initializing Dev Inputs"));

	InputMode->SetGameInstance(GetGameInstance());
	InputProcessor->OnInputEvent.AddRaw(InputManager.Get(), &FDevInputManager::OnInputEvent);

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
		UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Dev Input process registered"));
	}
	else
	{
		UE_LOG_FUNCTION(LogDevInputs, Error, TEXT("Slate application is not initialized, Dev Input process not registered, and Dev Input will not be handled"));
	}
}

void UDevInputs::Deinitialize()
{
	UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Deinitializing Dev Inputs"));

	InputManager->Dispose();
	InputMode->Dispose();
	InputProcessor->Reset();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		UE_LOG_FUNCTION(LogDevInputs, Verbose, TEXT("Dev Input process unregistered"));
	}
}

bool UDevInputs::ShouldCreateSubsystem(UObject* Outer) const
{
#ifdef DEV_HUB_AVAILABLE

	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	if (!GIsClient) { return false; }
	if (IsRunningCommandlet() || IsRunningDedicatedServer()) { return false; }

	if (!FSlateApplication::IsInitialized()) { return false; }

	const UWorld* World = Outer->GetWorld();
	if (!World) { return false; }

	const TEnumAsByte<EWorldType::Type> WorldType = World->WorldType;
	if (WorldType != EWorldType::Game && WorldType != EWorldType::PIE) { return false; }

	return true;

#else // DEV_HUB_AVAILABLE

	return false;

#endif // DEV_HUB_AVAILABLE
}

void UDevInputs::BeginDestroy()
{
	Deinitialize();
	Super::BeginDestroy();
}

void UDevInputs::AddBinding(TUniquePtr<FDevInputBinding>&& InBindingPtr) const
{
	InputManager->AddBinding(MoveTemp(InBindingPtr));
}

const TArray<TUniquePtr<FDevInputBinding>>& UDevInputs::GetBindings() const
{
	return InputManager->GetBindings();
}

bool UDevInputs::RemoveBinding(const FDevInputBinding& InBinding) const
{
	return InputManager->RemoveBinding(InBinding);
}

bool UDevInputs::RemoveBindingByHandle(const uint32 InHandle) const
{
	return InputManager->RemoveBindingByHandle(InHandle);
}

void UDevInputs::RemoveBindingAt(const int32 InIndex) const
{
	InputManager->RemoveBindingAt(InIndex);
}

void UDevInputs::ClearBindingsForObject(const void* InObject) const
{
	InputManager->ClearBindingsForObject(InObject);
}

void UDevInputs::ClearBindingsForOwner(const FDevInputOwner& InOwner) const
{
	InputManager->ClearBindingsForOwner(InOwner);
}

void UDevInputs::ClearBindings() const
{
	InputManager->ClearBindings();
}

EDevInputType UDevInputs::GetCurrentInputType() const
{
	return InputProcessor->CurrentInputType;
}

void UDevInputs::ConsumeInputEvent() const
{
	InputProcessor->ConsumeInputEvent();
}

void UDevInputs::EmulateKeyPress(const FKey& InKey) const
{
	InputProcessor->EmulateKeyPress(InKey);
}

FDevInputEvent& UDevInputs::OnInputEvent() const
{
	return InputProcessor->OnInputEvent;
}

void UDevInputs::SetInputMode(const FInputModeDataBase& InData) const
{
	InputMode->SetInputMode(InData);
}

void UDevInputs::RestoreInputMode() const
{
	InputMode->RestoreInputMode();
}

FDevInputTypeChanged& UDevInputs::OnInputTypeChanged() const
{
	return InputProcessor->OnInputTypeChanged;
}
