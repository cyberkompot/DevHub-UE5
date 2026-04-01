// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"

#if UE_VERSION_AT_LEAST(5, 6, 0)

template<class T>
struct TRemoveConst
{
	using Type = std::remove_const_t<T>;
};

#endif

#if UE_VERSION_AT_LEAST(5, 7, 0)

template <typename T>
struct TIsConst
{
	static constexpr bool Value = false;
};

template <typename T>
struct TIsConst<const T>
{
	static constexpr bool Value = true;
};

#endif
