// Copyright 2016-2020 Cadic AB. All Rights Reserved.
// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

namespace UnrealBuildTool.Rules
{
	public class ShaderDeclarationDemo : ModuleRules
	{
		public ShaderDeclarationDemo(ReadOnlyTargetRules Target)
			: base(Target)
		{			
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			
			PrivateIncludePaths.AddRange(new string[] 
			{
				"ShaderDeclarationDemo/Private"
			});

			PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Renderer",
                "RenderCore",
                "RHI",
                "Projects"
			});
		}
	}
}