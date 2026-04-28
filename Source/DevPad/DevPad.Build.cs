using System.Linq;
using UnrealBuildTool;

public class DevPad : ModuleRules
{
    public DevPad(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        if (Target.GlobalDefinitions.Contains("DEVHUB_NO_PCH"))
        {
            PCHUsage = ModuleRules.PCHUsageMode.NoPCHs;
        }

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
                "DevMenus",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "InputCore",
                "StructUtils",
                "UMG",
            }
        );
    }
}