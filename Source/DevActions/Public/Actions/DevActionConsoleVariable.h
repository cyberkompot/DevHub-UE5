// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "DevActionConsoleVariable.generated.h"

/**
 * Console Variable action UI controls.
 */
UENUM(BlueprintType, Category = "DevHub|Action")
enum struct EDevActionCVarUserInterfaceType : uint8
{
	/** UI control will be auto-detected base on CVar type. Toggle Button for booleans, and Radio Button for other types. */
	AutoDetect = 0,

	/** Prefers the CVar to be presented as a Toggle Button. Convenient for integer CVars that behave as zero–one toggles. */
	ToggleButton = 1,

	/** Prefers the CVar to be presented as a Radio Button. */
	RadioButton = 2,
};

struct DEVACTIONS_API FDevActionConsoleVariableView
{
	FDevActionConsoleVariableView(const FString& InCVarName, const FString& InCVarValue, const EDevActionCVarUserInterfaceType InUserInterfaceType = EDevActionCVarUserInterfaceType::AutoDetect)
		: CVarName(InCVarName), CVarValue(InCVarValue), UserInterfaceType(InUserInterfaceType) {}

	const FString& CVarName;
	const FString& CVarValue;
	const EDevActionCVarUserInterfaceType UserInterfaceType = EDevActionCVarUserInterfaceType::AutoDetect;

	/** Resolve the target CVar. Nullptr if CVarName is not defined or CVar is missing. */
	IConsoleVariable* GetCVar() const;

	bool IsActionEnabled(const UObject* WorldContextObject) const;
	bool IsActionVisible(const UObject* WorldContextObject) const;
	ECheckBoxState GetActionCheckState(const UObject* WorldContextObject) const;
	EUserInterfaceActionType GetActionUserInterfaceType(const UObject* WorldContextObject) const;
	FStringView GetActionToolTip() const;

	void ExecuteAction(const UObject* WorldContextObject) const;
};

/**
 * Sets the specified Console Variable.
 */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Console Variable")
struct DEVACTIONS_API FDevActionConsoleVariable : public FDevActionBase
{
	GENERATED_BODY()

	FDevActionConsoleVariable() = default;
	explicit FDevActionConsoleVariable(const FString& InCVarName, const FString& InCVarValue, const EDevActionCVarUserInterfaceType InUserInterfaceType = EDevActionCVarUserInterfaceType::AutoDetect)
		: CVarName(InCVarName), CVarValue(InCVarValue), UserInterfaceType(InUserInterfaceType) {}

	/** Console variable name (e.g., "r.Fog", "t.MaxFPS"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	FString CVarName;

	/** Target console variable value. Ignored for boolean CVars, which are toggled instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	FString CVarValue;

	/** Preferred UI control for the Console variable. Auto Detect falls back to Toggle Button for boolean CVars and Radio Button for other types. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	EDevActionCVarUserInterfaceType UserInterfaceType = EDevActionCVarUserInterfaceType::AutoDetect;

protected:
	//~ Begin FDevAction interface.
	virtual ECheckBoxState OnGetActionCheckState(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).GetActionCheckState(WorldContextObject); }
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).IsActionVisible(WorldContextObject); }
	virtual EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).GetActionUserInterfaceType(WorldContextObject); }
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override { FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).ExecuteAction(WorldContextObject); }
	//~ End FDevAction interface.
};
