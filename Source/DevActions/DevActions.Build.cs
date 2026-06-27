// Copyright (c) Alexandr Pereverzev.

using System.Linq;
using UnrealBuildTool;

public class DevActions : ModuleRules
{
	public DevActions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		if (Target.GlobalDefinitions.Contains("NO_PCH"))
		{
			PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
		}

		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"DevCore",
				"Core",
				"CoreUObject",
				"Engine",
				"StructUtils",
				"Slate",
				"SlateCore",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"DevConsole",
			});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
