// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevConsole : ModuleRules
{
    public DevConsole(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "DevCore",
                "CoreUObject",
                "Engine",
            }
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        PrivateIncludePaths.Add(System.IO.Path.Combine(EngineDirectory, "Source/Runtime/Core/Private"));
    }
}