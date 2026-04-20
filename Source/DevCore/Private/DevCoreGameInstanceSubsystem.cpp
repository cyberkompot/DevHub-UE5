// Copyright (c) Alexandr Pereverzev.

#include "DevCoreGameInstanceSubsystem.h"

#include "DevCore.h"

bool UDevCoreGameInstanceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if DEV_HUB_AVAILABLE

	if (!Super::ShouldCreateSubsystem(Outer)) { return false; }

	if (!GIsClient) { return false; }
	if (IsRunningCommandlet() || IsRunningDedicatedServer()) { return false; }

	if (!FSlateApplication::IsInitialized()) { return false; }

	const UWorld* World = Outer->GetWorld();
	if (!World) { return false; }

	const TEnumAsByte<EWorldType::Type> WorldType = World->WorldType;
	if (WorldType != EWorldType::Game && WorldType != EWorldType::PIE) { return false; }

	return true;

#else

	return false;

#endif // DEV_HUB_AVAILABLE
}
