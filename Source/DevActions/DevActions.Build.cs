// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevActions : ModuleRules
{
	public DevActions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
