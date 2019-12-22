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

#include "PixelShaderExample.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "Runtime/RenderCore/Public/PixelShaderUtils.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

/************************************************************************/
/* This is the type we use as vertices for our fullscreen quad.         */
/************************************************************************/
struct FTextureVertex
{
	FVector4 Position;
	FVector2D UV;
};

/************************************************************************/
/* We define our vertex declaration to let us get our UV coords into    */
/* the shader                                                           */
/************************************************************************/
class FTextureVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
}; 

// It seems to be the convention to expose all vertex declarations as globals, and then reference them as externs in the headers where they are needed.
// It kind of makes sense since they do not contain any parameters that change and are purely used as their names suggest, as declarations :)
TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;

/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FSimplePassThroughVS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FSimplePassThroughVS, Global);
	SHADER_USE_PARAMETER_STRUCT(FSimplePassThroughVS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

/**********************************************************************************************/
/* This class carries our parameter declarations and acts as the bridge between cpp and HLSL. */
/**********************************************************************************************/
class FPixelShaderExamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FPixelShaderExamplePS);
	SHADER_USE_PARAMETER_STRUCT(FPixelShaderExamplePS, FGlobalShader);

	// Here we declare the layout of our parameter struct that we're going to use.
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SRV(Texture2D<uint>, ComputeShaderOutput)
		SHADER_PARAMETER(FVector4, StartColor)
		SHADER_PARAMETER(FVector4, EndColor)
		SHADER_PARAMETER(float, BlendFactor)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

// Vertex shaders generally still use this macro
//                           ShaderType                            ShaderPath                      Shader function name    Type
IMPLEMENT_SHADER_TYPE(, FSimplePassThroughVS, TEXT("/Plugin/ShaderPlugin/Private/PixelShader.usf"), TEXT("MainVertexShader"), SF_Vertex);

// For the pixel shader we will use the global macro.
//                           ShaderType                            ShaderPath                      Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FPixelShaderExamplePS, "/Plugin/ShaderPlugin/Private/PixelShader.usf", "MainPixelShader", SF_Pixel);

void FPixelShaderExample::AddPass_RenderThread(FRDGBuilder& GraphBuilder, const FShaderUsageExampleParameters& DrawParameters, FShaderUsageExampleResources& DrawResources)
{
	if (!DrawParameters.RenderTarget)
	{
		return;
	}
	   
	FPixelShaderExamplePS::FParameters* Parameters = GraphBuilder.AllocParameters<FPixelShaderExamplePS::FParameters>();
	Parameters->ComputeShaderOutput = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(DrawResources.ComputeShaderOutput))->GetRHI();
	Parameters->StartColor = FVector4(DrawParameters.StartColor.R, DrawParameters.StartColor.G, DrawParameters.StartColor.B, DrawParameters.StartColor.A);
	Parameters->EndColor = FVector4(DrawParameters.EndColor.R, DrawParameters.EndColor.G, DrawParameters.EndColor.B, DrawParameters.EndColor.A);
	Parameters->BlendFactor = DrawParameters.ComputeShaderBlend;

	GraphBuilder.AddPass(RDG_EVENT_NAME("ShaderPlugin PixelShaderExample"),	Parameters,	ERDGPassFlags::Raster, 
	[Parameters, DrawParameters](FRHICommandList& RHICmdList)
	{
		// This is where the magic happens
		FTexture2DRHIRef CurrentTexture = DrawParameters.RenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
		FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::Load_Store);

		RHICmdList.BeginRenderPass(RPInfo, TEXT("ShaderPlugin: Draw compute shader output to render target"));

		TShaderMapRef<FSimplePassThroughVS> VertexShader(GetGlobalShaderMap(DrawParameters.ShaderFeatureLevel));
		TShaderMapRef<FPixelShaderExamplePS> PixelShader(GetGlobalShaderMap(DrawParameters.ShaderFeatureLevel));

		SetShaderParameters(RHICmdList, *PixelShader, PixelShader->GetPixelShader(), *Parameters);

		// Set the graphic pipeline state.
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Never>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		FPixelShaderUtils::DrawFullscreenQuad(RHICmdList, 1);

		RHICmdList.EndRenderPass();
	});
}
