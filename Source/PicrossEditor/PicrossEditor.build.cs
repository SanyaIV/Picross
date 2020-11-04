using UnrealBuildTool;

public class PicrossEditor : ModuleRules
{
    public PicrossEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.NoSharedPCHs;

        PrivatePCHHeaderFile = "Public/PicrossEditor.h";

        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Picross", "Array3D", "UnrealEd" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(
            new string[]
            {
                "PicrossEditor/Public",
                "Picross"
            });

        PrivateIncludePaths.AddRange(
            new string[]
            {
                "PicrossEditor/Private"
            });
    }
}