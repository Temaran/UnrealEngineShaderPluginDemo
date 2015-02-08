/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2015 Fredrik Lindh
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

#include "Private/PixelShaderDeclaration.h"

/***************************************************************************/
/* This class demonstrates how to use the pixel shader we have declared.   */
/* Most importantly which RHI functions are needed to call and how to get  */
/* some interesting output.                                                */
/***************************************************************************/
class PIXELSHADER_API FPixelShaderUsageExample
{
public:
	FPixelShaderUsageExample(FColor StartColor, ERHIFeatureLevel::Type ShaderFeatureLevel);
	~FPixelShaderUsageExample();

	/********************************************************************************************************/
	/* Let the user change rendertarget during runtime if they want to :D                                   */
	/* @param RenderTarget - This is the output rendertarget!                                               */
	/* @param InputTexture - This is a rendertarget that's used as a texture parameter to the shader :)     */
	/* @param EndColor - This will be set to the dynamic parameter buffer each frame                        */
	/* @param TextureParameterBlendFactor - The scalar weight that decides how much of the texture to blend */
	/********************************************************************************************************/
	void ExecutePixelShader(UTextureRenderTarget2D* RenderTarget, FTexture2DRHIRef InputTexture, FColor EndColor, float TextureParameterBlendFactor);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal();

	/************************************************************************/
	/* Save a screenshot of the target to the project saved folder          */
	/************************************************************************/
	void Save()
	{
		bSave = true;
	}

private:
	bool bIsPixelShaderExecuting;
	bool bMustRegenerateSRV;
	bool bIsUnloading;
	bool bSave;

	FPixelShaderConstantParameters ConstantParameters;
	FPixelShaderVariableParameters VariableParameters;
	ERHIFeatureLevel::Type FeatureLevel;

	/** Main texture */
	FTexture2DRHIRef CurrentTexture;
	FTexture2DRHIRef TextureParameter;
	UTextureRenderTarget2D* CurrentRenderTarget;
	
	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef TextureParameterSRV;

	void SaveScreenshot(FRHICommandListImmediate& RHICmdList);
};
