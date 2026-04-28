// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevInputTypes.h"

using FDevInputShortcutStringBuilder = TStringBuilder<NAME_SIZE>;

struct FDevInputShortcutBuilder final
{
	/** Parses a name representation of input shortcuts into the input tokens sequence, for example: Ctrl+Shift+Z,R | L1+R1. */
	static DEVINPUTS_API void FromName(const FName InShortcutName, FDevInputSequence &OutInputTokens);

	/** Generates a string name of shortcuts base on the input tokens sequence, for example: Ctrl+Shift+Z,R | L1+R1. */
	static DEVINPUTS_API FName ToName(const FDevInputSequence& InInputTokens, const EDevInputDisplayNameLength InDisplayNameLenght = EDevInputDisplayNameLength::Short);

	static DEVINPUTS_API FName Roundtrip(const FName& InShortcutName, const EDevInputDisplayNameLength InDisplayNameLenght = EDevInputDisplayNameLength::Short);
};

struct DEVINPUTS_API FDevInputShortcutReader final
{
	explicit FDevInputShortcutReader(const FName InShortcutName);

	void operator ++();
	FORCEINLINE explicit operator bool() const { return (!StringView.IsEmpty() || !Token.IsNone()); }
	FORCEINLINE FDevInputToken operator *() const { return Token; }
	FORCEINLINE FDevInputToken operator ->() const { return Token; }

private:
	FString Container;
	FStringView StringView;
	FDevInputToken Token = EDevInputTokens::None;
	int32 TokesCount = 0;
};

struct DEVINPUTS_API FDevInputShortcutWriter final
{
	explicit FDevInputShortcutWriter() = default;
	explicit FDevInputShortcutWriter(const EDevInputDisplayNameLength InNameLenght)
		: DisplayNameLenght(InNameLenght) {};

	EDevInputDisplayNameLength DisplayNameLenght = EDevInputDisplayNameLength::Short;

	FORCEINLINE FName ToName() const { return FName(StringBuilder); }

	FDevInputShortcutWriter& operator <<(const FDevInputToken& InToken);
	FDevInputShortcutWriter& operator <<(const FDevInputSequence& InInputTokens);

private:
	FDevInputShortcutStringBuilder StringBuilder = {};
};
