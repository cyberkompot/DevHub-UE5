// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "UObject/ReflectedTypeAccessors.h"

struct FDevCoreEnums final
{
	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	static TEnumType EnumFromString(const FString& Value)
	{
		const int64 ByName = StaticEnum<TEnumType>()->GetValueByNameString(Value, EGetByNameFlags::None);
		return (ByName != INDEX_NONE) ? static_cast<TEnumType>(ByName) : static_cast<TEnumType>(FCString::Atoi(*Value));
	}

	template<typename TEnumType, typename = typename TEnableIf<TIsEnum<TEnumType>::Value>::Type>
	static FString EnumToString(TEnumType Value)
	{
		return StaticEnum<TEnumType>()->GetNameStringByValue(static_cast<int64>(Value));
	}
};
