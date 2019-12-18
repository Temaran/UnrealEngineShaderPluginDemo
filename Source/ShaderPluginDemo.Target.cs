// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShaderPluginDemoTarget : TargetRules
{
	public ShaderPluginDemoTarget(TargetInfo Target)
        : base(Target)
	{
		Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("ShaderPluginDemo");
    }
}
