// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"
#include "Engine/EngineBaseTypes.h"
#include "Framework/Commands/InputChord.h"
#include "DevInputTypes.generated.h"

struct FInputKeyParams;
struct FDevInputBindingContext;
class FDevInputManager;

UENUM(BlueprintType)
enum struct EDevInputDisplayNameLength : uint8
{
	Short = 0,
	Long = 1,
};

USTRUCT(BlueprintType, Blueprintable)
struct FDevInputToken
{
	GENERATED_BODY()

	FDevInputToken() = default;
	FDevInputToken(const FName InTokenName)
		: TokenName(InTokenName) {}
	FDevInputToken(const ANSICHAR* InName)
		: TokenName(FName(InName)) {}
	FDevInputToken(const WIDECHAR* InName)
		: TokenName(FName(InName)) {}
	FDevInputToken(const FKey& InKey)
		: TokenName(InKey.GetFName()) {}

	DEVINPUTS_API FORCEINLINE FName GetName() const { return TokenName; }
	DEVINPUTS_API FORCEINLINE bool IsNone() const { return TokenName.IsNone(); }
	DEVINPUTS_API FORCEINLINE bool IsValid() const;
	DEVINPUTS_API FORCEINLINE FString ToString() const { return ToString(EDevInputDisplayNameLength::Short); }
	DEVINPUTS_API FString ToString(const EDevInputDisplayNameLength InDisplayNameLenght) const;

	FORCEINLINE bool operator ==(const FDevInputToken& Rhs) const { return (TokenName == Rhs.TokenName); }
	FORCEINLINE bool operator !=(const FDevInputToken& Rhs) const { return (TokenName != Rhs.TokenName); }
	FORCEINLINE bool operator <(const FDevInputToken& Rhs) const { return TokenName.LexicalLess(Rhs.TokenName); }

	FORCEINLINE operator FKey() const { return FKey(TokenName); }

	FORCEINLINE friend uint32 GetTypeHash(const FDevInputToken& Value) { return GetTypeHash(Value.TokenName); }

private:
	UPROPERTY(EditAnywhere, Category = "DevHub|Input")
	FName TokenName;
};

struct EDevInputTokens final
{
	static DEVINPUTS_API const FDevInputToken None;

	struct Special final
	{
		static DEVINPUTS_API const FDevInputToken Plus;
		static DEVINPUTS_API const FDevInputToken Coma;
		static DEVINPUTS_API const FDevInputToken Or;
	};

	struct PairedModifiers final
	{
		static DEVINPUTS_API const FDevInputToken Control;
		static DEVINPUTS_API const FDevInputToken Alt;
		static DEVINPUTS_API const FDevInputToken Shift;
		static DEVINPUTS_API const FDevInputToken Command;
	};

	static DEVINPUTS_API FDevInputToken FindToken(const FStringView& InAnyTokenName);

	static DEVINPUTS_API FORCEINLINE bool IsValidToken(const FDevInputToken& InToken);
	static DEVINPUTS_API FORCEINLINE bool IsSpecialToken(const FDevInputToken& InToken);

	static DEVINPUTS_API FText GetTokenDisplayName(const FDevInputToken& InToken, const EDevInputDisplayNameLength InDisplayNameLenght = EDevInputDisplayNameLength::Short);

private:
	friend class FDevInputsModule;

	static void Initialise();
};

using FDevInputSequence = TArray<FDevInputToken>;


USTRUCT(BlueprintType, Blueprintable, Meta = (ShowOnlyInnerProperties))
struct FDevInputShortcut
{
	GENERATED_BODY()

	DEVINPUTS_API FDevInputShortcut() = default;
	DEVINPUTS_API FDevInputShortcut(const FName InName)
		: ShortcutName(InName) {}
	DEVINPUTS_API FDevInputShortcut(const ANSICHAR* InName)
		: ShortcutName(FName(InName)) {}
	DEVINPUTS_API FDevInputShortcut(const WIDECHAR* InName)
		: ShortcutName(FName(InName)) {}

	FORCEINLINE static FDevInputShortcut GetEmpty() { return FDevInputShortcut(); }

	DEVINPUTS_API FORCEINLINE FName GetName() const { return ShortcutName; }
	DEVINPUTS_API FORCEINLINE bool IsNone() const { return ShortcutName.IsNone(); }
	DEVINPUTS_API FORCEINLINE bool IsValid() const { return ShortcutName.IsValid(); } // TODO: Check parse and analise validity.
	DEVINPUTS_API FORCEINLINE FString ToString() const { return (ShortcutName.IsNone()) ? FString() : ShortcutName.ToString(); }
	DEVINPUTS_API FORCEINLINE FText ToText() const { return (ShortcutName.IsNone()) ? FText::GetEmpty() : FText::FromName(ShortcutName); }

	FORCEINLINE bool operator ==(const FDevInputShortcut& Rhs) const { return (ShortcutName == Rhs.ShortcutName); }
	FORCEINLINE bool operator !=(const FDevInputShortcut& Rhs) const { return (ShortcutName != Rhs.ShortcutName); }
	FORCEINLINE bool operator <(const FDevInputShortcut& Rhs) const { return ShortcutName.LexicalLess(Rhs.ShortcutName); }

	FORCEINLINE friend uint32 GetTypeHash(const FDevInputShortcut& Value) { return GetTypeHash(Value.ShortcutName); }

private:
	UPROPERTY(EditAnywhere, Category = "DevHub|Input", Meta = (DisplayName = "Shortcut"))
	FName ShortcutName;
};


USTRUCT(BlueprintType, Blueprintable)
struct FDevInputOwner
{
	GENERATED_BODY()

private:
	struct FStoreName
	{
		int32 Index;
		int32 Number;
	};

	enum class EValueType : uint8
	{
		None,
		Pointer,
		Name,
	};

public:
	FORCEINLINE FDevInputOwner() : ValueInt64(0), ValueType(EValueType::None) {}
	FORCEINLINE FDevInputOwner(void* InPointer) : ValueInt64(reinterpret_cast<int64>(InPointer)), ValueType(EValueType::Pointer) {}

	FDevInputOwner(const WIDECHAR* InValue) : FDevInputOwner(FName(InValue)) {}
	FDevInputOwner(const ANSICHAR* InValue) : FDevInputOwner(FName(InValue)) {}

	FDevInputOwner(const FName InValue)
	{
		if (InValue == NAME_None)
		{
			ValueInt64 = 0;
			ValueType = EValueType::None;
		}
		else
		{
			ValueName.Index = InValue.GetComparisonIndex().ToUnstableInt();
			ValueName.Number = InValue.GetNumber();
			ValueType = EValueType::Name;
		}
	}

	FORCEINLINE bool operator ==(const FDevInputOwner& Other) const
	{
		return Other.ValueInt64 == ValueInt64 && Other.ValueType == ValueType;
	}

	FORCEINLINE bool operator !=(const FDevInputOwner& Other) const
	{
		return Other.ValueInt64 != ValueInt64 || Other.ValueType != ValueType;
	}

	friend uint32 GetTypeHash(const FDevInputOwner& Key)
	{
		return GetTypeHash(Key.ValueInt64);
	}

	FORCEINLINE bool IsSet() const { return ValueInt64 != 0; }

	FName TryGetName() const
	{
		if (ValueType == EValueType::Name)
		{
			const FNameEntryId EntryId = FNameEntryId::FromUnstableInt(ValueName.Index);
			return FName(EntryId, EntryId, ValueName.Number);
		}

		return NAME_None;
	}

private:

	union
	{
		int64 ValueInt64;
		FStoreName ValueName;
	};

	EValueType ValueType;
};


/** Delegate signature for debug key events. */
DECLARE_DELEGATE(FDevInputKeyHandlerSignature);
DECLARE_DYNAMIC_DELEGATE(FDevInputKeyHandlerDynamicSignature);

/** Delegate signature for debug shortcut events. */
DECLARE_DELEGATE(FDevInputShortcutHandlerSignature);
DECLARE_DYNAMIC_DELEGATE(FDevInputShortcutHandlerDynamicSignature);

// By default, this behavior is on in the editor only. If you would like to turn this
// off, then add the following to your Build.cs file:
//		PublicDefinitions.Add("ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES=0");
#ifndef ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES
	#define ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES	WITH_EDITOR
#elif !WITH_EDITOR
	#define ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES	0
#endif

/** Unified storage for both native and dynamic delegates with any signature. */
template<typename TSignature>
struct TDevInputUnifiedDelegate
{
protected:
	/** Holds the delegate to call. */
	TSharedPtr<TSignature> Delegate;

	/** Should this delegate fire with an Editor Script function guard? */
	bool bShouldFireWithEditorScriptGuard = false;

public:
	bool IsBound() const
	{
		return Delegate.IsValid() && Delegate->IsBound();
	}

	bool IsBoundToObject(void const* Object) const
	{
		return IsBound() && Delegate->IsBoundToObject(Object);
	}

	void Unbind()
	{
		if (Delegate)
		{
			Delegate->Unbind();
		}
	}

	void SetShouldFireWithEditorScriptGuard(const bool bNewValue) { bShouldFireWithEditorScriptGuard = bNewValue; }
	
	bool ShouldFireWithEditorScriptGuard() const { return bShouldFireWithEditorScriptGuard; }

	/** Binds a native delegate, hidden for script delegates */
	template<	typename UserClass,
				typename TSig = TSignature,
				typename... TVars>
	void BindDelegate(UserClass* Object, typename TSig::template TMethodPtr<UserClass, TVars...> Func, TVars... Vars)
	{
		Unbind();
		Delegate = MakeShared<TSig>(TSig::CreateUObject(Object, Func, Vars...));
	}

	/** Binds a script delegate on an arbitrary UObject, hidden for native delegates */
	template<	typename TSig = TSignature,
				typename = typename TEnableIf<TIsDerivedFrom<TSig, FScriptDelegate>::IsDerived || TIsDerivedFrom<TSig, FMulticastScriptDelegate>::IsDerived>::Type>
	void BindDelegate(UObject* Object, const FName FuncName)
	{
		Unbind();
		Delegate = MakeShared<TSig>();
		Delegate->BindUFunction(Object, FuncName);
	}

	/** Binds a lambda expression */
	template<typename TSig = TSignature, typename FunctorType, typename... VarTypes>
	void BindLambda(FunctorType&& InFunctor, VarTypes&&... Vars)
	{
		Unbind();
		Delegate = MakeShared<TSig>();
		Delegate->BindLambda(Forward<FunctorType>(InFunctor), Forward<VarTypes>(Vars)...);
	}

	template<typename TSig = TSignature>
	TSig& MakeDelegate()
	{
		Unbind();
		Delegate = MakeShared<TSig>();
		return *Delegate;
	}

	template<typename... TArgs>
	void Execute(TArgs... Args) const
	{
		if (IsBound())
		{
#if ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES
			if (bShouldFireWithEditorScriptGuard)
			{
				FEditorScriptExecutionGuard ScriptGuard;
				Delegate->Execute(Args...);
			}
			else
			{
				Delegate->Execute(Args...);	
			}
#else
			Delegate->Execute(Args...);
#endif	// ENABLE_EDITOR_CALLABLE_INPUT_DELEGATES
		}
	}
};

/** A basic binding unique identifier. */
struct FDevInputBindingHandle
{
	DEVINPUTS_API FDevInputBindingHandle();	// Generates a handle.
	virtual ~FDevInputBindingHandle() = default;

	uint32 GetHandle() const { return Handle; }

	bool operator ==(const FDevInputBindingHandle& Rhs) const { return (GetHandle() == Rhs.GetHandle()); }

private:
	uint32 Handle = 0;
};

struct FDevInputBinding : FDevInputBindingHandle
{
	FDevInputBinding() = default;
	FDevInputBinding(const EInputEvent InKeyEvent, bool bInExecuteWhenPaused)
		: KeyEvent(InKeyEvent), bExecuteWhenPaused(bInExecuteWhenPaused) {}

	/** Key event to bind it to (e.g. pressed, released, double click) */
	TEnumAsByte<EInputEvent> KeyEvent = IE_Pressed;

	bool bExecuteWhenPaused = false;

	FDevInputOwner Owner;

	virtual void Execute() const = 0;
	virtual bool IsBoundToObject(void const* InObject) const = 0;
	virtual void PopulateInputSequence(FDevInputSequence& OutInputSequence) const = 0;

private:
	friend FDevInputManager;

	TPimplPtr<FDevInputBindingContext> Context;
};

struct FDevInputKeyBinding : FDevInputBinding
{
	FDevInputKeyBinding(const FInputChord& InChord)
		: Chord(InChord) {}
	FDevInputKeyBinding(const FInputChord& InChord, const EInputEvent InKeyEvent, const bool bInExecuteWhenPaused)
		: FDevInputBinding(InKeyEvent, bInExecuteWhenPaused), Chord(InChord) {}

	FORCEINLINE FInputChord GetChord() const { return Chord; }

	//~ Begin FDevInputBinding interface.
	virtual void PopulateInputSequence(FDevInputSequence& OutInputSequence) const override;
	//~ End FDevInputBinding interface.

private:
	/** Input chord to bind to. */
	FInputChord Chord;
};

template<typename TSignature>
struct FDevInputKeySpecialisedBinding final : FDevInputKeyBinding
{
	FDevInputKeySpecialisedBinding(const FInputChord& InChord)
		: FDevInputKeyBinding(InChord) {}
	FDevInputKeySpecialisedBinding(const FInputChord& InChord, const EInputEvent InKeyEvent, const bool bInExecuteWhenPaused)
		: FDevInputKeyBinding(InChord, InKeyEvent, bInExecuteWhenPaused) {}

	TDevInputUnifiedDelegate<TSignature> Delegate;

	//~ Begin FDevInputBinding interface.
	virtual void Execute() const override { Delegate.Execute(); };
	virtual bool IsBoundToObject(void const* InObject) const override { return Delegate.IsBoundToObject(InObject); };
	//~ End FDevInputBinding interface.
};

using FDevInputKeyDelegateBinding = FDevInputKeySpecialisedBinding<FDevInputKeyHandlerSignature>;
using FDevInputKeyDynamicDelegateBinding = FDevInputKeySpecialisedBinding<FDevInputKeyHandlerDynamicSignature>;


struct FDevInputShortcutBinding : FDevInputBinding
{
	FDevInputShortcutBinding(const FDevInputShortcut& InShortcut)
		: Shortcut(InShortcut) {}
	FDevInputShortcutBinding(const FDevInputShortcut& InShortcut, const EInputEvent InKeyEvent, const bool bInExecuteWhenPaused)
		: FDevInputBinding(InKeyEvent, bInExecuteWhenPaused), Shortcut(InShortcut) {}

	FORCEINLINE FDevInputShortcut GetShortcut() const { return Shortcut; }

	//~ Begin FDevInputBinding interface.
	virtual void PopulateInputSequence(FDevInputSequence& OutInputSequence) const override;
	//~ End FDevInputBinding interface.

private:
	/** Input shortcut to bind to. */
	FDevInputShortcut Shortcut;
};

template<typename TSignature>
struct TDevInputShortcutSpecialisedBinding final : FDevInputShortcutBinding
{
	TDevInputShortcutSpecialisedBinding(const FDevInputShortcut& InShortcut)
		: FDevInputShortcutBinding(InShortcut) {}
	TDevInputShortcutSpecialisedBinding(const FDevInputShortcut& InShortcut, const EInputEvent InKeyEvent, const bool bInExecuteWhenPaused)
		: FDevInputShortcutBinding(InShortcut, InKeyEvent, bInExecuteWhenPaused) {}

	TDevInputUnifiedDelegate<TSignature> Delegate;

	//~ Begin FDevInputBinding interface.
	virtual void Execute() const override { Delegate.Execute(); };
	virtual bool IsBoundToObject(void const* InObject) const override { return Delegate.IsBoundToObject(InObject); };
	//~ End FDevInputBinding interface.
};

using FDevInputShortcutDelegateBinding = TDevInputShortcutSpecialisedBinding<FDevInputShortcutHandlerSignature>;
using FDevInputShortcutDynamicDelegateBinding = TDevInputShortcutSpecialisedBinding<FDevInputShortcutHandlerDynamicSignature>;


UENUM(BlueprintType)
enum class EDevInputType : uint8
{
	Undefined = 0,
	Mouse = 1,
	Keyboard = 2,
	Gamepad = 3,
	Touch = 4,
};

DECLARE_MULTICAST_DELEGATE_OneParam(FDevInputEvent, const FInputKeyParams&);
DECLARE_MULTICAST_DELEGATE_OneParam(FDevInputTypeChanged, const EDevInputType);
