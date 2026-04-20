// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreCompatibility.h"
#include "UObject/ObjectPtr.h"

#if UE_COMPATIBILITY_OBJECT_PTR_DECAY

namespace DevCore::Utils::ObjectPtr
{
	/* Returns const access to the underlying storage of TObjectPtr or its containers (use when raw view is needed). */
	template <typename T,
		typename DecayTraits = typename UE::Core::Private::TObjectPtrDecayTypeOf<T>,
		typename U = typename DecayTraits::Type>
	[[nodiscard]] const U& ObjectPtrDecay(const T& Value)
	{
		return ::ObjectPtrDecay(Value);
	}

	/* Wraps raw storage back into TObjectPtr form (inverse of ObjectPtrDecay). */
	template <typename T,
		typename U = typename UE::Core::Private::TObjectPtrWrapTypeOf<T>::Type>
	[[nodiscard]] U& ObjectPtrWrap(T& Value)
	{
		return ::ObjectPtrWrap(Value);
	}

	template <typename T,
		typename U = typename UE::Core::Private::TObjectPtrWrapTypeOf<T>::Type>
	[[nodiscard]] const U& ObjectPtrWrap(const T& Value)
	{
		return ::ObjectPtrWrap(Value);
	}
}

#else // UE_COMPATIBILITY_OBJECT_PTR_DECAY

namespace DevCore::Utils::ObjectPtr::Private
{
	template <typename T>
	struct TObjectPtrDecayTypeOf
	{
		using Type = T;
		static FORCEINLINE void PerformDecayActions(const T&) { /** Nop. */ }
	};

	template <typename T>
	struct TObjectPtrDecayTypeOf<TObjectPtr<T>>
	{
		using Type = T*;

		static FORCEINLINE void PerformDecayActions(const TObjectPtr<T>& Value)
		{
#if UE_WITH_OBJECT_HANDLE_LATE_RESOLVE || UE_WITH_OBJECT_HANDLE_TRACKING
			(void)Value.Get();
#endif
		}
	};

	template <typename T>
	struct TObjectPtrDecayTypeOf<TSet<T>>
	{
		using Type = TSet<typename TObjectPtrDecayTypeOf<T>::Type>;

		static FORCEINLINE void PerformDecayActions(const TSet<T>& Value)
		{
			for (const auto& V : Value)
			{
				TObjectPtrDecayTypeOf<T>::PerformDecayActions(V);
			}
		}
	};

	template <typename K, typename V>
	struct TObjectPtrDecayTypeOf<TMap<K, V>>
	{
		using Type = TMap<typename TObjectPtrDecayTypeOf<K>::Type, typename TObjectPtrDecayTypeOf<V>::Type>;

		static FORCEINLINE void PerformDecayActions(const TMap<K, V>& Value)
		{
			for (const auto& KV : Value)
			{
				TObjectPtrDecayTypeOf<K>::PerformDecayActions(KV.Key);
				TObjectPtrDecayTypeOf<V>::PerformDecayActions(KV.Value);
			}
		}
	};

	template <typename T>
	struct TObjectPtrDecayTypeOf<TArray<T>>
	{
		using Type = TArray<typename TObjectPtrDecayTypeOf<T>::Type>;

		static FORCEINLINE void PerformDecayActions(const TArray<T>& Value)
		{
			for (const auto& V : Value)
			{
				TObjectPtrDecayTypeOf<T>::PerformDecayActions(V);
			}
		}
	};

	template <typename T>
	struct TObjectPtrWrapTypeOf
	{
		using Type = T;
	};

	template <typename T>
	struct TObjectPtrWrapTypeOf<T*>
	{
		using Type = TObjectPtr<T>;
	};

	template <typename T>
	struct TObjectPtrWrapTypeOf<TArrayView<T>>
	{
		using Type = TArrayView<typename TObjectPtrWrapTypeOf<T>::Type>;
	};

	template <typename T>
	struct TObjectPtrWrapTypeOf<TArray<T>>
	{
		using Type = TArray<typename TObjectPtrWrapTypeOf<T>::Type>;
	};

	template <typename T>
	struct TObjectPtrWrapTypeOf<TSet<T>>
	{
		using Type = TSet<typename TObjectPtrWrapTypeOf<T>::Type>;
	};

	template <typename K, typename V>
	struct TObjectPtrWrapTypeOf<TMap<K, V>>
	{
		using Type = TMap<typename TObjectPtrWrapTypeOf<K>::Type, typename TObjectPtrWrapTypeOf<V>::Type>;
	};

	namespace Unsafe
	{
		template <typename T,
			typename DecayTraits = TObjectPtrDecayTypeOf<T>,
			typename U = typename DecayTraits::Type>
		[[nodiscard]] U& Decay(T& A)
		{
			DecayTraits::PerformDecayActions(A);
			return reinterpret_cast<U&>(A);
		}
	}
}

namespace DevCore::Utils::ObjectPtr
{
	/* Returns const access to the underlying storage of TObjectPtr or its containers (use when raw view is needed). */
	template <typename T,
		typename DecayTraits = typename DevCore::Utils::ObjectPtr::Private::TObjectPtrDecayTypeOf<T>,
		typename U = typename DecayTraits::Type>
	[[nodiscard]] const U& ObjectPtrDecay(const T& Value)
	{
		DecayTraits::PerformDecayActions(Value);
		return reinterpret_cast<const U&>(Value);
	}

	/* Wraps raw storage back into TObjectPtr form (inverse of ObjectPtrDecay). */
	template <typename T,
		typename U = typename DevCore::Utils::ObjectPtr::Private::TObjectPtrWrapTypeOf<T>::Type>
	[[nodiscard]] U& ObjectPtrWrap(T& Value)
	{
		return reinterpret_cast<U&>(Value);
	}

	/* Wraps raw storage back into TObjectPtr form (inverse of ObjectPtrDecay). */
	template <typename T,
		typename U = typename DevCore::Utils::ObjectPtr::Private::TObjectPtrWrapTypeOf<T>::Type>
	[[nodiscard]] const U& ObjectPtrWrap(const T& Value)
	{
		return reinterpret_cast<const U&>(Value);
	}
}

#endif // UE_COMPATIBILITY_OBJECT_PTR_DECAY
