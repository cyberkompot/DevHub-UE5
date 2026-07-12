// Copyright (c) Alexandr Pereverzev.

#include "DevMenuQueries.h"

#include "DevMenuUtils.h"

void FDevMenuGetEntriesQuery::ExecuteQuery(IDevMenuEntriesIterator& InEntriesIterator) const
{
	if (!QueryDelegate.IsBound()) { return; }

	for (/** Nop */; InEntriesIterator; ++InEntriesIterator)
	{
		if (IDevMenuEntry* Entry = *InEntriesIterator)
		{
			ProcessEntry(Entry);
		}
	}
}

void FDevMenuGetEntriesQuery::ProcessEntry(IDevMenuEntry* InEntry) const
{
	const FDevMenuQueryResult QueryResult = FDevMenuQueryResultFactory::CreateResult(InEntry, OuterQueryResult);

	if (!!(QueryDepth & EDevMenuQueryDepth::TopLevelEntries))
	{
		QueryDelegate.Execute(QueryResult);
	}

	if (const EDevMenuQueryDepth SubEntriesDepth = (QueryDepth & EDevMenuQueryDepth::AnyLevelSubEntries); !!SubEntriesDepth)
	{
		const EDevMenuQueryDepth SubQueryDepth = (SubEntriesDepth == EDevMenuQueryDepth::AnyLevelSubEntries)
			? EDevMenuQueryDepth::AllEntries
			: EDevMenuQueryDepth::TopLevelEntries;
		InEntry->QuerySubEntries(FDevMenuGetEntriesQuery(QueryResult, SubQueryDepth, QueryDelegate));
	}
}


void FDevMenuGetEntriesByPathQuery::ExecuteQuery(IDevMenuEntriesIterator& InEntriesIterator) const
{
	if (!QueryDelegate.IsBound()) { return; }

	if (QueryPath.IsEmpty()) { Super::ExecuteQuery(InEntriesIterator); return; }

	for (/** Nop */; InEntriesIterator; ++InEntriesIterator)
	{
		if (IDevMenuEntry* Entry = *InEntriesIterator)
		{
			// No allocations while converting FName fot FString.
			FNameBuilder EntryAddressBuilder;
			Entry->GetEntryName().AppendString(EntryAddressBuilder);
			const FStringView EntryAddress = EntryAddressBuilder.ToView();

			if (const EMatchResult MatchResult = IsAddressMatch(EntryAddress); MatchResult == EMatchResult::PartialMatch)
			{
				FDevMenuQueryResult QueryResult = FDevMenuQueryResultFactory::CreateResult(Entry, OuterQueryResult);
				FString SubPath = MakeSubPath(EntryAddress);
				FDevMenuGetEntriesByPathQuery SubQuery(QueryResult, SubPath, QueryDepth, QueryDelegate);
				Entry->QuerySubEntries(SubQuery);
			}
			else if (MatchResult == EMatchResult::ExactMatch)
			{
				ProcessEntry(Entry);
			}
		}
	}
}

FDevMenuGetEntriesByPathQuery::EMatchResult FDevMenuGetEntriesByPathQuery::IsAddressMatch(const FStringView& InEntryName) const
{
	if (const FStringView QueryPathStart(*QueryPath, FMath::Min(QueryPath.Len(), InEntryName.Len())); QueryPathStart == InEntryName)
	{
		// Name equals the path or represents a valid part of the path requested in the query.
		const int32 QueryLen = QueryPath.Len();
		const int32 EntryNameLen = InEntryName.Len();
		if (EntryNameLen == QueryLen)
		{
			return EMatchResult::ExactMatch;
		}
		if (QueryPath[EntryNameLen] == FDevMenuPaths::PathDelimiter)
		{
			return EMatchResult::PartialMatch;
		}
	}

	return EMatchResult::NoMatch;
}

FString FDevMenuGetEntriesByPathQuery::MakeSubPath(const FStringView& InEntryName) const
{
	return FString(QueryPath.RightChop(InEntryName.Len() + 1));
}


FDevMenuQueryResult FDevMenuQueryResultFactory::CreateResult(IDevMenuEntry* InEntry, const FDevMenuQueryResult& InOuterQueryResult)
{
	UDevMenu* Menu = InEntry->CastTo<UDevMenu>();
	return (Menu)
		? FDevMenuQueryResult(Menu, nullptr, InEntry, InEntry->GetEntryName().ToString())
		: FDevMenuQueryResult(InOuterQueryResult.Menu, InOuterQueryResult.Outer, InEntry, FDevMenuPaths::Combine(InOuterQueryResult.Path, InEntry->GetEntryName().ToString()));
}
