// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevEditor : ModuleRules
{
    public DevEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "DevInputs",
                "DevMenus",
                "AssetTools",
                "Core",
                "CoreUObject",
                "UnrealEd",
            }
        );
    }
}