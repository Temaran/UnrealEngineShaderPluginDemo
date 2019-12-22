/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2015-2020 Fredrik Lindh
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "RenderGraphResources.h"

struct FShaderUsageExampleParameters
{
	class UTextureRenderTarget2D* RenderTarget;
	ERHIFeatureLevel::Type ShaderFeatureLevel;
	FColor StartColor;
	FColor EndColor;
	float SimulationSpeed;
	float ComputeShaderBlend;
	float DeltaTimeSecs;
	float TotalElapsedTimeSecs;
	bool bSaveComputeShaderOutput;
	bool bSavePixelShaderOutput;

	FShaderUsageExampleParameters(ERHIFeatureLevel::Type InShaderFeatureLevel, UTextureRenderTarget2D& InRenderTarget, float InDeltaTimeSeconds, float InTotalTimeSeconds)
		: RenderTarget(&InRenderTarget)
		, ShaderFeatureLevel(InShaderFeatureLevel)
		, StartColor(FColor::White)
		, EndColor(FColor::White)
		, SimulationSpeed(1.0f)
		, DeltaTimeSecs(InDeltaTimeSeconds)
		, TotalElapsedTimeSecs(InTotalTimeSeconds)
		, bSaveComputeShaderOutput(false)
		, bSavePixelShaderOutput(false)
	{
	}
};

struct FShaderUsageExampleResources
{
	FRDGTextureRef ComputeShaderOutput;
	FRDGTextureUAVRef ComputeShaderOutputUAV;

	FShaderUsageExampleResources()
		: ComputeShaderOutput(nullptr)
		, ComputeShaderOutputUAV(nullptr)
	{
	}
};

/************************************************************************************************************/
/* This is the class we use to show how to best set up shaders in UE4.                                      */
/* You can still use shaders with direct calls to the device if you want. That way might also               */
/* be easier to understand. To see examples of that, see the 4.21 branch in this repo.                      */
/*                                                                                                          */
/* Using task graphs like in this example though is easier in the way that it allows usage                  */
/* of GPU resources to be scheduled better, it's practical for setting up textures and parameters.          */
/* As the engine almost exclusively uses task graphs now for rendering tasks as well, learning by example   */
/* is also easier right now if you elect to use them.                                                       */
/************************************************************************************************************/
class SHADERPLUGIN_API FShaderUsageExample
{
public:

public:
	FShaderUsageExample();
	~FShaderUsageExample();

	/********************************************************************************************************/
	/* Let the user change rendertarget during runtime if they want to :D                                   */
	/* @param RenderTarget - This is the output rendertarget!                                               */
	/* @param InputTexture - This is a rendertarget that's used as a texture parameter to the shader :)     */
	/* @param EndColor - This will be set to the dynamic parameter buffer each frame                        */
	/* @param TextureParameterBlendFactor - The scalar weight that decides how much of the texture to blend */
	/********************************************************************************************************/
	void Draw(FShaderUsageExampleParameters& DrawParameters);

private:
	volatile bool bIsShaderExecuting;
	
	void Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters);
	void SaveCSScreenshot_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* Texture);
	void SavePSScreenShot_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef CurrentTexture);
};
