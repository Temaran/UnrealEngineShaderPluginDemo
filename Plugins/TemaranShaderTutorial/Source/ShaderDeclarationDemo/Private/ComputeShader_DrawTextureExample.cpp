// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#include "ComputeShader_DrawTextureExample.h"
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
class FGalaxySimulatorExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FGalaxySimulatorExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FGalaxySimulatorExampleCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint>, OutputTexture)
		SHADER_PARAMETER(FVector2f, TextureSize) // Metal doesn't support GetDimensions(), so we send in this data via our parameters.
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
//                            ShaderType                                   ShaderPath                            Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FGalaxySimulatorExampleCS, "/TutorialShaders/Private/ComputeShader_DrawTextureExample.usf", "SimulateGalaxy", SF_Compute);

/**********************************************************************************************/
/* These functions schedule our Compute Shader work from the CPU!							  */
/**********************************************************************************************/

void FComputeShader_DrawTextureExample::DispatchGalaxySimulation_RenderThread(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& InputParameters, FRDGTextureUAVRef OutputTextureUAV)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_DispatchGalaxySimulation);
	SCOPED_DRAW_EVENT(RDGBuilder.RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	// This shader shows how you can use a compute shader to write to a texture.
	// Here we send our inputs using parameters as part of the shader parameter struct. This is an efficient way of sending in simple constants for a shader, but does not work well if you need to send larger amounts of data.
	FGalaxySimulatorExampleCS::FParameters* ShaderParams = RDGBuilder.AllocParameters<FGalaxySimulatorExampleCS::FParameters>();
	ShaderParams->OutputTexture = OutputTextureUAV;
	ShaderParams->TextureSize = FVector2f(InputParameters.GetRenderTargetSize().X, InputParameters.GetRenderTargetSize().Y);
	ShaderParams->SimulationState = InputParameters.SimulationState;

	TShaderMapRef<FGalaxySimulatorExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FIntVector GroupCounts = FIntVector(FMath::DivideAndRoundUp(InputParameters.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION), FMath::DivideAndRoundUp(InputParameters.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1);
	FComputeShaderUtils::AddPass(RDGBuilder, RDG_EVENT_NAME("GalaxySimulation"), ERDGPassFlags::Compute | ERDGPassFlags::NeverCull, ComputeShader, ShaderParams, GroupCounts);
}
