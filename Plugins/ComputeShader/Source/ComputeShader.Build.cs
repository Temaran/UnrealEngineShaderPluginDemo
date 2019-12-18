namespace UnrealBuildTool.Rules
{
	public class ComputeShader : ModuleRules
	{
		public ComputeShader(ReadOnlyTargetRules Target)
			: base(Target)
        {			
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
            PrivateIncludePaths.AddRange(new string[] 
			{
				"Private"
			});

			PublicDependencyModuleNames.AddRange(new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
                "RenderCore",
                "ShaderCore",
                "RHI"
			});
		}
	}
}