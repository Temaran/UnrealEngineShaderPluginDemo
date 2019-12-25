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

#include "ComputeShaderExample.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32 // This has to be the same as in the compute shader's spec [X, X, 1]

/************************************************************************/
/* Here starts the shader shell code                                    */
/************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************************************/
/* This class carries our parameter declarations and acts as the bridge between cpp and HLSL. */
/**********************************************************************************************/
class FComputeShaderExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FComputeShaderExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FComputeShaderExampleCS, FGlobalShader);

// Here we declare the layout of our parameter struct that we're going to use.
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(float, SimulationSpeed)
		SHADER_PARAMETER(float, TotalTimeElapsedSeconds)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FComputeShaderExampleCS, "/Plugin/ShaderPlugin/Private/ComputeShader.usf", "MainComputeShader", SF_Compute);

void FComputeShaderExample::AddPass_RenderThread(FRDGBuilder& GraphBuilder, const FShaderUsageExampleParameters& DrawParameters, FRDGTextureRef& ComputeShaderOutput)
{
	FIntPoint TextureExtent(DrawParameters.RenderTarget->SizeX, DrawParameters.RenderTarget->SizeY);

	FRDGTextureDesc Desc;
	Desc.Extent = TextureExtent;
	Desc.Format = PF_R32_UINT;
	Desc.NumMips = 1;
	Desc.NumSamples = 1;
	Desc.TargetableFlags = TexCreate_ShaderResource | TexCreate_UAV;
	Desc.DebugName = TEXT("ShaderPlugin_ComputeShaderOutput");
	ComputeShaderOutput = GraphBuilder.CreateTexture(Desc, TEXT("ShaderPlugin_ComputeShaderOutputTexture"), ERDGResourceFlags::None);

	FComputeShaderExampleCS::FParameters* Parameters = GraphBuilder.AllocParameters<FComputeShaderExampleCS::FParameters>();
	Parameters->SimulationSpeed = DrawParameters.SimulationSpeed;
	Parameters->TotalTimeElapsedSeconds = DrawParameters.TotalElapsedTimeSecs;
	Parameters->OutputTexture = GraphBuilder.CreateUAV(ComputeShaderOutput);

	// We can use this util here to make it a bit easier to setup the compute shader pass
	TShaderMapRef<FComputeShaderExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("ShaderPlugin ComputeShaderExample running with sim speed: %f. Saving to disk: %d", DrawParameters.SimulationSpeed, DrawParameters.bSaveComputeShaderOutput),
		*ComputeShader,
		Parameters,
		FIntVector(TextureExtent.X / NUM_THREADS_PER_GROUP_DIMENSION, TextureExtent.Y / NUM_THREADS_PER_GROUP_DIMENSION, 1));
}
