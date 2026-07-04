// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"

#if UE_VERSION_AT_LEAST(5, 2, 0)

template <typename A, typename B>
struct TIsSame
{
	enum { Value = false };
};

template <typename T>
struct TIsSame<T, T>
{
	enum { Value = true };
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

#if UE_VERSION_AT_LEAST(5, 6, 0)

template <typename T>
struct TRemoveConst
{
	using Type = T;
};

template <typename T>
struct TRemoveConst<const T>
{
	using Type = T;
};

#endif
