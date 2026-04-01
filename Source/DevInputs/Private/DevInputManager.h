// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"

struct FDevInputBindingContext;

enum struct EDevInputContextTokenFlags : uint8
{
	Undefined = 0,
	Key = 1,
	LeftModifier = 2,
	RightModifier = 4,
	BothModifier = LeftModifier | RightModifier,
};

ENUM_CLASS_FLAGS(EDevInputContextTokenFlags);

struct FDevInputContextToken final : FDevInputToken
{
	FDevInputContextToken(const FName InTokenName, const EDevInputContextTokenFlags InTokenFlags)
		: FDevInputToken(InTokenName), TokenFlags(InTokenFlags) {};

	EDevInputContextTokenFlags TokenFlags;

	FString ToString() const { return GetName().ToString(); } // TODO Add modifiers information

	FORCEINLINE bool operator ==(const FDevInputContextToken& Rhs) const { return (GetName() == Rhs.GetName() && (TokenFlags & Rhs.TokenFlags) != EDevInputContextTokenFlags::Undefined); }
	FORCEINLINE bool operator !=(const FDevInputContextToken& Rhs) const { return !(*this == Rhs); }
	FORCEINLINE bool operator <(const FDevInputContextToken& Rhs) const { const int32 Result = GetName().Compare(Rhs.GetName()); return (Result < 0 || (Result == 0 && TokenFlags < Rhs.TokenFlags)); }

	FORCEINLINE friend uint32 GetTypeHash(const FDevInputContextToken& InToken) { return GetTypeHash(InToken.GetName()); }
};

struct FDevInputContextTokenFactory final
{
	static FDevInputContextToken CreateToken(const FKey& InKey);
	static FDevInputContextToken CreateToken(const FDevInputToken& InToken);
};


using FDevInputContextTokensSet = TArray<FDevInputContextToken, TNonRelocatableInlineAllocator<8>>;

struct FDevInputContextFragment final
{
	FORCEINLINE FDevInputContextFragment(FDevInputBinding& InBinding)
		: Binding(&InBinding), NextFragment(nullptr) {}

	FDevInputBinding* Binding;
	FDevInputContextFragment* NextFragment;
	FDevInputContextTokensSet FragmentTokens;
};


using FDevInputContextFragmentsSet = TArray<FDevInputContextFragment, TNonRelocatableInlineAllocator<8>>;

struct FDevInputBindingContext final
{
	FDevInputBindingContext(FDevInputBinding& InBinding);

	FDevInputContextFragmentsSet ContextFragments;
	FDevInputContextFragment* PressedFragment;
};


template <typename InElementType>
struct TDevInputContextDeque final
{
	FORCEINLINE void Empty() { Container.Empty(); }
	FORCEINLINE void PushLast(const InElementType& Element) { Container.Add(Element); }
	FORCEINLINE void RemoveSingle(const InElementType& Element) { Container.RemoveSingle(Element); }

	bool TryPopFirst(InElementType& OutValue)
	{
		if (Container.IsEmpty()) { return false; }
		OutValue = MoveTempIfPossible(Container.GetData()[0]);
		Container.RemoveAt(0);
		return true;
	}

private:
	TArray<InElementType> Container;
};


class FDevInputManager final
{
public:
	void AddBinding(TUniquePtr<FDevInputBinding>&& InBindingPtr);

	const TArray<TUniquePtr<FDevInputBinding>>& GetBindings() const { return Bindings; }

	bool RemoveBinding(const FDevInputBinding& InBinding);
	bool RemoveBindingByHandle(const uint32 InHandle);
	void RemoveBindingAt(const int32 InIndex);

	void ClearBindingsForObject(const void* InObject);
	void ClearBindingsForOwner(const FDevInputOwner& InOwner);
	void ClearBindings();

	void OnInputEvent(const FInputKeyParams& InKeyParams);

	void Dispose();

private:
	enum struct EPressCheckStrategy : uint8
	{
		ExactMatch = 0,
		PartialMatch = 1,
	};

	enum struct EPressCheckResult : uint8
	{
		NotPressed = 0,
		InProgress = 1,
		Pressed = 2,
	};

	TArray<TUniquePtr<FDevInputBinding>> Bindings;

	TMultiMap<FDevInputContextToken, FDevInputContextFragment*> InitialFragmentsLookup;
	TArray<FDevInputContextFragment*> ChainedFragmentsLookup;

	TDevInputContextDeque<FDevInputContextFragment*> TriggeredFragmentsOnDown;
	TDevInputContextDeque<FDevInputContextFragment*> TriggeredFragmentsOnUp;

	TArray<FDevInputBinding*> PressedBindings;
	TArray<FDevInputContextToken> PressedTokens;

	void RegisterBinding(FDevInputBinding& InBinding);
	void UnregisterBinding(FDevInputBinding& InBinding);

	void RegisterFragment(FDevInputContextFragment* InFragment);
	void UnregisterFragment(FDevInputContextFragment* InFragment);

	void ExecuteBinding(const FDevInputBinding* InBinding) const;

	EPressCheckResult IsFragmentPressed(const FDevInputContextFragment* InFragment, const EPressCheckStrategy InStrategy) const;

	void OnTokenDown(const FDevInputContextToken& InToken);
	void OnTokenUp(const FDevInputContextToken& InToken);
	void OnFragmentDown(FDevInputContextFragment* InFragment);
	void OnFragmentUp(FDevInputContextFragment* InFragment);
};
