// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevActionTypes.h"
#include "DevActionConsoleCommand.generated.h"

struct DEVACTIONS_API FDevActionConsoleCommandView
{
	explicit FDevActionConsoleCommandView(const FString& InCommand)
		: Command(InCommand) {}

	const FString& Command;

	/** Resolve the target CCommand. Nullptr if Command is not defined or CCommand is missing. */
	IConsoleCommand* GetCCommand() const;

	bool IsActionEnabled(const UObject* WorldContextObject) const;
	bool IsActionVisible(const UObject* WorldContextObject) const;
	FStringView GetActionToolTip() const;

	void ExecuteAction(const UObject* WorldContextObject) const;
};

/**
 * Executes the specified Console Command.
 */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Console Command")
struct DEVACTIONS_API FDevActionConsoleCommand : public FDevAction
{
	GENERATED_BODY()

	FDevActionConsoleCommand() = default;
	explicit FDevActionConsoleCommand(const FString& InCommand)
		: Command(InCommand) {}
	explicit FDevActionConsoleCommand(const FString& InCommand, const FString& InArg)
		: Command((InArg.IsEmpty()) ? InCommand : InCommand + TEXT(" ") + InArg) {}

	/** Console command to execute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	FString Command;

protected:
	//~ Begin FDevAction interface.
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return FDevActionConsoleCommandView(Command).IsActionVisible(WorldContextObject); }
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override { FDevActionConsoleCommandView(Command).ExecuteAction(WorldContextObject); }
	//~ End FDevAction interface.
};
