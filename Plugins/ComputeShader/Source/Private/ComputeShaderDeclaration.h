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

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

//This buffer should contain variables that never, or rarely change
BEGIN_UNIFORM_BUFFER_STRUCT(FComputeShaderConstantParameters, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, SimulationSpeed)
END_UNIFORM_BUFFER_STRUCT(FComputeShaderConstantParameters)

//This buffer is for variables that change very often (each frame for example)
BEGIN_UNIFORM_BUFFER_STRUCT(FComputeShaderVariableParameters, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, TotalTimeElapsedSeconds)
END_UNIFORM_BUFFER_STRUCT(FComputeShaderVariableParameters)

typedef TUniformBufferRef<FComputeShaderConstantParameters> FComputeShaderConstantParametersRef;
typedef TUniformBufferRef<FComputeShaderVariableParameters> FComputeShaderVariableParametersRef;

/***************************************************************************/
/* This class is what encapsulates the shader in the engine.               */
/* It is the main bridge between the HLSL located in the engine directory  */
/* and the engine itself.                                                  */
/***************************************************************************/
class FComputeShaderDeclaration : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FComputeShaderDeclaration, Global);

public:

	FComputeShaderDeclaration() {}

	explicit FComputeShaderDeclaration(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << OutputSurface;

		return bShaderHasOutdatedParams;
	}

	//This function is required to let us bind our runtime surface to the shader using an UAV.
	void SetSurfaces(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef OutputSurfaceUAV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetUniformBuffers(FRHICommandList& RHICmdList, FComputeShaderConstantParameters& ConstantParameters, FComputeShaderVariableParameters& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void UnbindBuffers(FRHICommandList& RHICmdList);

private:
	//This is the actual output resource that we will bind to the compute shader
	FShaderResourceParameter OutputSurface;
};
