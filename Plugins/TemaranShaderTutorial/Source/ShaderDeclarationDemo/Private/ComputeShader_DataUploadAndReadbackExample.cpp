// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#include "ComputeShader_DataUploadAndReadbackExample.h"
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

// Here is another, smaller compute shader that illustrates how to push data to the GPU, and how to read it back!
class FReduceSumExampleCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FReduceSumExampleCS);
	SHADER_USE_PARAMETER_STRUCT(FReduceSumExampleCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<int32>, InputBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<int32>, OutputBuffer)
		SHADER_PARAMETER(uint32, InputBufferLength)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// This example shader uses wave operations, so it requires SM6.
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), 1);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};

// This will tell the engine to create the shader and where the shader entry point is.
//                            ShaderType                                 ShaderPath                                Shader function name  Type
IMPLEMENT_GLOBAL_SHADER(FReduceSumExampleCS, "/TutorialShaders/Private/ComputeShader_DataUploadAndReadbackExample.usf", "ReduceSum", SF_Compute);


/**********************************************************************************************/
/* These functions schedule our Compute Shader work from the CPU!							  */
/**********************************************************************************************/

void FComputeShader_DataUploadAndReadbackExample::ReduceSum(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& InputParameters, FIntegerSummationWorkSet& ReduceSummationWorkSet)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_RunComputeShaders); // Used to gather CPU profiling data for Unreal Insights.
	SCOPED_DRAW_EVENT(RDGBuilder.RHICmdList, ShaderPlugin_Compute); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	FScopeLock WriteOutputLock(&ReduceSummationWorkSet.WorkSetLock);

	// Before we send off our new jobs, let's first pick up the output from any previous jobs!
	ReadbackReduceSum_RenderThread(RDGBuilder, ReduceSummationWorkSet);

	// Now we can send off any new requests.
	for (auto& [RequestId, ArrayToSum] : InputParameters.IntegerSummationRequests)
	{
		DispatchReduceSum_RenderThread(RDGBuilder, ArrayToSum, RequestId, ReduceSummationWorkSet);
	}
}

void FComputeShader_DataUploadAndReadbackExample::ReadbackReduceSum_RenderThread(FRDGBuilder& RDGBuilder, FIntegerSummationWorkSet& ReduceSummationWorkSet)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_ReadbackReduceSum);

	// Here we look through our previous summation requests and finish any that might be ready.
	for (auto& [RequestId, WorkData] : ReduceSummationWorkSet.ReduceSummationWork)
	{
		if (!WorkData.bReadbackComplete && WorkData.Readback.IsReady())
		{
			int32* Buffer = (int32*)WorkData.Readback.Lock(WorkData.NrWorkGroups * sizeof(int32));
			if (!Buffer)
			{
				continue;
			}

			WorkData.Result = 0;
			for (int32 GroupDataIdx = 0; GroupDataIdx < WorkData.NrWorkGroups; GroupDataIdx++)
			{
				WorkData.Result += Buffer[GroupDataIdx];
			}

			WorkData.Readback.Unlock();
			WorkData.bReadbackComplete = true;
		}
	}
}

void FComputeShader_DataUploadAndReadbackExample::DispatchReduceSum_RenderThread(FRDGBuilder& RDGBuilder, const TArray<int32>& ArrayToSum, const int32 RequestId, FIntegerSummationWorkSet& ReduceSummationWorkSet)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_DispatchReduceSum);

	FIntegerSummationResult& WorkData = ReduceSummationWorkSet.ReduceSummationWork.Add(RequestId);
	WorkData.NrWorkGroups = FMath::DivideAndRoundUp(ArrayToSum.Num(), NUM_THREADS_PER_GROUP_DIMENSION);

	// In this example shader, we send a larger data set from a CPU side array, run a Compute Shader on it, and then perform a read-back to grab the result.
	// First we create an RDG buffer with the appropriate size, and then instruct the graph to upload our CPU data to it.
	FRDGBufferRef IntegerBuffer = RDGBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(ArrayToSum.GetTypeSize(), ArrayToSum.Num()), TEXT("IntegerBuffer"));
	RDGBuilder.QueueBufferUpload(IntegerBuffer, ArrayToSum.GetData(), ArrayToSum.GetTypeSize() * ArrayToSum.Num(), ERDGInitialDataFlags::None);

	FRDGBufferRef OutputBuffer = RDGBuilder.CreateBuffer(FRDGBufferDesc::CreateStructuredDesc(sizeof(int32), WorkData.NrWorkGroups), TEXT("ReductionOutput"));

	FReduceSumExampleCS::FParameters* ShaderParams = RDGBuilder.AllocParameters<FReduceSumExampleCS::FParameters>();
	ShaderParams->InputBuffer = RDGBuilder.CreateSRV(IntegerBuffer);
	ShaderParams->OutputBuffer = RDGBuilder.CreateUAV(OutputBuffer);
	ShaderParams->InputBufferLength = ArrayToSum.Num();

	// Clear the output buffer
	AddClearUAVPass(RDGBuilder, ShaderParams->OutputBuffer, 0);

	TShaderMapRef<FReduceSumExampleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FIntVector GroupCounts = FIntVector(WorkData.NrWorkGroups, 1, 1);
	FComputeShaderUtils::AddPass(RDGBuilder, RDG_EVENT_NAME("DispatchReduceSum_RenderThread"), ERDGPassFlags::Compute | ERDGPassFlags::NeverCull, ComputeShader, ShaderParams, GroupCounts);

	// Schedule a pass that will perform the read-back! We need to read one integer for each work group as we will have to sum them to get the final result.
	FRHIGPUBufferReadback* Readback = &WorkData.Readback;
	AddReadbackBufferPass(RDGBuilder, RDG_EVENT_NAME("ReadbackReduceSum_RenderThread"), OutputBuffer,
		[Readback, OutputBuffer](FRHICommandList& RHICmdList)
		{
			Readback->EnqueueCopy(RHICmdList, OutputBuffer->GetRHI());
		});
}
