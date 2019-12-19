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

/***************************************************************************/
/* This class demonstrates how to use the compute shader we have declared. */
/* Most importantly which RHI functions are needed to call and how to get  */
/* some interesting output.                                                */
/***************************************************************************/
class SHADERPLUGIN_API FComputeShaderExample
{
public:
	FComputeShaderExample(float InSimulationSpeed, int32 InTextureWidth, int32 InTextureHeight, ERHIFeatureLevel::Type ShaderFeatureLevel);
	~FComputeShaderExample();

	/************************************************************************/
	/* Run this to execute the compute shader once!                         */
	/* @param TotalElapsedTimeSeconds - We use this for simulation state    */
	/************************************************************************/
	void ExecuteComputeShader(float TotalElapsedTimeSeconds);

	/************************************************************************/
	/* Save a screenshot of the target to the project saved folder          */
	/************************************************************************/
	void Save()
	{
		bSave = true;
	}

	FTexture2DRHIRef GetTexture() { return Texture; }

private:
	int32 TextureWidth;
	int32 TextureHeight;
	float SimulationSpeed;

	volatile bool bIsComputeShaderExecuting;
	volatile bool bIsUnloading;
	volatile bool bSave;

	ERHIFeatureLevel::Type FeatureLevel;

	/** Main texture */
	FTexture2DRHIRef Texture;

	/** We need a UAV if we want to be able to write to the resource*/
	FUnorderedAccessViewRHIRef TextureUAV;

	void ExecuteComputeShader_RenderThread(float TotalElapsedTimeSeconds);
	void SaveScreenshot(FRHICommandListImmediate& RHICmdList);
};
