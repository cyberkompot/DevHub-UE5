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

#endif // UE_VERSION_AT_LEAST(5, 2, 0)

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

#endif // UE_VERSION_AT_LEAST(5, 6, 0)

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

#endif // UE_VERSION_AT_LEAST(5, 7, 0)

#if UE_VERSION_AT_LEAST(5, 8, 0)

template<bool Predicate,typename TrueClass,typename FalseClass>
class TChooseClass;

template<typename TrueClass,typename FalseClass>
class TChooseClass<true,TrueClass,FalseClass>
{
public:
	typedef TrueClass Result;
};

template<typename TrueClass,typename FalseClass>
class TChooseClass<false,TrueClass,FalseClass>
{
public:
	typedef FalseClass Result;
};

template <typename T>
struct TIsMemberPointer
{
	enum { Value = false };
};

template <typename T, typename U> struct TIsMemberPointer<T U::*> { enum { Value = true }; };

template <typename T> struct TIsMemberPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsMemberPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsMemberPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };

#else

#include "Templates/ChooseClass.h"
#include "Templates/IsMemberPointer.h"

#endif // UE_VERSION_AT_LEAST(5, 8, 0)
