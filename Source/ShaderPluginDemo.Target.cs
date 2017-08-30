// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShaderPluginDemoTarget : TargetRules
{
	public ShaderPluginDemoTarget(TargetInfo Target)
	{
		Type = TargetType.Game;
        ExtraModuleNames.Add("ShaderPluginDemo");
    }

}
