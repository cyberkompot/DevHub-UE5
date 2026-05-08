// Copyright (c) Alexandr Pereverzev.

using System.Linq;
using UnrealBuildTool;

public class DevEditor : ModuleRules
{
    public DevEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        if (Target.GlobalDefinitions.Contains("DEVHUB_NO_PCH"))
        {
            PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
        }

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "DevActions",
                "DevCore",
                "DevInputs",
                "DevMenus",
                "DevPad",
                "AssetTools",
                "Core",
                "CoreUObject",
                "UnrealEd",
            }
        );
    }
}