// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevInputs : ModuleRules
{
	public DevInputs(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"DevCore",
				"CommonInput",
				"Core",
				"CoreUObject",
				"Engine",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"InputCore",
				"Slate",
				"SlateCore",
			});
	}
}
