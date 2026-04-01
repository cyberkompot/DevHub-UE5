// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h" // Included for log macros.
#include "UObject/ObjectMacros.h"
#include "UObject/ReflectedTypeAccessors.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDevInputs, VeryVerbose, All);

namespace DevInput::Logging
{
	template<typename EnumType>
	FString EnumToString(EnumType InEnumValue)
	{
		return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue));
	}

	template<typename TTokenType>
	FString TokensToString(const TArray<TTokenType>& InTokens)
	{
		TStringBuilder<1024> SB;
		for (const TTokenType& Token : InTokens)
		{
			if (SB.Len()) { SB.Append(", "); }
			SB.Append(Token.ToString());
		}
		return SB.ToString();
	}
} // namespace DevInput::Logging
