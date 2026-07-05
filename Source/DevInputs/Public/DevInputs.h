// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"
#include "Engine/GameViewportClient.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DevInputs.generated.h"

struct FInputModeDataBase;
class FDevInputManager;
class FDevInputMode;
class FDevInputProcessor;

UCLASS(NotBlueprintable)
class DEVINPUTS_API UDevInputs final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Input")
	static UDevInputs* Get(const UObject* WorldContextObject);

public:
	template <typename UserClass>
	FDevInputKeyDelegateBinding& BindKey(const FInputChord& Chord, const EInputEvent KeyEvent, UserClass* Object, typename FDevInputKeyHandlerSignature::TMethodPtr<UserClass> Func, const bool bExecuteWhenPaused = true) const
	{
		FDevInputKeyDelegateBinding& Binding = AddBinding<FDevInputKeyDelegateBinding>(Chord, KeyEvent, bExecuteWhenPaused);
		Binding.Delegate.BindDelegate(Object, Func);
		return Binding;
	}

	FDevInputKeyDynamicDelegateBinding& BindKey(const FInputChord& Chord, const EInputEvent KeyEvent, UObject* Object, const FName FunctionName, const bool bExecuteWhenPaused = true) const
	{
		FDevInputKeyDynamicDelegateBinding& Binding = AddBinding<FDevInputKeyDynamicDelegateBinding>(Chord, KeyEvent, bExecuteWhenPaused);
		Binding.Delegate.BindDelegate(Object, FunctionName);
		return Binding;
	}

	FDevInputKeyDelegateBinding& BindKey(const FInputChord& Chord) const
	{
		return AddBinding<FDevInputKeyDelegateBinding>(Chord);
	}

	template <typename UserClass>
	FDevInputShortcutDelegateBinding& BindShortcut(const FDevInputShortcut& Shortcut, const EInputEvent KeyEvent, UserClass* Object, typename FDevInputKeyHandlerSignature::TMethodPtr<UserClass> Func, const bool bExecuteWhenPaused = true) const
	{
		FDevInputShortcutDelegateBinding& Binding = AddBinding<FDevInputShortcutDelegateBinding>(Shortcut, KeyEvent, bExecuteWhenPaused);
		Binding.Delegate.BindDelegate(Object, Func);
		return Binding;
	}

	FDevInputShortcutDynamicDelegateBinding& BindShortcut(const FDevInputShortcut& Shortcut, const EInputEvent KeyEvent, UObject* Object, const FName FunctionName, const bool bExecuteWhenPaused = true) const
	{
		FDevInputShortcutDynamicDelegateBinding& Binding = AddBinding<FDevInputShortcutDynamicDelegateBinding>(Shortcut, KeyEvent, bExecuteWhenPaused);
		Binding.Delegate.BindDelegate(Object, FunctionName);
		return Binding;
	}

	FDevInputShortcutDelegateBinding& BindShortcut(const FDevInputShortcut& Shortcut) const
	{
		return AddBinding<FDevInputShortcutDelegateBinding>(Shortcut);
	}

	template <typename TBindingType, typename... ArgsType>
	TBindingType& AddBinding(ArgsType&&... Args) const
	{
		TUniquePtr<TBindingType> BindingPtr = MakeUnique<TBindingType>(Forward<ArgsType>(Args)...);
		TBindingType& Binding = *BindingPtr;
		AddBinding(MoveTemp(BindingPtr));
		return Binding;
	}

	const TArray<TUniquePtr<FDevInputBinding>>& GetBindings() const;

	bool RemoveBinding(const FDevInputBinding& InBinding) const;
	bool RemoveBindingByHandle(const uint32 InHandle) const;
	void RemoveBindingAt(const int32 InIndex) const;

	void ClearBindingsForObject(const void* InObject) const;
	void ClearBindingsForOwner(const FDevInputOwner& InOwner) const;
	void ClearBindings() const;

public:
	EDevInputType GetCurrentInputType() const;
	FName GetCurrentGamepadName() const;

	void ConsumeInputEvent() const;
	void EmulateKeyPress(const FKey& InKey) const;

	FDevInputTypeChanged& OnInputTypeChanged() const;
	FDevInputGamepadChanged& OnInputGamepadChanged() const;
	FDevInputEvent& OnInputEvent() const;

public:
	void SetInputMode(const FInputModeDataBase& InData) const;
	void RestoreInputMode() const;

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	//~ Begin UObject Interface.
	virtual void BeginDestroy() override;
	//~ End UObject Interface.

private:
	friend class UDevInputSubsystem;

	UDevInputs();

	TPimplPtr<FDevInputManager> InputManager;
	TPimplPtr<FDevInputMode> InputMode;
	TSharedPtr<FDevInputProcessor> InputProcessor;

	void AddBinding(TUniquePtr<FDevInputBinding>&& InBindingPtr) const;
};
