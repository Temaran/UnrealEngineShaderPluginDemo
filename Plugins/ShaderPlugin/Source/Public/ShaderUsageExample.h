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
	UTextureRenderTarget2D* RenderTarget;
	ERHIFeatureLevel::Type ShaderFeatureLevel;
	FColor StartColor;
	FColor EndColor;
	float SimulationSpeed;
	float ComputeShaderBlend;
	float DeltaTimeSecs;
	float TotalElapsedTimeSecs;
	bool bSaveComputeShaderOutput;
	bool bSavePixelShaderOutput;

	FShaderUsageExampleParameters()	{ }
	FShaderUsageExampleParameters(ERHIFeatureLevel::Type InShaderFeatureLevel, UTextureRenderTarget2D* InRenderTarget, float InDeltaTimeSeconds, float InTotalTimeSeconds)
		: RenderTarget(InRenderTarget)
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
	TRefCountPtr<IPooledRenderTarget> RenderTarget;

	FShaderUsageExampleResources(TRefCountPtr<IPooledRenderTarget>& InRenderTarget)
		: ComputeShaderOutput(nullptr)
		, RenderTarget(InRenderTarget)
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
	/********************************************************************************************************/
	/* We can use two different strategies when we want to render something.                                */
	/* Active: We can either set it up in an "active" fashion, where we need to call a function to execute  */
	/* rendering code. This is useful when we only need to render something now and then.                   */
	/* When using this class with the "active" strategy, we need to call Draw() every frame we want to draw.*/
	/*                                                                                                      */
	/* Passive: We can also simply register to render every frame. This is obviously useful when you need   */
	/* to render something that changes every frame.                                                        */
	/* To use this class with the "passive" strategy, we need to call SetParameters() every time we have    */
	/* updated data.                                                                                        */
	/********************************************************************************************************/
	FShaderUsageExample(bool bRenderEveryFrame, FIntPoint InTextureSize);
	~FShaderUsageExample();

	/********************************************************************************************************/
	/* You need to call this function every time you want to draw something if you are using the "active"   */
	/* strategy.                                                                                            */
	/********************************************************************************************************/
	void Draw(FShaderUsageExampleParameters& DrawParameters);

	/********************************************************************************************************/
	/* You need to call this function every time you want to update your parameters if you are using the    */
	/* "passive" strategy.                                                                                  */
	/********************************************************************************************************/
	void UpdateParameters(FShaderUsageExampleParameters& DrawParameters);

private:
	TRefCountPtr<IPooledRenderTarget> RenderTarget;
	FShaderUsageExampleParameters CachedShaderUsageExampleParameters;
	FDelegateHandle OnPostResolvedSceneColorHandle; 
	FCriticalSection RenderEveryFrameLock;
	FIntPoint TextureSize;
	volatile bool bIsShaderExecuting;
	volatile bool bCachedParametersValid;
	bool bIsRenderingEveryFrame;

	void DrawEveryFrame_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
	void Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters);
	void SaveCSScreenshot_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* Texture);
	void SavePSScreenShot_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef CurrentTexture);
};
