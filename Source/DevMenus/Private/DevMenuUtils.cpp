// Copyright (c) Alexandr Pereverzev.

#include "DevMenuUtils.h"

const FName EDevMenuPaths::None(NAME_None);
const FName EDevMenuPaths::Undefined("Undefined");
const FName EDevMenuPaths::MainMenu("MainMenu");
const FName EDevMenuPaths::ContextMenu("ContextMenu");

FString FDevMenuPaths::GetRootDirectory(const FString& InEntryPath)
{
	int32 Index; return (InEntryPath.FindChar(PathDelimiter, Index)) ? InEntryPath.Left(Index) : FString();
}

FString FDevMenuPaths::GetDirectory(const FString& InEntryPath)
{
	int32 Index; return (InEntryPath.FindLastChar(PathDelimiter, Index)) ? InEntryPath.Left(Index) : FString();
}

FString FDevMenuPaths::GetName(const FString& InEntryPath)
{
	int32 Index; return (InEntryPath.FindLastChar(PathDelimiter, Index)) ? InEntryPath.Mid(Index + 1) : InEntryPath;
}

int32 FDevMenuPaths::GetPathDepth(const FName& InEntryPath)
{
	TStringBuilder<NAME_SIZE> EntryPathBuilder;
	InEntryPath.AppendString(EntryPathBuilder);

	int32 Count = 0;
	for (int32 i = 0, Num = EntryPathBuilder.Len(); i < Num; ++i)
	{
		if (EntryPathBuilder.GetData()[i] == PathDelimiter)
		{
			++Count;
		}
	}
	return Count;
}

int32 FDevMenuPaths::GetPathDepth(const FString& InEntryPath)
{
	int32 Count = 0;
	for (const TCHAR Char : InEntryPath)
	{
		if (Char == PathDelimiter)
		{
			++Count;
		}
	}
	return Count;
}

FName FDevMenuPaths::Combine(const FName& InDirectoryPath, const FName& InEntryName)
{
	if (!InDirectoryPath.IsNone() && !InEntryName.IsNone())
	{
		FStringBuilderBase CombinedPath;
		InDirectoryPath.AppendString(CombinedPath);
		CombinedPath.AppendChar(PathDelimiter);
		InEntryName.AppendString(CombinedPath);
		return FName(CombinedPath);
	}
	else if (!InDirectoryPath.IsNone())
	{
		return InDirectoryPath;
	}
	else
	{
		return InEntryName;
	}
}

FString FDevMenuPaths::Combine(const FString& InDirectoryPath, const FString& InEntryName)
{
	if (InDirectoryPath.Len() && InEntryName.Len())
	{
		FString CombinedPath;
		CombinedPath.Reserve(InDirectoryPath.Len() + 1 + InEntryName.Len());
		CombinedPath += InDirectoryPath;
		CombinedPath += PathDelimiter;
		CombinedPath += InEntryName;
		return CombinedPath;
	}
	else if (InDirectoryPath.Len())
	{
		return InDirectoryPath;
	}
	else
	{
		return InEntryName;
	}
}

void FDevMenuPaths::ExtractRootDirectory(const FString& InEntryPath, FString& OutRootDirectoryPath, FString& OutRemainingPath)
{
	if (int32 Index; InEntryPath.FindChar(PathDelimiter, Index))
	{
		OutRootDirectoryPath = InEntryPath.Left(Index);
		OutRemainingPath = InEntryPath.Mid(Index + 1);
	}
	else
	{
		OutRootDirectoryPath = FString();
		OutRemainingPath = InEntryPath;
	}
}

void FDevMenuPaths::ExtractDirectory(const FString& InEntryPath, FString& OutDirectoryPath, FString& OutRemainingPath)
{
	if (int32 Index; InEntryPath.FindLastChar(PathDelimiter, Index))
	{
		OutDirectoryPath = InEntryPath.Left(Index);
		OutRemainingPath = InEntryPath.Mid(Index + 1);
	}
	else
	{
		OutDirectoryPath = FString();
		OutRemainingPath = InEntryPath;
	}
}

FString FDevMenuPaths::TrimPath(const FStringView& InEntryPath)
{
	FStringView TrimmedPath = InEntryPath.TrimStartAndEnd();

	int32 Index;
	while (TrimmedPath.FindLastChar(PathDelimiter, Index) && Index == TrimmedPath.Len() - 1)
	{
		TrimmedPath = TrimmedPath.LeftChop(1);
	}
	while (TrimmedPath.FindChar(PathDelimiter, Index) && Index == 0)
	{
		TrimmedPath = TrimmedPath.RightChop(1);
	}

	return FString(TrimmedPath);
}
