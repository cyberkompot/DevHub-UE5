// Copyright (c) Alexandr Pereverzev.

#include "DevActionTypes.h"

#include "DevActionLogging.h"
#include "Engine/Engine.h"

namespace DevAction::Setting
{
	bool GShowInvalidActions = false;
	FAutoConsoleVariableRef CVarShowInvalidActions(TEXT("DevHub.Action.Settings.ShowInvalidActions"), GShowInvalidActions, TEXT("Show invalid Dev Actions (for example associated with Console Commands and Variables from plugins that are not installed or from future engine versions)"));
}

void FDevAction::ExecuteAction(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE

	try
	{
		OnExecuteAction(WorldContextObject);
	}
	catch (const std::exception& e)
	{
		UE_LOG_FUNCTION(LogDevActions, Error, TEXT("Unhandled exception encountered during action execution: %hs"), e.what());
	}
	catch (...)
	{
		UE_LOG(LogDevActions, Error, TEXT("Unhandled exception encountered during action execution"));
	}

#endif // DEV_HUB_AVAILABLE
}

ECheckBoxState FDevAction::OnGetActionCheckState(const UObject* WorldContextObject) const
{
	return ECheckBoxState::Unchecked;
}

bool FDevAction::OnGetActionVisibility(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE
	return true;
#else
	return false;
#endif // DEV_HUB_AVAILABLE
}

EUserInterfaceActionType FDevAction::OnGetActionUserInterfaceType(const UObject* WorldContextObject) const
{
	return EUserInterfaceActionType::Button;
}


ECheckBoxState FDevActionObject::OnGetActionCheckState(const UObject* WorldContextObject) const
{
	const UDevActionScript* ActionScriptCDO = GetActionScriptDefaultObject(EGetDefaultObjectErrorMode::ReturnNull);
	return (ActionScriptCDO) ? ActionScriptCDO->GetActionCheckState(WorldContextObject) : Super::OnGetActionCheckState(WorldContextObject);
}

bool FDevActionObject::OnGetActionVisibility(const UObject* WorldContextObject) const
{
	const UDevActionScript* ActionScriptCDO = GetActionScriptDefaultObject(EGetDefaultObjectErrorMode::LogAndReturnNull);
	return (ActionScriptCDO) ? ActionScriptCDO->GetActionVisibility(WorldContextObject) : Super::OnGetActionVisibility(WorldContextObject);
}

EUserInterfaceActionType FDevActionObject::OnGetActionUserInterfaceType(const UObject* WorldContextObject) const
{
	const UDevActionScript* ActionScriptCDO = GetActionScriptDefaultObject(EGetDefaultObjectErrorMode::ReturnNull);
	return (ActionScriptCDO) ? ActionScriptCDO->GetActionUserInterfaceType(WorldContextObject) : Super::OnGetActionUserInterfaceType(WorldContextObject);
}

void FDevActionObject::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (const UDevActionScript* ActionScriptCDO = GetActionScriptDefaultObject(EGetDefaultObjectErrorMode::LogAndReturnNull))
	{
		ActionScriptCDO->ExecuteAction(WorldContextObject);
	}
}

UDevActionScript* FDevActionObject::GetActionScriptDefaultObject(const EGetDefaultObjectErrorMode InErrorMode) const
{
	if (ActionScript.IsNull())
	{
		if (InErrorMode >= EGetDefaultObjectErrorMode::LogAndReturnNull)
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Action Script is not defined"));
		}
		return nullptr;
	}

	const TSubclassOf<UDevActionScript> ActionScriptClass = ActionScript.LoadSynchronous();
	if (!ActionScriptClass)
	{
		if (InErrorMode >= EGetDefaultObjectErrorMode::LogAndReturnNull)
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Action Script asset is missing"));
		}
		return nullptr;
	}

	UDevActionScript* ActionScriptCDO = ActionScriptClass->GetDefaultObject<UDevActionScript>();
	if (!ActionScriptCDO)
	{
		if (InErrorMode >= EGetDefaultObjectErrorMode::LogAndReturnNull)
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Action Script class default object is missing"));
		}
		return nullptr;
	}

	return ActionScriptCDO;
}


ECheckBoxState UDevActionScript::GetActionCheckState(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE

	TGuardValue WorldGuard(World, GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull));
	try
	{
		return OnGetActionCheckState(WorldContextObject);
	}
	catch (const std::exception& e)
	{
		UE_LOG_FUNCTION(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action check state: %hs"), e.what());
	}
	catch (...)
	{
		UE_LOG(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action check state"));
	}

#endif // DEV_HUB_AVAILABLE

	return ECheckBoxState::Unchecked;
}

bool UDevActionScript::GetActionVisibility(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE

	if (!FindFunction(GET_FUNCTION_NAME_CHECKED(ThisClass, OnGetActionVisibility))) { return true; }

	TGuardValue WorldGuard(World, GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull));
	try
	{
		return OnGetActionVisibility(WorldContextObject);
	}
	catch (const std::exception& e)
	{
		UE_LOG_FUNCTION(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action check state: %hs"), e.what());
	}
	catch (...)
	{
		UE_LOG(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action check state"));
	}

#endif // DEV_HUB_AVAILABLE

	return false;
}

EUserInterfaceActionType UDevActionScript::GetActionUserInterfaceType(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE

	if (!FindFunction(GET_FUNCTION_NAME_CHECKED(ThisClass, OnGetActionUserInterfaceType))) { return EUserInterfaceActionType::Button; }

	TGuardValue WorldGuard(World, GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull));
	try
	{
		return OnGetActionUserInterfaceType(WorldContextObject);
	}
	catch (const std::exception& e)
	{
		UE_LOG_FUNCTION(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action user interface type: %hs"), e.what());
	}
	catch (...)
	{
		UE_LOG(LogDevActions, Error, TEXT("Unhandled exception encountered during getting action user interface type"));
	}

#endif // DEV_HUB_AVAILABLE

	return EUserInterfaceActionType::Button;
}

void UDevActionScript::ExecuteAction(const UObject* WorldContextObject) const
{
#ifdef DEV_HUB_AVAILABLE

	TGuardValue WorldGuard(World, GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull));
	try
	{
		OnExecuteAction(WorldContextObject);
	}
	catch (const std::exception& e)
	{
		UE_LOG_FUNCTION(LogDevActions, Error, TEXT("Unhandled exception encountered during action execution: %hs"), e.what());
	}
	catch (...)
	{
		UE_LOG(LogDevActions, Error, TEXT("Unhandled exception encountered during action execution"));
	}

#endif // DEV_HUB_AVAILABLE
}


ECheckBoxState FDevActionsSet::OnGetActionCheckState(const UObject* WorldContextObject) const
{
	if (Actions.IsEmpty()) { return ECheckBoxState::Unchecked; }

	int32 UncheckedCount = 0, CheckedCount = 0, UndeterminedCount = 0;
	for (int32 i = 0; i < Actions.Num() && UncheckedCount == 0 && CheckedCount == 0 && UndeterminedCount == 0; ++i)
	{
		if (const FDevActionInstancedAction& Action = Actions[i]; Action.IsValid())
		{
			switch (Action.Get<FDevAction>().GetActionCheckState(WorldContextObject))
			{
				case ECheckBoxState::Unchecked: ++UncheckedCount; break;
				case ECheckBoxState::Checked: ++CheckedCount; break;
				case ECheckBoxState::Undetermined:
				default: ++UndeterminedCount; break;
			}
		}
	}

	// Default return state is Unchecked, which makes it inconvenient to determine the overall state for a set of actions.
	// We treat Checked and Undetermined states with higher priority and derive the summary state based on their combination, ignoring the Unchecked state.
	if (CheckedCount)
	{
		return ECheckBoxState::Checked;
	}
	else if (UndeterminedCount)
	{
		return ECheckBoxState::Undetermined;
	}
	else
	{
		return ECheckBoxState::Unchecked;
	}
}

bool FDevActionsSet::OnGetActionVisibility(const UObject* WorldContextObject) const
{
	if (Actions.IsEmpty()) { return true; }

	// At least one sub-action in the set must be visible for the main action to be visible.
	bool bVisibility = false;
	for (int32 i = 0; i < Actions.Num() && !bVisibility; ++i)
	{
		if (const FDevActionInstancedAction& Action = Actions[i]; Action.IsValid())
		{
			bVisibility |= Action.Get<FDevAction>().GetActionVisibility(WorldContextObject);
		}
	}
	return bVisibility;
}

EUserInterfaceActionType FDevActionsSet::OnGetActionUserInterfaceType(const UObject* WorldContextObject) const
{
	if (Actions.IsEmpty()) { return EUserInterfaceActionType::Button; }

	// Priority order is open for reconsideration and should remain consistent with OnGetActionCheckState.
	static const TArray<EUserInterfaceActionType, TFixedAllocator<6>> UserInterfaceActionTypePriorities
	{
		EUserInterfaceActionType::None,
		EUserInterfaceActionType::Button,
		EUserInterfaceActionType::CollapsedButton,
		EUserInterfaceActionType::Check,
		EUserInterfaceActionType::RadioButton,
		EUserInterfaceActionType::ToggleButton,
	};

	int32 MaxPriority = INDEX_NONE;
	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		if (const FDevActionInstancedAction& Action = Actions[i]; Action.IsValid())
		{
			MaxPriority = FMath::Max(MaxPriority, UserInterfaceActionTypePriorities.IndexOfByKey(Action.Get<FDevAction>().GetActionUserInterfaceType(WorldContextObject)));
		}
	}
	const EUserInterfaceActionType UserInterfaceActionType = (MaxPriority == INDEX_NONE)
		? EUserInterfaceActionType::Button
		: UserInterfaceActionTypePriorities[MaxPriority];

	return UserInterfaceActionType;
}

void FDevActionsSet::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (Actions.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Actions are not defined"));
		return;
	}

	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		if (const FDevActionInstancedAction& Action = Actions[i]; Action.IsValid())
		{
			Action.Get<FDevAction>().ExecuteAction(WorldContextObject);
		}
		else
		{
			UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Action is not defined at position %d. Remaining actions will be executed normally"), i);
		}
	}
}


TAttribute<FText> FDevActionsStack::OnGetActionLabel(const UObject* WorldContextObject) const
{
	if (const FDevAction* Action = GetFirstVisibleAction(WorldContextObject))
	{
		if (TAttribute<FText> LabelAttribute = Action->GetActionLabel(WorldContextObject); LabelAttribute.IsSet()) { return MoveTemp(LabelAttribute); }
	}
	return FDevActionBase::OnGetActionLabel(WorldContextObject);
}

ECheckBoxState FDevActionsStack::OnGetActionCheckState(const UObject* WorldContextObject) const
{
	if (const FDevAction* Action = GetFirstVisibleAction(WorldContextObject)) { return Action->GetActionCheckState(WorldContextObject); }
	return ECheckBoxState::Undetermined;
}

bool FDevActionsStack::OnGetActionVisibility(const UObject* WorldContextObject) const
{
	return !!GetFirstVisibleAction(WorldContextObject);
}

EUserInterfaceActionType FDevActionsStack::OnGetActionUserInterfaceType(const UObject* WorldContextObject) const
{
	if (const FDevAction* Action = GetFirstVisibleAction(WorldContextObject)) { return Action->GetActionUserInterfaceType(WorldContextObject); }
	return EUserInterfaceActionType::None;
}

void FDevActionsStack::OnExecuteAction(const UObject* WorldContextObject) const
{
	if (const FDevAction* Action = GetFirstVisibleAction(WorldContextObject)) { Action->ExecuteAction(WorldContextObject); }
}

const FDevAction* FDevActionsStack::GetFirstVisibleAction(const UObject* WorldContextObject) const
{
	for (const FInstancedStruct& InstancedAction : Actions)
	{
		if (const FDevAction* Action = InstancedAction.GetPtr<FDevAction>(); Action && Action->GetActionVisibility(WorldContextObject)) { return Action; }
	}
	return nullptr;
}
