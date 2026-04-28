// Copyright (c) Alexandr Pereverzev.

#include "DevInputTypes.h"

#include "DevInputShortcutBuilder.h"
#include "Modules/ModuleManager.h"

using Special = EDevInputTokens::Special;
using PairedModifiers = EDevInputTokens::PairedModifiers;


bool FDevInputToken::IsValid() const
{
	return EDevInputTokens::IsValidToken(*this);
}

FString FDevInputToken::ToString(const EDevInputDisplayNameLength InDisplayNameLength) const
{
	return EDevInputTokens::GetTokenDisplayName(*this, InDisplayNameLength).ToString();
}


const FDevInputToken EDevInputTokens::None(NAME_None);

const FDevInputToken Special::Plus("[PLUS]");
const FDevInputToken Special::Coma("[COMA]");
const FDevInputToken Special::Or("[OR]");

const FDevInputToken PairedModifiers::Control("Control");
const FDevInputToken PairedModifiers::Alt("Alt");
const FDevInputToken PairedModifiers::Shift("Shift");
const FDevInputToken PairedModifiers::Command("Command");

namespace
{
#define DEBUG_INPUTS_ADD_TOKEN(Token, ...) { Token, MakeShared<FKeyDetails>(Token, ##__VA_ARGS__) }
#define DEBUG_INPUTS_ADD_ONE_ALIAS(Token, Alias1) { Alias1, Token }
#define DEBUG_INPUTS_ADD_TWO_ALIASES(Token, Alias1, Alias2) { Alias1, Token }, { Alias2, Token }
#define DEBUG_INPUTS_ADD_THREE_ALIASES(Token, Alias1, Alias2, Alias3) { Alias1, Token }, { Alias2, Token }, { Alias3, Token }
#define DEBUG_INPUTS_ADD_FOUR_ALIASES(Token, Alias1, Alias2, Alias3, Alias4) { Alias1, Token }, { Alias2, Token }, { Alias3, Token }, { Alias4, Token }

	const TMap<FDevInputToken, TSharedPtr<FKeyDetails>> InputTokens
	{
		// Special Tokens.
		DEBUG_INPUTS_ADD_TOKEN(Special::Plus, INVTEXT("[PLUS]")),
		DEBUG_INPUTS_ADD_TOKEN(Special::Coma, INVTEXT("[COMA]")),
		DEBUG_INPUTS_ADD_TOKEN(Special::Or, INVTEXT("[OR]")),

		// Paired Modifiers.
		DEBUG_INPUTS_ADD_TOKEN(PairedModifiers::Control, INVTEXT("Control"), INVTEXT("Ctrl"), FKeyDetails::ModifierKey),
		DEBUG_INPUTS_ADD_TOKEN(PairedModifiers::Alt, INVTEXT("Alt"), FKeyDetails::ModifierKey),
		DEBUG_INPUTS_ADD_TOKEN(PairedModifiers::Shift, INVTEXT("Shift"), FKeyDetails::ModifierKey),
		DEBUG_INPUTS_ADD_TOKEN(PairedModifiers::Command, INVTEXT("Command"), FKeyDetails::ModifierKey),
	};
	TMap<FString, FDevInputToken> InputAliases
	{
		// Mouse.
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::LeftMouseButton, TEXT("Left Mouse Button"), TEXT("LMB")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::RightMouseButton, TEXT("Right Mouse Button"), TEXT("RMB")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::MiddleMouseButton, TEXT("Middle Mouse Button"), TEXT("MMB")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::ThumbMouseButton, TEXT("Thumb Mouse Button One"), TEXT("TMB1")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::ThumbMouseButton2, TEXT("Thumb Mouse Button Two"), TEXT("TMB2")),

		// Keyboard.
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::LeftControl, TEXT("Left Control"), TEXT("Left Ctrl")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::RightControl, TEXT("Right Control"), TEXT("Right Ctrl")),

		// Controller.
		DEBUG_INPUTS_ADD_THREE_ALIASES(EKeys::Gamepad_DPad_Up, TEXT("Gamepad D-pad Top"), TEXT("D-pad Up"), TEXT("D-pad Top")),
		DEBUG_INPUTS_ADD_THREE_ALIASES(EKeys::Gamepad_DPad_Down, TEXT("Gamepad D-pad Bottom"), TEXT("D-pad Down"), TEXT("D-pad Bottom")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::Gamepad_DPad_Right, TEXT("Gamepad D-pad Right"), TEXT("D-pad Right")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::Gamepad_DPad_Left, TEXT("Gamepad D-pad Left"), TEXT("D-pad Left")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftStick_Up, TEXT("Gamepad Left Thumbstick Up"), TEXT("Left Thumbstick Up"), TEXT("L3 Up"), TEXT("LS Up")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftStick_Down, TEXT("Gamepad Left Thumbstick Down"), TEXT("Left Thumbstick Down"), TEXT("L3 Down"), TEXT("LS Down")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftStick_Right, TEXT("Gamepad Left Thumbstick Right"), TEXT("Left Thumbstick Right"), TEXT("L3 Right"), TEXT("LS Right")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftStick_Left, TEXT("Gamepad Left Thumbstick Left"), TEXT("Left Thumbstick Left"), TEXT("L3 Left"), TEXT("LS Left")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightStick_Up, TEXT("Gamepad Right Thumbstick Up"), TEXT("Right Thumbstick Up"), TEXT("R3 Up"), TEXT("RS Up")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightStick_Down, TEXT("Gamepad Right Thumbstick Down"), TEXT("Right Thumbstick Down"), TEXT("R3 Down"), TEXT("RS Down")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightStick_Right, TEXT("Gamepad Right Thumbstick Right"), TEXT("Right Thumbstick Right"), TEXT("R3 Right"), TEXT("RS Right")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightStick_Left, TEXT("Gamepad Right Thumbstick Left"), TEXT("Right Thumbstick Left"), TEXT("R3 Left"), TEXT("RS Left")),

		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::Gamepad_Special_Left, TEXT("Gamepad Special Left"), TEXT("Special Left")),
		DEBUG_INPUTS_ADD_TWO_ALIASES(EKeys::Gamepad_Special_Right, TEXT("Gamepad Special Right"), TEXT("Special Right")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_FaceButton_Top, TEXT("Gamepad Face Button Top"), TEXT("Face Up"), TEXT("Face Top"), TEXT("Triangle")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_FaceButton_Bottom, TEXT("Gamepad Face Button Bottom"), TEXT("Face Down"), TEXT("Face Bottom"), TEXT("Cross")),
		DEBUG_INPUTS_ADD_THREE_ALIASES(EKeys::Gamepad_FaceButton_Right, TEXT("Gamepad Face Button Right"), TEXT("Face Right"), TEXT("Circle")),
		DEBUG_INPUTS_ADD_THREE_ALIASES(EKeys::Gamepad_FaceButton_Left, TEXT("Gamepad Face Button Left"), TEXT("Face Left"), TEXT("Square")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftShoulder, TEXT("Gamepad Left Shoulder"), TEXT("Left Shoulder"), TEXT("L1"), TEXT("LB")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightShoulder, TEXT("Gamepad Right Shoulder"), TEXT("Right Shoulder"), TEXT("R1"), TEXT("RB")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftTrigger, TEXT("Gamepad Left Trigger"), TEXT("Left Trigger"), TEXT("L2"), TEXT("LT")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightTrigger, TEXT("Gamepad Right Trigger"), TEXT("Right Trigger"), TEXT("R2"), TEXT("RT")),

		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_LeftThumbstick, TEXT("Gamepad Left Thumbstick"), TEXT("Left Thumbstick"), TEXT("L3"), TEXT("LS")),
		DEBUG_INPUTS_ADD_FOUR_ALIASES(EKeys::Gamepad_RightThumbstick, TEXT("Gamepad Right Thumbstick"), TEXT("Right Thumbstick"), TEXT("R3"), TEXT("RS")),
	};

#undef DEBUG_INPUTS_ADD_TOKEN
#undef DEBUG_INPUTS_ADD_ONE_ALIAS
#undef DEBUG_INPUTS_ADD_TWO_ALIASES
#undef DEBUG_INPUTS_ADD_THREE_ALIASES

	const TSet<FDevInputToken> ModifierTokens
	{
		// Keyboard modifiers — left and right separately.
		FDevInputToken(EKeys::LeftControl),
		FDevInputToken(EKeys::RightControl),
		FDevInputToken(EKeys::LeftAlt),
		FDevInputToken(EKeys::RightAlt),
		FDevInputToken(EKeys::LeftShift),
		FDevInputToken(EKeys::RightShift),
		FDevInputToken(EKeys::LeftCommand),
		FDevInputToken(EKeys::RightCommand),

		// Gamepad triggers, shoulders, and thumbsticks.
		FDevInputToken(EKeys::Gamepad_LeftTrigger),
		FDevInputToken(EKeys::Gamepad_RightTrigger),
		FDevInputToken(EKeys::Gamepad_LeftShoulder),
		FDevInputToken(EKeys::Gamepad_RightShoulder),
		FDevInputToken(EKeys::Gamepad_LeftThumbstick),
		FDevInputToken(EKeys::Gamepad_RightThumbstick),
	};
}

static const TMap<FString, FDevInputToken>& GetInputTokensLookup()
{
	static const TMap<FString, FDevInputToken>& InputTokensLookup = []
	{
		FModuleManager::Get().LoadModuleChecked(TEXT("InputCore"));

		TMap<FString, FDevInputToken>& Lookup = InputAliases;

		TArray<FKey> Keys;
		EKeys::Initialize();
		EKeys::GetAllKeys(Keys);

		Lookup.Reserve(Lookup.Num() + InputTokens.Num() * 3); // Name, short display name, and long display name.

		for (const FKey& Key : Keys)
		{
			const FDevInputToken Token{ Key };
			const FString Name = Key.GetFName().ToString();
			const FString ShortDisplayName = Key.GetDisplayName(false).ToString();
			const FString LongDisplayName = Key.GetDisplayName(true).ToString();
			Lookup.Add(Name, Token);
			Lookup.Add(ShortDisplayName, Token);
			if (LongDisplayName != ShortDisplayName) { Lookup.Add(LongDisplayName, Token); }
		}

		for (const auto& [Token, KeyDetails] : InputTokens)
		{
			const FString Name = Token.GetName().ToString();
			const FString ShortDisplayName = KeyDetails->GetDisplayName(false).ToString();
			const FString LongDisplayName = KeyDetails->GetDisplayName(true).ToString();
			Lookup.Add(Name, Token);
			Lookup.Add(ShortDisplayName, Token);
			if (!LongDisplayName.IsEmpty() && LongDisplayName != ShortDisplayName) { Lookup.Add(LongDisplayName, Token); }
		}

		return Lookup;
	}();

	return InputTokensLookup;
}

FDevInputToken EDevInputTokens::FindToken(const FStringView& InAnyTokenName)
{
	const FDevInputToken* TokenPtr = GetInputTokensLookup().FindByHash(GetTypeHash(InAnyTokenName), InAnyTokenName);
	return (TokenPtr) ? *TokenPtr : None;
}

bool EDevInputTokens::IsValidToken(const FDevInputToken& InToken)
{
	return InputTokens.Contains(InToken) || EKeys::GetKeyDetails(InToken);
}

bool EDevInputTokens::IsSpecialToken(const FDevInputToken& InToken)
{
	return (InToken == Special::Plus)
		|| (InToken == Special::Coma)
		|| (InToken == Special::Or);
}

bool EDevInputTokens::IsModifierToken(const FDevInputToken& InToken)
{
	return ModifierTokens.Contains(InToken);
}

FText EDevInputTokens::GetTokenDisplayName(const FDevInputToken& InToken, const EDevInputDisplayNameLength InDisplayNameLength)
{
	if (InToken.IsNone()) { return FText::GetEmpty(); }

	const TSharedPtr<FKeyDetails>* KeyDetailsPtr = InputTokens.Find(InToken);
	const TSharedPtr<FKeyDetails> KeyDetails = (KeyDetailsPtr)
		? *KeyDetailsPtr
		: EKeys::GetKeyDetails(InToken);

	return (KeyDetails)
		? KeyDetails->GetDisplayName(InDisplayNameLength == EDevInputDisplayNameLength::Long)
		: FText::FromName(InToken.GetName());
}

void EDevInputTokens::Initialise()
{
	GetInputTokensLookup();
}


// Must be in C++ to avoid duplicate statics across execution units.
static uint32 GInputBindingHandle = 1;

FDevInputBindingHandle::FDevInputBindingHandle()
{
	// Handles are shared between all binding types.
	Handle = GInputBindingHandle++;
}


void FDevInputKeyBinding::PopulateInputSequence(FDevInputSequence& OutInputSequence) const
{
	if (!Chord.IsValidChord()) { return; }

	OutInputSequence.Reserve(OutInputSequence.Num() + Chord.bCtrl + Chord.bAlt + Chord.bShift + Chord.bCmd + 1);

	if (Chord.bCtrl) { OutInputSequence.Add(PairedModifiers::Control); }
	if (Chord.bAlt) { OutInputSequence.Add(PairedModifiers::Alt); }
	if (Chord.bShift) { OutInputSequence.Add(PairedModifiers::Shift); }
	if (Chord.bCmd) { OutInputSequence.Add(PairedModifiers::Command); }

	OutInputSequence.Emplace(Chord.Key);
}

void FDevInputShortcutBinding::PopulateInputSequence(FDevInputSequence& OutInputSequence) const
{
	if (!Shortcut.IsValid()) { return; }

	FDevInputShortcutBuilder::FromName(Shortcut.GetName(), OutInputSequence);
}
