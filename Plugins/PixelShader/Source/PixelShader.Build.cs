namespace UnrealBuildTool.Rules
{
	public class PixelShader : ModuleRules
	{
		public PixelShader(TargetInfo Target)
        {
            PrivateIncludePaths.AddRange(
                new string[] {
					"PixelShader/Private"
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