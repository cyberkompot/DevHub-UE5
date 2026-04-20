// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"
#include UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH

#include "Framework/Commands/UICommandInfo.h"
#include "Styling/SlateTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectMacros.h"
#include "DevActionTypes.generated.h"

struct FDevAction;
class UDevActionScript;

using FDevActionInstancedAction = FInstancedStruct;
using FDevActionInstancedActions = TArray<FDevActionInstancedAction>;

namespace DevAction::Setting
{
	extern bool GShowInvalidActions;
}

/**
 * Base struct for building actions.
 * Inheriting classes should provide the logic for action execution.
 */
USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Action", Meta = (Hidden))
struct FDevAction
{
	GENERATED_BODY()

	virtual ~FDevAction() = default;

	FORCEINLINE TAttribute<FText> GetActionLabel(const UObject* WorldContextObject) const { return OnGetActionLabel(WorldContextObject); }
	FORCEINLINE ECheckBoxState GetActionCheckState(const UObject* WorldContextObject) const { return OnGetActionCheckState(WorldContextObject); }
	FORCEINLINE bool GetActionVisibility(const UObject* WorldContextObject) const { return OnGetActionVisibility(WorldContextObject); }
	FORCEINLINE EUserInterfaceActionType GetActionUserInterfaceType(const UObject* WorldContextObject) const { return OnGetActionUserInterfaceType(WorldContextObject); }

	/** Executes action exclusively in development builds and handles exceptions not handled by the implementation. */
	DEVACTIONS_API void ExecuteAction(const UObject* WorldContextObject) const;

protected:
	DEVACTIONS_API virtual TAttribute<FText> OnGetActionLabel(const UObject* WorldContextObject) const { return TAttribute<FText>(); }
	DEVACTIONS_API virtual ECheckBoxState OnGetActionCheckState(const UObject* WorldContextObject) const;
	DEVACTIONS_API virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const;
	DEVACTIONS_API virtual EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const;

	/** Method for implementing an action execution. */
	DEVACTIONS_API virtual void OnExecuteAction(const UObject* WorldContextObject) const {};
};

USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Action")
struct FDevActionNone : public FDevAction
{
	GENERATED_BODY()

protected:
	//~ Begin FDevAction interface.
	virtual EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const override { return EUserInterfaceActionType::None; }
	DEVACTIONS_API virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override { return false; }
	//~ End FDevAction interface.
};

USTRUCT(BlueprintType, Blueprintable, Category = "DevHub|Action", Meta = (Hidden))
struct FDevActionBase : public FDevAction
{
	GENERATED_BODY()

	/** Action label. */
	UPROPERTY(EditAnywhere, Category = "Dev Action")
	FText Label;

protected:
	//~ Begin FDevAction interface.
	virtual TAttribute<FText> OnGetActionLabel(const UObject* WorldContextObject) const override { return Label; }
	//~ End FDevAction interface.
};


/**
 * Executes the specified Blueprint-implemented action.
 */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Dev Action Script")
struct FDevActionObject : public FDevAction
{
	GENERATED_BODY()

	/** The Blueprint in which the function with the specified name is to be executed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Action")
	TSoftClassPtr<UDevActionScript> ActionScript;

protected:
	//~ Begin FDevAction interface.
	virtual ECheckBoxState OnGetActionCheckState(const UObject* WorldContextObject) const override;
	virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override;
	virtual EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const override;
	virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	//~ Begin FDevAction interface.

private:
	enum class EGetDefaultObjectErrorMode
	{
		ReturnNull = 0, // Silently returns nullptr, the calling code is expected to handle this gracefully.
		LogAndReturnNull = 1, // Raises a runtime warning but still returns nullptr, the calling code is expected to handle this gracefully.
	};

	UDevActionScript* GetActionScriptDefaultObject(const EGetDefaultObjectErrorMode InErrorMode) const;
};

/**
 * Executes a set of actions, typically used when multiple actions need to be assigned to a single command.
 */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Action", DisplayName = "Dev Actions Set")
struct FDevActionsSet : public FDevActionBase
{
	GENERATED_BODY()

	/** Set of actions that need to be executed. */
	UPROPERTY(EditAnywhere, Category = "Dev Action", Meta = (BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct))
	TArray<FInstancedStruct> Actions;

protected:
	//~ Begin FDevAction interface.
	DEVACTIONS_API virtual ECheckBoxState OnGetActionCheckState(const UObject* WorldContextObject) const override;
	DEVACTIONS_API virtual bool OnGetActionVisibility(const UObject* WorldContextObject) const override;
	DEVACTIONS_API virtual EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const override;
	DEVACTIONS_API virtual void OnExecuteAction(const UObject* WorldContextObject) const override;
	//~ Begin FDevAction interface.
};


/**
 * Base class for Blueprint-implemented actions.
 * Inheriting Blueprint should provide the logic for action execution.
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable, Abstract, Category = "DevHub|Action", Meta = (LoadBehavior = "LazyOnDemand"))
class UDevActionScript : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	DEVACTIONS_API ECheckBoxState GetActionCheckState(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintCallable)
	DEVACTIONS_API bool GetActionVisibility(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintCallable)
	DEVACTIONS_API EUserInterfaceActionType GetActionUserInterfaceType(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintCallable)
	DEVACTIONS_API void ExecuteAction(const UObject* WorldContextObject) const;

	//~ Begin UObject interface.
	virtual UWorld* GetWorld() const override { return World; }
	//~ Begin UObject interface.

protected:
	friend struct FDevActionObject;

	UFUNCTION(BlueprintImplementableEvent, Meta = (WorldContext = "WorldContextObject", DisplayName = "On Get Action Check State", ScriptName = "OnGetActionCheckState"))
	DEVACTIONS_API ECheckBoxState OnGetActionCheckState(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintImplementableEvent, Meta = (WorldContext = "WorldContextObject", DisplayName = "On Get Action Visibility", ScriptName = "OnGetActionVisibility"))
	DEVACTIONS_API bool OnGetActionVisibility(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintImplementableEvent, Meta = (WorldContext = "WorldContextObject", DisplayName = "On Get Action User Interface Type", ScriptName = "OnGetActionUserInterfaceType"))
	DEVACTIONS_API EUserInterfaceActionType OnGetActionUserInterfaceType(const UObject* WorldContextObject) const;

	/** Method for implementing an action execution in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (WorldContext = "WorldContextObject", DisplayName = "On Execute Action", ScriptName = "OnExecuteAction"))
	DEVACTIONS_API void OnExecuteAction(const UObject* WorldContextObject) const;

private:
	UPROPERTY(Transient, SkipSerialization)
	mutable UWorld* World;
};
