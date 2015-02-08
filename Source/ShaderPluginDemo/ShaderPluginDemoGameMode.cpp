// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "ShaderPluginDemo.h"
#include "ShaderPluginDemoGameMode.h"
#include "ShaderPluginDemoHUD.h"
#include "ShaderPluginDemoCharacter.h"

AShaderPluginDemoGameMode::AShaderPluginDemoGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/ShaderPluginDemo/PlayerCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AShaderPluginDemoHUD::StaticClass();
}
