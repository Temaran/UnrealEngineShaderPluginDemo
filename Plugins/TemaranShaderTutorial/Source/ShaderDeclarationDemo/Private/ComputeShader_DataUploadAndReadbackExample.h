// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "ShaderDeclarationDemoModule.h"

/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FComputeShader_DataUploadAndReadbackExample
{
public:
	static void ReduceSum(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& InputParameters, FIntegerSummationWorkSet& ReduceSummationWorkSet);

	// The Array Sum demo code shows how you can send larger sets of data into Compute Shaders and then read back the results in a safe manner.
	static void ReadbackReduceSum_RenderThread(FRDGBuilder& RDGBuilder, FIntegerSummationWorkSet& ReduceSummationWorkSet);
	static void DispatchReduceSum_RenderThread(FRDGBuilder& RDGBuilder, const TArray<int32>& ArrayToSum, const int32 RequestId, FIntegerSummationWorkSet& ReduceSummationWorkSet);
};
