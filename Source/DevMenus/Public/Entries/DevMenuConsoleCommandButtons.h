// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"
#include "Actions/DevActionConsoleCommand.h"
#include "DevMenuConsoleCommandButtons.generated.h"

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Command Button"))
struct DEVMENUS_API FDevMenuConsoleCommandButton : public FDevMenuExecutionBase
{
	GENERATED_BODY()

	/** Console command to execute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "InputShortcut"))
	FString Command;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }
	virtual TAttribute<FText> GetToolTip() const override { return (Command.IsEmpty()) ? TAttribute<FText>() : TAttribute<FText>(FText::FromStringView(FDevActionConsoleCommandView(Command).GetActionToolTip())); }
	virtual bool IsEnabled(const UObject* WorldContextObject) const override { return FDevActionConsoleCommandView(Command).IsActionEnabled(WorldContextObject); }
	virtual bool IsVisible(const UObject* WorldContextObject) const override { return FDevActionConsoleCommandView(Command).IsActionVisible(WorldContextObject); }
	virtual void ExecuteEntry(const UObject* WorldContextObject) override { return FDevActionConsoleCommandView(Command).ExecuteAction(WorldContextObject); }
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuExecutionBase;
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Command Argument Info"))
struct DEVMENUS_API FDevMenuConsoleCommandArgumentInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	FString Argument;

	/** Label format for Console Variable value. Supports {Variable} and {Value} placeholders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	FText LabelOverride;
};

USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Console Command Group"))
struct DEVMENUS_API FDevMenuConsoleCommandGroup : public FDevMenuDynamicOuterWithLayoutBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "Layout"))
	FString Command;

	/** Label format for Console Variable values entries. Supports {Command} and {Argument} placeholders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", Meta = (DisplayAfter = "Command"))
	FText LabelFormat = INVTEXT("{Argument}");

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "LabelFormat"))
	TArray<FDevMenuConsoleCommandArgumentInfo> CommandArgumentsInfos;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }
	virtual TAttribute<FText> GetToolTip() const override { return (Command.IsEmpty()) ? TAttribute<FText>() : TAttribute<FText>(FText::FromStringView(FDevActionConsoleCommandView(Command).GetActionToolTip())); }
	virtual bool IsEnabled(const UObject* WorldContextObject) const override { return FDevActionConsoleCommandView(Command).IsActionEnabled(WorldContextObject); }
	virtual bool IsVisible(const UObject* WorldContextObject) const override { return FDevActionConsoleCommandView(Command).IsActionVisible(WorldContextObject); }
	//~ End IDevMenuEntry interface.

protected:
	//~ Begin FDevMenuDynamicOuterBase interface.
	virtual FDevMenuInstancedEntries CreateDynamicEntries() const override;
	//~ End FDevMenuDynamicOuterBase interface.

private:
	using Super = FDevMenuDynamicOuterWithLayoutBase;

	FText FormatEntryLabel(const FDevMenuConsoleCommandArgumentInfo& CVarValueInfo) const;
};
