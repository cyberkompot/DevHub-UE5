// Copyright (c) Alexandr Pereverzev.

using UnrealBuildTool;

public class DevCore : ModuleRules
{
    public DevCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );
    }
}