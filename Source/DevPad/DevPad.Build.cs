using UnrealBuildTool;

public class DevPad : ModuleRules
{
    public DevPad(ReadOnlyTargetRules Target) : base(Target)
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
                "DevActions",
                "DevInputs",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "StructUtils",
                "UMG",
            }
        );
    }
}