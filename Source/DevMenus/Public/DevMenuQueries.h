// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevMenuTypes.h"

enum class EDevMenuQueryDepth : uint8
{
	TopLevelEntries = 1,
	FirstLevelSubEntries = 2,
	AnyLevelSubEntries = 4 | FirstLevelSubEntries,
	AllEntries = TopLevelEntries | AnyLevelSubEntries,
};

ENUM_CLASS_FLAGS(EDevMenuQueryDepth);


struct FDevMenuGetEntriesQuery : IDevMenuEntriesQuery
{
	FDevMenuGetEntriesQuery(const EDevMenuQueryDepth InQueryDepth, const FDevMenuQueryDelegate& InQueryDelegate)
		: FDevMenuGetEntriesQuery(FDevMenuQueryResult::Empty, InQueryDepth, InQueryDelegate) {}
	FDevMenuGetEntriesQuery(const FDevMenuQueryResult& InOuterQueryResult, const EDevMenuQueryDepth InQueryDepth, const FDevMenuQueryDelegate& InQueryDelegate)
		: OuterQueryResult(InOuterQueryResult), QueryDepth(InQueryDepth), QueryDelegate(InQueryDelegate) {}

	//~ Begin IDevMenusEntriesQuery interface.
	virtual void ExecuteQuery(IDevMenuEntriesIterator& InEntriesIterator) const override;
	//~ End IDevMenusEntriesQuery interface.

protected:
	FDevMenuQueryResult OuterQueryResult;
	EDevMenuQueryDepth QueryDepth;
	FDevMenuQueryDelegate QueryDelegate;

	void ProcessEntry(IDevMenuEntry* InEntry) const;
};

struct FDevMenuGetEntriesByPathQuery : FDevMenuGetEntriesQuery
{
	FDevMenuGetEntriesByPathQuery(const FName InQueryPath, const EDevMenuQueryDepth InQueryDepth, const FDevMenuQueryDelegate& InQueryDelegate)
		: FDevMenuGetEntriesByPathQuery(InQueryPath.ToString(), InQueryDepth, InQueryDelegate) {}
	FDevMenuGetEntriesByPathQuery(const FString& InQueryPath, const EDevMenuQueryDepth InQueryDepth, const FDevMenuQueryDelegate& InQueryDelegate)
		: FDevMenuGetEntriesByPathQuery(FDevMenuQueryResult::Empty, InQueryPath, InQueryDepth, InQueryDelegate) {}
	FDevMenuGetEntriesByPathQuery(const FDevMenuQueryResult& InOuterQueryResult, const FString& InQueryPath, const EDevMenuQueryDepth InQueryDepth, const FDevMenuQueryDelegate& InQueryDelegate)
		: Super(InOuterQueryResult, InQueryDepth, InQueryDelegate), QueryPath(InQueryPath) {}

	//~ Begin IDevMenusEntriesQuery interface.
	virtual void ExecuteQuery(IDevMenuEntriesIterator& InEntriesIterator) const override;
	//~ End IDevMenusEntriesQuery interface.

protected:
	FString QueryPath;

	enum class EMatchResult : uint8
	{
		NoMatch = 0,
		PartialMatch = 1, // Name is a valid part of the path requested in the query.
		ExactMatch = 2, // Name is exactly matches path requested in the query.
	};

	EMatchResult IsAddressMatch(const FStringView& InEntryName) const;

	FString MakeSubPath(const FStringView& InEntryName) const;

private:
	using Super = FDevMenuGetEntriesQuery;
};

struct FDevMenuQueryResultFactory final
{
	static FDevMenuQueryResult CreateResult(IDevMenuEntry* InEntry, const FDevMenuQueryResult& InOuterQueryResult);
};
