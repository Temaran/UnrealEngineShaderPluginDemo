// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#include "VertexAndPixelShader_FullscreenTexturedQuadExample.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Runtime/RenderCore/Public/PixelShaderUtils.h"

/************************************************************************/
/* Simple static vertex buffer.                                         */
/************************************************************************/
class FSimpleScreenVertexBuffer : public FVertexBuffer
{
public:
	/** Initialize the RHI for this rendering resource */
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		TResourceArray<FFilterVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(6);

		Vertices[0].Position = FVector4f(-1, 1, 0, 1);
		Vertices[0].UV = FVector2f(0, 0);

		Vertices[1].Position = FVector4f(1, 1, 0, 1);
		Vertices[1].UV = FVector2f(1, 0);

		Vertices[2].Position = FVector4f(-1, -1, 0, 1);
		Vertices[2].UV = FVector2f(0, 1);

		Vertices[3].Position = FVector4f(1, -1, 0, 1);
		Vertices[3].UV = FVector2f(1, 1);

		// Create vertex buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(TEXT("ShaderDemoSquare"), &Vertices);
		VertexBufferRHI = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};
TGlobalResource<FSimpleScreenVertexBuffer> GSimpleScreenVertexBuffer;

/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FSimplePassThroughVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSimplePassThroughVS);
	SHADER_USE_PARAMETER_STRUCT(FSimplePassThroughVS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	// Add your own VS params here!
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

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<uint>, ComputeShaderOutput)
		SHADER_PARAMETER(FVector4f, StartColor)
		SHADER_PARAMETER(FVector4f, EndColor)
		SHADER_PARAMETER(FVector2f, TextureSize) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
		SHADER_PARAMETER(float, BlendFactor)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

// Since we have to allocate the params via the RenderGraph, it's practical to use a wrapper struct like this for both VS and PS inputs.
// We also have to include binding slots in our parameter struct, as this is what we will be rendering to, and this wrapper struct is perfect for that!
BEGIN_SHADER_PARAMETER_STRUCT(FDrawExampleParameters, )
SHADER_PARAMETER_STRUCT_INCLUDE(FSimplePassThroughVS::FParameters, VS)
SHADER_PARAMETER_STRUCT_INCLUDE(FPixelShaderExamplePS::FParameters, PS)
RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

// This will tell the engine to create the shader and where the shader entry point is.
//                           ShaderType                            ShaderPath                                           Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FSimplePassThroughVS, "/TutorialShaders/Private/VertexAndPixelShader_FullscreenTexturedQuad.usf", "MainVertexShader", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FPixelShaderExamplePS, "/TutorialShaders/Private/VertexAndPixelShader_FullscreenTexturedQuad.usf", "MainPixelShader", SF_Pixel);

void FVertexAndPixelShader_FullscreenTexturedQuadExample::DrawToRenderTarget_RenderThread(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& DrawParameters, FRDGTextureSRVRef OutputTexture)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_PixelShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RDGBuilder.RHICmdList, ShaderPlugin_Pixel); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	// First, we must register our output render target resource with the graph.
	TRefCountPtr<IPooledRenderTarget> RenderTarget = CreateRenderTarget(DrawParameters.RenderTarget->GetResource()->GetTexture2DRHI(), TEXT("Draw Example RenderTarget"));
	FRDGTextureRef RenderTargetTexture = RDGBuilder.RegisterExternalTexture(RenderTarget);

	FDrawExampleParameters* ShaderParams = RDGBuilder.AllocParameters<FDrawExampleParameters>();
	ShaderParams->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
	ShaderParams->PS.ComputeShaderOutput = OutputTexture;
	ShaderParams->PS.StartColor = FVector4f(DrawParameters.StartColor.R, DrawParameters.StartColor.G, DrawParameters.StartColor.B, DrawParameters.StartColor.A) / 255.0f;
	ShaderParams->PS.EndColor = FVector4f(DrawParameters.EndColor.R, DrawParameters.EndColor.G, DrawParameters.EndColor.B, DrawParameters.EndColor.A) / 255.0f;
	ShaderParams->PS.TextureSize = FVector2f(DrawParameters.GetRenderTargetSize().X, DrawParameters.GetRenderTargetSize().Y);
	ShaderParams->PS.BlendFactor = DrawParameters.ComputeShaderBlend;

	auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FSimplePassThroughVS> VertexShader(ShaderMap);
	TShaderMapRef<FPixelShaderExamplePS> PixelShader(ShaderMap);

	// If we want to use a vertex shader like this, we have to schedule a custom pass!
	RDGBuilder.AddPass(
		RDG_EVENT_NAME("FPixelShaderExample"),
		ShaderParams,
		ERDGPassFlags::Raster,
		[ShaderParams, VertexShader, PixelShader](FRHICommandList& RHICmdList)
		{
			// Set the graphic pipeline state.
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			// Set parameters
			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), ShaderParams->PS);
			SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), ShaderParams->VS);

			// Draw
			RHICmdList.SetStreamSource(0, GSimpleScreenVertexBuffer.VertexBufferRHI, 0);
			RHICmdList.DrawPrimitive(0, 2, 1);
		});
}
