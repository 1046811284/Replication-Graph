using UnrealBuildTool;

public class RepGraph : ModuleRules
{
	public RepGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        //ÃÌº”ƒ£øÈ“¿¿µ:
		PrivateDependencyModuleNames.AddRange(new string[] { "ReplicationGraph" });
	}
}
