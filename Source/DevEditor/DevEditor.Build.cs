// Copyright (c) Alexandr Pereverzev.

using System.Linq;
using UnrealBuildTool;

public class DevEditor : ModuleRules
{
    public DevEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        if (Target.GlobalDefinitions.Contains("NO_PCH"))
        {
            PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
        }

        PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Editor")); // Required for ClassBag compatibility across UE 5.1-5.6.

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
                "Engine",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "StructUtils",
                "StructUtilsEditor",
                "UnrealEd",
            }
        );
    }
}