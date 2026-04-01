// Copyright (c) Alexandr Pereverzev.

#include "Actions/DevActionConsoleVariable.h"

#include "DevActionConsoleLibrary.h"
#include "DevActionLogging.h"

IConsoleVariable* FDevActionConsoleVariableView::GetCVar() const
{
	return FDevActionConsoleLibrary::Get().FindConsoleVariable(CVarName);
}

bool FDevActionConsoleVariableView::IsActionEnabled(const UObject* WorldContextObject) const
{
#if !NO_CVARS

	const IConsoleVariable* CVar = GetCVar();
	if (!CVar) { return false; }

#if UE_COMPATIBILITY_SUPPORTED_CONSOLE_VARIABLE_IS_ENABLED
	return CVar->IsEnabled();
#else
	return true;
#endif // UE_COMPATIBILITY_SUPPORTED_CONSOLE_VARIABLE_IS_ENABLED

#else
	return false;
#endif // !NO_CVARS
}

bool FDevActionConsoleVariableView::IsActionVisible(const UObject* WorldContextObject) const
{
#if !NO_CVARS

	const IConsoleVariable* CVar = GetCVar();
	return (CVar || DevAction::Setting::GShowInvalidActions);

#else
	return false;
#endif // !NO_CVARS
}

ECheckBoxState FDevActionConsoleVariableView::GetActionCheckState(const UObject* WorldContextObject) const
{
#if !NO_CVARS

	const IConsoleVariable* CVar = GetCVar();
	if (!CVar) { return ECheckBoxState::Unchecked; }

	bool bChecked = false;
	if (CVarValue.IsEmpty())
	{
		bChecked = (CVar->IsVariableString()) ? CVar->GetString().IsEmpty() : CVar->GetBool();
	}
	else if (CVar->IsVariableBool())
	{
		bChecked = (CVar->GetBool() == FCString::ToBool(*CVarValue));
	}
	else if (CVar->IsVariableInt())
	{
		int32 ParsedInt = 0;
		bChecked = LexTryParseString(ParsedInt, *CVarValue) && (CVar->GetInt() == ParsedInt);
	}
	else if (CVar->IsVariableFloat())
	{
		float ParsedFloat = 0.0f;
		bChecked = LexTryParseString(ParsedFloat, *CVarValue) && FMath::IsNearlyEqual(CVar->GetFloat(), ParsedFloat);
	}
	else
	{
		bChecked = CVar->GetString().Equals(CVarValue, ESearchCase::IgnoreCase);
	}

	return (bChecked) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;

#else
	return false;
#endif // !NO_CVARS
}

EUserInterfaceActionType FDevActionConsoleVariableView::GetActionUserInterfaceType(const UObject* WorldContextObject) const
{
	switch (UserInterfaceType)
	{
		case EDevActionCVarUserInterfaceType::ToggleButton: return EUserInterfaceActionType::ToggleButton;
		case EDevActionCVarUserInterfaceType::RadioButton:  return EUserInterfaceActionType::RadioButton;
		case EDevActionCVarUserInterfaceType::AutoDetect:
		default:
			const IConsoleVariable* CVar = GetCVar();
			return (CVar) ? (CVar->IsVariableBool()) ? EUserInterfaceActionType::ToggleButton : EUserInterfaceActionType::RadioButton : EUserInterfaceActionType::Button;
	}
}

FStringView FDevActionConsoleVariableView::GetActionToolTip() const
{
#if !NO_CVARS

	const IConsoleVariable* CVar = GetCVar();
	return (CVar) ? FStringView(CVar->GetHelp()) : FStringView();

#else
	return FStringView();
#endif // !NO_CVARS
}

void FDevActionConsoleVariableView::ExecuteAction(const UObject* WorldContextObject) const
{
#if !NO_CVARS

	if (CVarName.IsEmpty())
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("CVar is not defined"));
		return;
	}

	IConsoleVariable* CVar = GetCVar();
	if (!CVar)
	{
		UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("CVar is missing: %s"), *CVarName);
		return;
	}

	const bool bTreatCVarAsBool = (UserInterfaceType == EDevActionCVarUserInterfaceType::ToggleButton)
		|| ((UserInterfaceType == EDevActionCVarUserInterfaceType::AutoDetect) && (CVar && CVar->IsVariableBool()));
	if (bTreatCVarAsBool)
	{
		CVar->Set(!CVar->GetBool(), ECVF_SetByConsole);
	}
	else
	{
		CVar->Set(*CVarValue, ECVF_SetByConsole);
	}

#else

	UE_LOG_FUNCTION(LogDevActions, Warning, TEXT("Console variables are disabled in this build"));

#endif // !NO_CVARS
}
