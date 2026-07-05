// Copyright (c) Alexandr Pereverzev.

using System.Linq;
using UnrealBuildTool;

public class DevInputs : ModuleRules
{
	public DevInputs(ReadOnlyTargetRules Target) : base(Target)
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
				"CommonInput",
				"Core",
				"CoreUObject",
				"Engine",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"InputCore",
				"Slate",
				"SlateCore",
			});
	}
}
