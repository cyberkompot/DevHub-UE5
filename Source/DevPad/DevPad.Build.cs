using System.Linq;
using UnrealBuildTool;

public class DevPad : ModuleRules
{
    public DevPad(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        if (Target.GlobalDefinitions.Contains("NO_PCH"))
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
                "CommonUI",
                "CoreUObject",
                "DeveloperSettings",
                "Engine",
                "InputCore",
                "SlateCore",
                "StructUtils",
                "UMG",
            }
        );
    }
}