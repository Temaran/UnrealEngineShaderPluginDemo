// Copyright 2016-2020 Cadic AB. All Rights Reserved.
// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

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

#define NUM_THREADS_PER_GROUP_DIMENSION 32

/**********************************************************************************************/
/* This class carries our parameter declarations and acts as the bridge between cpp and HLSL. */
/**********************************************************************************************/
class FComputeShaderExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FComputeShaderExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FComputeShaderExampleCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<uint>, OutputTexture)
		SHADER_PARAMETER(FVector2D, TextureSize) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
		SHADER_PARAMETER(float, SimulationState)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                            ShaderPath                     Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FComputeShaderExampleCS, "/TutorialShaders/Private/ComputeShader.usf", "MainComputeShader", SF_Compute);

void FComputeShaderExample::RunComputeShader_RenderThread(FRHICommandListImmediate& RHICmdList, const FShaderUsageExampleParameters& DrawParameters, FUnorderedAccessViewRHIRef ComputeShaderOutputUAV)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ComputeShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, ComputeShaderOutputUAV);
	
	FComputeShaderExampleCS::FParameters PassParameters;
	PassParameters.OutputTexture = ComputeShaderOutputUAV;
	PassParameters.TextureSize = FVector2D(DrawParameters.GetRenderTargetSize().X, DrawParameters.GetRenderTargetSize().Y);
	PassParameters.SimulationState = DrawParameters.SimulationState;

	TShaderMapRef<FComputeShaderExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FIntVector GroupCounts = FIntVector(FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION), FMath::DivideAndRoundUp(DrawParameters.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1);

	FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, PassParameters, GroupCounts);
}
