// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "Actions/DevActionConsoleVariable.h"
#include "DevMenuConsoleVariableButtons.generated.h"

USTRUCT(BlueprintType, Category = "DevHub", DisplayName = "Console Variable")
struct DEVMENUS_API FTest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Test;
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Variable Button"))
struct DEVMENUS_API FDevMenuConsoleVariableButton : public FDevMenuExecutionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ShowOnlyInnerProperties))
	FTest Action;

	/** Console variable name (e.g., "r.Fog", "t.MaxFPS"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "InputShortcut"))
	FString CVarName;

	/** Target console variable value. Ignored for boolean CVars, which are toggled instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "CVarName"))
	FString CVarValue;

	/** Preferred UI control for the Console variable. Auto Detect falls back to Toggle Button for boolean CVars and Radio Button for other types. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "CVarValue"))
	EDevActionCVarUserInterfaceType UserInterfaceType = EDevActionCVarUserInterfaceType::AutoDetect;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }
	virtual TAttribute<FText> GetToolTip() const override { return (CVarName.IsEmpty()) ? TAttribute<FText>() : TAttribute<FText>(FText::FromStringView(FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).GetActionToolTip())); }
	virtual ECheckBoxState GetCheckState(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).GetActionCheckState(WorldContextObject); }
	virtual bool IsEnabled(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).IsActionEnabled(WorldContextObject); }
	virtual bool IsVisible(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).IsActionVisible(WorldContextObject); }
	virtual void ExecuteEntry(const UObject* WorldContextObject) override { FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).ExecuteAction(WorldContextObject); }
	//~ End IDevMenuEntry interface.

	//~ Begin FDevMenuExecutionBase interface.
	virtual EUserInterfaceActionType GetUserInterfaceType(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, CVarValue, UserInterfaceType).GetActionUserInterfaceType(WorldContextObject); }
	//~ End FDevMenuExecutionBase interface.

private:
	using Super = FDevMenuExecutionBase;
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Variable Value Info"))
struct DEVMENUS_API FDevMenuConsoleVariableValueInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	FString CVarValue;

	/** Label format for Console Variable value. Supports {Variable} and {Value} placeholders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	FText LabelOverride;
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Variable Group"))
struct DEVMENUS_API FDevMenuConsoleVariableGroup : public FDevMenuDynamicOuterWithLayoutBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "Layout"))
	FString CVarName;

	/** Label format for Console Variable values entries. Supports {Variable} and {Value} placeholders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "CVarName"))
	FText LabelFormat = INVTEXT("{Value}");

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "LabelFormat"))
	TArray<FDevMenuConsoleVariableValueInfo> CVarValuesInfos;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }
	virtual TAttribute<FText> GetToolTip() const override { return (CVarName.IsEmpty()) ? TAttribute<FText>() : TAttribute<FText>(FText::FromStringView(FDevActionConsoleVariableView(CVarName, FString(), EDevActionCVarUserInterfaceType::ToggleButton).GetActionToolTip())); }
	virtual bool IsEnabled(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, FString(), EDevActionCVarUserInterfaceType::ToggleButton).IsActionEnabled(WorldContextObject); }
	virtual bool IsVisible(const UObject* WorldContextObject) const override { return FDevActionConsoleVariableView(CVarName, FString(), EDevActionCVarUserInterfaceType::ToggleButton).IsActionVisible(WorldContextObject); }
	//~ End IDevMenuEntry interface.

protected:
	//~ Begin FDevMenuDynamicOuterBase interface.
	virtual FDevMenuInstancedEntries CreateDynamicEntries() const override;
	//~ End FDevMenuDynamicOuterBase interface.

private:
	using Super = FDevMenuDynamicOuterWithLayoutBase;

	FText FormatEntryLabel(const FDevMenuConsoleVariableValueInfo& CVarValueInfo) const;
};
