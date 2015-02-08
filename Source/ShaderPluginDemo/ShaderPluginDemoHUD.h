// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "ShaderPluginDemoHUD.generated.h"

UCLASS()
class AShaderPluginDemoHUD : public AHUD
{
	GENERATED_BODY()

public:
	AShaderPluginDemoHUD(const FObjectInitializer& ObjectInitializer);

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

