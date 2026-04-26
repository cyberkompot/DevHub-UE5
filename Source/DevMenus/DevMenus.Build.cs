// Copyright (c) Alexandr Pereverzev.

using System.Linq;
using UnrealBuildTool;

public class DevMenus : ModuleRules
{
	public DevMenus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		if (Target.GlobalDefinitions.Contains("DEVHUB_NO_PCH"))
		{
			PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
		}

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
