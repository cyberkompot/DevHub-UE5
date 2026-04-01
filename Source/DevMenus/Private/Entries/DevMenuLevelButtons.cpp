// Copyright (c) Alexandr Pereverzev.

#include "Entries/DevMenuLevelButtons.h"

#include "DevMenuFactory.h"
#include "DevMenuIterators.h"
#include "Actions/DevActionConsoleCommand.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"

#define UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS UE_VERSION_AT_LEAST(5, 1, 0)

FDevMenuInstancedEntries FDevMenuLoadLevelGroup::CreateDynamicEntries() const
{
	FDevMenuInstancedEntries DynamicEntries;

	const IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry) { return DynamicEntries; }

	FARFilter LevelsFilter;
#if UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
	LevelsFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
#else
	LevelsFilter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#endif // UE_COMPATIBILITY_FAR_FILTER_CLASS_PATHS
	LevelsFilter.bRecursiveClasses = true;

	AssetRegistry->EnumerateAssets(LevelsFilter, [this, &DynamicEntries](const FAssetData& AssetData)
	{
		if (const FString PackagePath = AssetData.PackagePath.ToString();
			IsPackageUnderDirectories(PackagePath, IncludedPaths, true)
			&& !IsPackageUnderDirectories(PackagePath, ExcludePaths, false))
		{
			DynamicEntries.Emplace(FDevMenuFactory::CreateActionButton<FDevActionConsoleCommand>(
				AssetData.PackageName,
				FText::FromName(AssetData.PackageName),
				FDevActionConsoleCommand(TEXT("Open"), AssetData.PackageName.ToString())));
		}
		return true;
	});

	struct FSortByEntryName
	{
		FORCEINLINE bool operator ()(const FDevMenuInstancedEntry& A, const FDevMenuInstancedEntry& B) const { return A.Get<FDevMenuActionButton>().EntryName.LexicalLess(B.Get<FDevMenuActionButton>().EntryName); }
	};
	DynamicEntries.Sort(FSortByEntryName());

	return DynamicEntries;
}

bool FDevMenuLoadLevelGroup::IsPackageUnderDirectories(const FString& InPackagePath, const TArray<FString>& InDirectories, const bool InResultForEmptyDirectories) const
{
	if (InDirectories.IsEmpty()) { return InResultForEmptyDirectories; }

	for (const FString& Directory : InDirectories)
	{
		if (FPaths::IsUnderDirectory(InPackagePath, Directory))
		{
			return true;
		}
	}
	return false;
}
