using UnrealBuildTool;

public class DescentCore : ModuleRules
{
	public DescentCore(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable Include-What-You-Use in the module.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// CORE ENGINE MODULES
		PublicDependencyModuleNames.Add("Core");
		PublicDependencyModuleNames.Add("CoreUObject");
		PublicDependencyModuleNames.Add("Engine");
	}
}
