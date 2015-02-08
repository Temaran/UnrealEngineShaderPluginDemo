namespace UnrealBuildTool.Rules
{
	public class ComputeShader : ModuleRules
	{
		public ComputeShader(TargetInfo Target)
        {
            PrivateIncludePaths.AddRange(
                new string[] {
					"ComputeShader/Private"
				}
                );

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "RenderCore",
                    "ShaderCore",
                    "RHI"
				}
				);
		}
	}
}