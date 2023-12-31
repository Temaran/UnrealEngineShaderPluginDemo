// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "ShaderDeclarationDemoModule.h"

/**************************************************************************************/
/* This is just an interface we use to keep all the compute shading code in one file. */
/**************************************************************************************/
class FComputeShaderExample
{
public:
	static void RunComputeShader_RenderThread(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& DrawParameters, FRDGTextureUAVRef OutputTextureUAV);
};
