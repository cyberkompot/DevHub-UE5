// Copyright (c) Alexandr Pereverzev.

#pragma once

struct EDevMenuPaths final
{
	DEVMENUS_API static const FName None;
	DEVMENUS_API static const FName Undefined;
	DEVMENUS_API static const FName MainMenu;
	DEVMENUS_API static const FName ContextMenu;
};

struct FDevMenuPaths final
{
	DEVMENUS_API static constexpr TCHAR PathDelimiter = '.';

	DEVMENUS_API FORCEINLINE static bool IsName(const FString& InEntryPath) { return (!IsPath(InEntryPath)); }
	DEVMENUS_API FORCEINLINE static bool IsPath(const FString& InEntryPath) { int32 Index; return InEntryPath.FindChar(PathDelimiter, Index); }

	DEVMENUS_API static FString GetRootDirectory(const FString& InEntryPath);
	DEVMENUS_API static FString GetDirectory(const FString& InEntryPath);
	DEVMENUS_API static FString GetName(const FString& InEntryPath);

	DEVMENUS_API static int32 GetPathDepth(const FName& InEntryPath);
	DEVMENUS_API static int32 GetPathDepth(const FString& InEntryPath);

	DEVMENUS_API static FName Combine(const FName& InDirectoryPath, const FName& InEntryName);
	DEVMENUS_API static FString Combine(const FString& InDirectoryPath, const FString& InEntryName);

	DEVMENUS_API static void ExtractRootDirectory(const FString& InEntryPath, FString& OutRootDirectoryPath, FString& OutRemainingPath);
	DEVMENUS_API static void ExtractDirectory(const FString& InEntryPath, FString& OutDirectoryPath, FString& OutRemainingPath);

	DEVMENUS_API static FString TrimPath(const FStringView& InEntryPath);
};

struct FDevMenuUtils final
{
	/** Takes an entry name and breaks it down into a human-readable text. */
	FORCEINLINE static FText EntryNameToDisplayText(const FName InEntryName) { return EntryNameToDisplayText(InEntryName.ToString()); }
	FORCEINLINE static FText EntryNameToDisplayText(const FString& InEntryName) { return FText::FromString(FName::NameToDisplayString(InEntryName, false)); }

	/** Takes an entry path, extracts the entry name, and breaks it down into a human-readable text. */
	FORCEINLINE static FText EntryPathToDisplayText(const FName InEntryPath) { return EntryPathToDisplayText(InEntryPath.ToString()); }
	FORCEINLINE static FText EntryPathToDisplayText(const FString& InEntryPath) { return EntryNameToDisplayText(FDevMenuPaths::GetName(InEntryPath)); }

	/**
	 * Returns the label as is. If the label is empty, the entry name is used instead and breaks it down into a human-readable text.
	 * If the entry name contains a sub-path (e.g., in the case of an embedded entry like "MaxFPS.60FPS"), only the name segment ("60FPS") is used for the resulting text.
	 */
	FORCEINLINE static FText LabelToDisplayText(const FName InEntryName, const FText& InLabel) { return (InLabel.IsEmpty()) ? EntryPathToDisplayText(InEntryName) : InLabel; }
	FORCEINLINE static FText LabelToDisplayText(const FString& InEntryName, const FText& InLabel) { return (InLabel.IsEmpty()) ? EntryPathToDisplayText(InEntryName) : InLabel; }
};
