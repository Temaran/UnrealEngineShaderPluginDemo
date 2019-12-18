// Copyright 2019 Cadic AB. All Rights Reserved.
// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

using UnrealBuildTool;

public class ShaderPluginDemo : ModuleRules
{
	public ShaderPluginDemo(ReadOnlyTargetRules Target) 
        : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"RHI", 
			"PixelShader", 
			"ComputeShader" 
		});
	}
}
