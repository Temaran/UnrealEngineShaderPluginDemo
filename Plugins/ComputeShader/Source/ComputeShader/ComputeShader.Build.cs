namespace UnrealBuildTool.Rules
{
	public class ComputeShader : ModuleRules
	{
		public ComputeShader(ReadOnlyTargetRules Target) : base(Target)
        {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/ComputeShaderPrivatePCH.h";

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

                PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                        "Projects",
                        // ... add private dependencies that you statically link with here ...
                    }
                );
		}
	}
}
