using UnrealBuildTool;

public class RepGraph : ModuleRules
{
	public RepGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        //���ģ������:
		PrivateDependencyModuleNames.AddRange(new string[] { "ReplicationGraph" });
	}
}
