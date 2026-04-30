// Copyright (c) Alexandr Pereverzev.

#include "DevInputManager.h"

#include "DevInputLogging.h"
#include "Framework/DevCorePlaySession.h"
#include "GameFramework/PlayerInput.h"

using namespace DevInput::Logging;

FDevInputContextToken FDevInputContextTokenFactory::CreateToken(const FKey& InKey)
{
	return CreateToken(FDevInputToken(InKey));
}

FDevInputContextToken FDevInputContextTokenFactory::CreateToken(const FDevInputToken& InToken)
{
	const FName TokenName = InToken.GetName();

	if (InToken == EDevInputTokens::PairedModifiers::Control) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Control.GetName(), EDevInputContextTokenFlags::BothModifier); }
	if (TokenName == EKeys::LeftControl.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Control.GetName(), EDevInputContextTokenFlags::LeftModifier); }
	if (TokenName == EKeys::RightControl.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Control.GetName(), EDevInputContextTokenFlags::RightModifier); }

	if (InToken == EDevInputTokens::PairedModifiers::Alt) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Alt.GetName(), EDevInputContextTokenFlags::BothModifier); }
	if (TokenName == EKeys::LeftAlt.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Alt.GetName(), EDevInputContextTokenFlags::LeftModifier); }
	if (TokenName == EKeys::RightAlt.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Alt.GetName(), EDevInputContextTokenFlags::RightModifier); }

	if (InToken == EDevInputTokens::PairedModifiers::Shift) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Shift.GetName(), EDevInputContextTokenFlags::BothModifier); }
	if (TokenName == EKeys::LeftShift.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Shift.GetName(), EDevInputContextTokenFlags::LeftModifier); }
	if (TokenName == EKeys::RightShift.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Shift.GetName(), EDevInputContextTokenFlags::RightModifier); }

	if (InToken == EDevInputTokens::PairedModifiers::Command) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Command.GetName(), EDevInputContextTokenFlags::BothModifier); }
	if (TokenName == EKeys::LeftCommand.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Command.GetName(), EDevInputContextTokenFlags::LeftModifier); }
	if (TokenName == EKeys::RightCommand.GetFName()) { return FDevInputContextToken(EDevInputTokens::PairedModifiers::Command.GetName(), EDevInputContextTokenFlags::RightModifier); }

	return FDevInputContextToken(TokenName, EDevInputContextTokenFlags::Key);
}

FDevInputBindingContext::FDevInputBindingContext(FDevInputBinding& InBinding)
	: PressedFragment(nullptr)
{
	FDevInputSequence InputSequence;
	InBinding.PopulateInputSequence(InputSequence);

	FDevInputContextFragment* ChainedFragment = nullptr;
	for (FDevInputSequence::TConstIterator ItInputTokens = InputSequence.CreateConstIterator(); ItInputTokens; ++ItInputTokens)
	{
		FDevInputContextFragment& ContextFragment = ContextFragments.Emplace_GetRef(InBinding);
		if (ChainedFragment)
		{
			ChainedFragment->NextFragment = &ContextFragment;
			ChainedFragment = nullptr;
		}

		for (/** Nop */; ItInputTokens; ++ItInputTokens)
		{
			FDevInputToken Token = *ItInputTokens;
			if (Token == EDevInputTokens::Special::Plus) { continue; }
			if (Token == EDevInputTokens::Special::Or) { break; }
			if (Token == EDevInputTokens::Special::Coma) { ChainedFragment = &ContextFragment; break; }

			ContextFragment.FragmentTokens.Emplace(FDevInputContextTokenFactory::CreateToken(Token));
		}
	}
}

void FDevInputManager::AddBinding(TUniquePtr<FDevInputBinding>&& InBindingPtr)
{
	RegisterBinding(*Bindings.Add_GetRef(MoveTemp(InBindingPtr)));
}

bool FDevInputManager::RemoveBinding(const FDevInputBinding& InBinding)
{
	for (int32 i = 0, Num = Bindings.Num(); i < Num; ++i)
	{
		if (*Bindings[i] == InBinding)
		{
			RemoveBindingAt(i);
			return true;
		}
	}
	return false;
}

bool FDevInputManager::RemoveBindingByHandle(const uint32 InHandle)
{
	for (int32 i = 0, Num = Bindings.Num(); i < Num; ++i)
	{
		if (Bindings[i]->GetHandle() == InHandle)
		{
			RemoveBindingAt(i);
			return true;
		}
	}
	return false;
}

void FDevInputManager::RemoveBindingAt(const int32 InIndex)
{
	UnregisterBinding(*Bindings[InIndex]);
	Bindings.RemoveAt(InIndex);
}

void FDevInputManager::ClearBindingsForObject(const void* InObject)
{
	for (int32 i = Bindings.Num() - 1; i >= 0; --i)
	{
		if (Bindings[i]->IsBoundToObject(InObject))
		{
			RemoveBindingAt(i);
		}
	}
}

void FDevInputManager::ClearBindingsForOwner(const FDevInputOwner& InOwner)
{
	for (int32 i = Bindings.Num() - 1; i >= 0; --i)
	{
		if (Bindings[i]->Owner == InOwner)
		{
			RemoveBindingAt(i);
		}
	}
}

void FDevInputManager::ClearBindings()
{
	InitialFragmentsLookup.Reset();
	ChainedFragmentsLookup.Reset();

	TriggeredFragmentsOnDown.Reset();
	TriggeredFragmentsOnUp.Reset();

	PressedBindings.Reset();

	Bindings.Empty();
}

void FDevInputManager::Dispose()
{
	ClearBindings();
	PressedTokens.Reset();
	PressedModifiers.Reset();
}

void FDevInputManager::RegisterBinding(FDevInputBinding& InBinding)
{
	InBinding.Context = MakePimpl<FDevInputBindingContext>(InBinding);
	for (FDevInputContextFragment& ContextFragments : InBinding.Context->ContextFragments)
	{
		RegisterFragment(&ContextFragments);
	}
}

void FDevInputManager::UnregisterBinding(FDevInputBinding& InBinding)
{
	PressedBindings.RemoveSingle(&InBinding);
	for (FDevInputContextFragment& ContextFragments : InBinding.Context->ContextFragments)
	{
		UnregisterFragment(&ContextFragments);
	}
	InBinding.Context = nullptr;
}

void FDevInputManager::OnInputEvent(const FDevInputKeyEventArgs& InKeyParams)
{
	const FDevInputContextToken Token = FDevInputContextTokenFactory::CreateToken(InKeyParams.Key);
	const bool bIsModifierToken = EDevInputTokens::IsModifierToken(Token);
	if (InKeyParams.Event == IE_Pressed)
	{
		PressedTokens.AddUnique(Token);
		if (bIsModifierToken) { PressedModifiers.AddUnique(Token); }
		UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Token down: Token = %s, Pressed Tokens = [%s]"), *Token.ToString(), *TokensToString(PressedTokens));
		OnTokenDown(Token);
	}
	else
	{
		PressedTokens.RemoveSingle(Token);
		if (bIsModifierToken) { PressedModifiers.RemoveSingle(Token); }
		UE_LOG_FUNCTION(LogDevInputs, VeryVerbose, TEXT("Token up: Token = %s, Pressed Tokens = [%s]"), *Token.ToString(), *TokensToString(PressedTokens));
		OnTokenUp(Token);
	}
}

void FDevInputManager::RegisterFragment(FDevInputContextFragment* InFragment)
{
	bool bIsAnyKeyRegistered = false;
	for (int32 i = InFragment->FragmentTokens.Num() - 1; i >= 0; --i)
	{
		const FDevInputContextToken& Token = InFragment->FragmentTokens[i];
		if (const bool bIsKey = (Token.TokenFlags == EDevInputContextTokenFlags::Key); bIsKey || !bIsAnyKeyRegistered)
		{
			InitialFragmentsLookup.Add(Token, InFragment);
			bIsAnyKeyRegistered |= bIsKey;
		}
	}
}

void FDevInputManager::UnregisterFragment(FDevInputContextFragment* InFragment)
{
	for (const FDevInputContextToken& Token : InFragment->FragmentTokens)
	{
		InitialFragmentsLookup.Remove(Token, InFragment);
	}

	ChainedFragmentsLookup.RemoveSingle(InFragment);
	TriggeredFragmentsOnDown.RemoveSingle(InFragment);

	TriggeredFragmentsOnUp.RemoveSingle(InFragment);
}

void FDevInputManager::ExecuteBinding(const FDevInputBinding* InBinding) const
{
	if (!InBinding->bExecuteWhenPaused)
	{
		if (const UObject* WorldContextObject = WorldContextObjectPtr.Get(); WorldContextObject && FDevCorePlaySession::IsGamePaused(WorldContextObject))
		{
			return;
		}
	}

	InBinding->Execute();
}

FDevInputManager::EPressCheckResult FDevInputManager::IsFragmentPressed(const FDevInputContextFragment* InFragment, const FDevInputContextToken& InToken, const EPressCheckStrategy InStrategy) const
{
	const FDevInputContextTokensSet& FragmentTokens = InFragment->FragmentTokens;
	if (InFragment->IsSingleTokenFragment())
	{
		return (InToken == FragmentTokens.Last() && (PressedModifiers.Num() == 0 || PressedTokens.Num() == 1)) ? EPressCheckResult::Pressed : EPressCheckResult::NotPressed;
	}
	else
	{
		const int32 FragmentTokensNum = FragmentTokens.Num();
		const int32 PressedTokensNum = PressedTokens.Num();
		if (FragmentTokensNum < PressedTokensNum && InStrategy != EPressCheckStrategy::PartialMatch) { return EPressCheckResult::NotPressed; }

		for (int32 i = 0; i < PressedTokensNum && i < FragmentTokensNum; ++i)
		{
			if (!FragmentTokens.Contains(PressedTokens[PressedTokensNum - 1 - i])) { return EPressCheckResult::NotPressed; }
		}
		return (FragmentTokensNum > PressedTokensNum) ? EPressCheckResult::InProgress : EPressCheckResult::Pressed;
	}
}

void FDevInputManager::OnTokenDown(const FDevInputContextToken& InToken)
{
	for (int32 i = 0; i < ChainedFragmentsLookup.Num(); /** Nop */)
	{
		if (FDevInputContextFragment* ChainedFragment = ChainedFragmentsLookup[i]; ChainedFragment->FragmentTokens.Contains(InToken))
		{
			if (IsFragmentPressed(ChainedFragment, InToken, EPressCheckStrategy::PartialMatch) == EPressCheckResult::Pressed)
			{
				TriggeredFragmentsOnDown.PushLast(ChainedFragment);
				ChainedFragmentsLookup.RemoveAt(i);
			}
			else
			{
				++i;
			}
		}
		else
		{
			ChainedFragmentsLookup.RemoveAt(i);
		}
	}

	thread_local TArray<FDevInputContextFragment*> InitialFragments;
	ON_SCOPE_EXIT { InitialFragments.Reset(); };

	InitialFragmentsLookup.MultiFind(InToken, InitialFragments);
	for (FDevInputContextFragment* InitialFragment : InitialFragments)
	{
		if (IsFragmentPressed(InitialFragment, InToken, EPressCheckStrategy::ExactMatch) == EPressCheckResult::Pressed)
		{
			TriggeredFragmentsOnDown.PushLast(InitialFragment);
		}
	}

	FDevInputContextFragment* TriggeredFragment;
	while (TriggeredFragmentsOnDown.TryPopFirst(TriggeredFragment))
	{
		if (TriggeredFragment->NextFragment)
		{
			ChainedFragmentsLookup.Add(TriggeredFragment);
		}
		else
		{
			OnFragmentDown(TriggeredFragment);
		}
	}
}

void FDevInputManager::OnTokenUp(const FDevInputContextToken& InToken)
{
	for (int32 i = 0; i < ChainedFragmentsLookup.Num(); /** Nop */)
	{
		if (const FDevInputContextFragment* ChainedFragment = ChainedFragmentsLookup[i]; ChainedFragment->FragmentTokens.Contains(InToken))
		{
			++i;
		}
		else
		{
			ChainedFragmentsLookup.RemoveAt(i);
		}
	}

	for (const FDevInputBinding* PressedBinding : PressedBindings)
	{
		if (FDevInputContextFragment* PressedFragment = PressedBinding->Context->PressedFragment; PressedFragment->FragmentTokens.Contains(InToken))
		{
			TriggeredFragmentsOnUp.PushLast(PressedFragment);
		}
	}

	FDevInputContextFragment* TriggeredFragment;
	while (TriggeredFragmentsOnUp.TryPopFirst(TriggeredFragment))
	{
		OnFragmentUp(TriggeredFragment);
	}
}

void FDevInputManager::OnFragmentDown(FDevInputContextFragment* InFragment)
{
	FDevInputBinding* Binding = InFragment->Binding;
	if (PressedBindings.Contains(Binding)) { return; } // Binding was triggered via a different sequence fragment.
	PressedBindings.Add(Binding);

	Binding->Context->PressedFragment = InFragment;

	if (Binding->KeyEvent == IE_Pressed)
	{
		ExecuteBinding(Binding);
	}
}

void FDevInputManager::OnFragmentUp(FDevInputContextFragment* InFragment)
{
	FDevInputBinding* Binding = InFragment->Binding;
	PressedBindings.Remove(Binding);

	Binding->Context->PressedFragment = nullptr;

	if (Binding->KeyEvent == IE_Released)
	{
		ExecuteBinding(Binding);
	}
}
