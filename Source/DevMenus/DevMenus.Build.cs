// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevMenus : ModuleRules
{
	public DevMenus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"DevCore",
				"DevActions",
				"DevInputs",
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"StructUtils",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"InputCore",
				"StructUtils",
				"UMG",
				"CommonUI",
				"DeveloperSettings",
			});
	}
}
