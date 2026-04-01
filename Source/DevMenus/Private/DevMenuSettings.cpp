// Copyright (c) Alexandr Pereverzev.

#include "DevMenuSettings.h"

#include "DevMenuFactory.h"

using namespace FDevMenuFactory;

UDevMenuSettings::UDevMenuSettings()
{
	MainMenuEntries = FDevMenuInstancedEntries
	{
		WithId(1818759668, CreateSubMenu(TEXT("File"))),
		WithId(1836037887, CreateSubMenu(TEXT("Debug"))),
		WithId(1823948022, CreateSubMenu(TEXT("Profiling"))),
		WithId(1824029960, CreateSubMenu(TEXT("Gameplay"))),
		WithId(1824029980, CreateSubMenu(TEXT("Animation"))),
		WithId(1824030000, CreateSubMenu(TEXT("Rendering"))),
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
