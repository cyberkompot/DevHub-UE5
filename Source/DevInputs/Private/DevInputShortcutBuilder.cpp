// Copyright (c) Alexandr Pereverzev.

#include "DevInputShortcutBuilder.h"

#include "Shader/ShaderTypes.h"

void FDevInputShortcutBuilder::FromName(const FName InShortcutName, FDevInputSequence& OutInputTokens)
{
	for (FDevInputShortcutReader Reader(InShortcutName); Reader; ++Reader)
	{
		OutInputTokens.Add(*Reader);
	}
}

FName FDevInputShortcutBuilder::ToName(const FDevInputSequence& InInputTokens, const EDevInputDisplayNameLength InDisplayNameLenght)
{
	FDevInputShortcutWriter Writer;
	Writer.DisplayNameLenght = InDisplayNameLenght;
	Writer << InInputTokens;
	return Writer.ToName();
}

FName FDevInputShortcutBuilder::Roundtrip(const FName& InShortcutName, const EDevInputDisplayNameLength InDisplayNameLenght)
{
	FDevInputShortcutWriter Writer(InDisplayNameLenght);
	for (FDevInputShortcutReader Reader(InShortcutName); Reader; ++Reader)
	{
		Writer << *Reader;
	}
	return Writer.ToName();
}

FDevInputShortcutReader::FDevInputShortcutReader(const FName InShortcutName)
	: Container(InShortcutName.ToString()), StringView(FStringView(Container).TrimStartAndEnd())
{
	++*this;
}

void FDevInputShortcutReader::operator ++()
{
	Token = EDevInputTokens::None;

	const int32 Len = StringView.Len();
	if (Len == 0) { return; }

	int32 Index = 0;
	if (TokesCount % 2)
	{
		// ----Input shortcut cannot begin with a special token.
		switch (StringView[Index])
		{
			case '+': Token = EDevInputTokens::Special::Plus; break;
			case ',': Token = EDevInputTokens::Special::Coma; break;
			case '|': Token = EDevInputTokens::Special::Or; break;
			default: break;
		}
		++Index;
	}

	if (Token.IsNone())
	{
		while (Index < Len)
		{
			if (const FStringView::ElementType Char = StringView[Index]; Char == '+' || Char == ',' || Char == '|') { break; }
			++Index;
		}
		if (const FStringView Value = StringView.Left(Index).TrimStartAndEnd(); Value.Len())
		{
			Token = FDevInputToken(EDevInputTokens::FindToken(Value));
			if (Token.IsNone())
			{
				Token = FName(Value);
			}
		}
	}

	StringView = StringView.RightChop(Index);
	++TokesCount;
}

FDevInputShortcutWriter& FDevInputShortcutWriter::operator <<(const FDevInputToken& InToken)
{
	if (InToken == EDevInputTokens::Special::Plus)
	{
		StringBuilder.Append(TEXT("+"));
	}
	else if (InToken == EDevInputTokens::Special::Coma)
	{
		StringBuilder.Append(TEXT(", "));
	}
	else if (InToken == EDevInputTokens::Special::Or)
	{
		StringBuilder.Append(TEXT(" | "));
	}
	else
	{
		StringBuilder.Append(InToken.ToString(DisplayNameLenght));
	}
	return *this;
}

FDevInputShortcutWriter& FDevInputShortcutWriter::operator <<(const FDevInputSequence& InInputTokens)
{
	for (const FDevInputToken& Token : InInputTokens)
	{
		*this << Token;
	}
	return *this;
}
