// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShaderPluginDemoEditorTarget : TargetRules
{
	public ShaderPluginDemoEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
        ExtraModuleNames.Add("ShaderPluginDemo");
    }

}
