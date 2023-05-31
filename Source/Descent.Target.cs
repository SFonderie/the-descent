using UnrealBuildTool;

public class DescentTarget : TargetRules
{
	public DescentTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;

		ExtraModuleNames.Add("DescentCore");
	}
}
