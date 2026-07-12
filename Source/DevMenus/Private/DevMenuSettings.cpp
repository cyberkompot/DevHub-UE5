// Copyright (c) Alexandr Pereverzev.

#include "DevMenuSettings.h"

#include "DevMenuFactory.h"

using namespace FDevMenuFactory;

UDevMenuSettings::UDevMenuSettings()
{
	MainMenuEntries = FDevMenuInstancedEntries
	{
		CreateSubMenu("File"),
		CreateSubMenu("Debug"),
		CreateSubMenu("Profiling"),
		CreateSubMenu("Gameplay"),
		CreateSubMenu("Animation"),
		CreateSubMenu("Rendering"),
	};
}

#if WITH_EDITOR

void UDevMenuSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (FDevMenuInstancedEntry& InstancedEntry : MainMenuEntries)
	{
		if (FDevMenuEntry* Entry = InstancedEntry.GetMutablePtr<FDevMenuEntry>(); Entry)
		{
			Entry->ResolveEntryPath();
		}
	}
}

#endif // #if WITH_EDITOR
