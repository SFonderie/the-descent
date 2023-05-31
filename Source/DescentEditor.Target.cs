using UnrealBuildTool;

public class DescentEditorTarget : TargetRules
{
	public DescentEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;

		ExtraModuleNames.Add("DescentCore");
	}
}
