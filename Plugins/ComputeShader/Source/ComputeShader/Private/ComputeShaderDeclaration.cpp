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

#include "ComputeShaderDeclaration.h"
#include "ComputeShaderPrivatePCH.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

//These are needed to actually implement the constant buffers so they are available inside our shader
//They also need to be unique over the entire solution since they can in fact be accessed from any shader
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FComputeShaderConstantParameters, TEXT("CSConstants"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FComputeShaderVariableParameters, TEXT("CSVariables"))

FComputeShaderDeclaration::FComputeShaderDeclaration(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
: FGlobalShader(Initializer)
{
	//This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
	OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
}

void FComputeShaderDeclaration::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
}

void FComputeShaderDeclaration::SetSurfaces(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef OutputSurfaceUAV)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (OutputSurface.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputSurface.GetBaseIndex(), OutputSurfaceUAV);
}

void FComputeShaderDeclaration::SetUniformBuffers(FRHICommandList& RHICmdList, FComputeShaderConstantParameters& ConstantParameters, FComputeShaderVariableParameters& VariableParameters)
{
	FComputeShaderConstantParametersRef ConstantParametersBuffer;
	FComputeShaderVariableParametersRef VariableParametersBuffer;

	ConstantParametersBuffer = FComputeShaderConstantParametersRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FComputeShaderVariableParametersRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FComputeShaderConstantParameters>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetComputeShader(), GetUniformBufferParameter<FComputeShaderVariableParameters>(), VariableParametersBuffer);
}

/* Unbinds buffers that will be used elsewhere */
void FComputeShaderDeclaration::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FComputeShaderRHIParamRef ComputeShaderRHI = GetComputeShader();

	if (OutputSurface.IsBound())
		RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputSurface.GetBaseIndex(), FUnorderedAccessViewRHIRef());
}

bool FComputeShaderDeclaration::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return true;
}

//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType                    ShaderFileName                Shader function name       Type
IMPLEMENT_SHADER_TYPE(, FComputeShaderDeclaration, TEXT("/Plugin/ComputeShader/Private/ComputeShaderExample.usf"), TEXT("MainComputeShader"), SF_Compute);

//Needed to make sure the plugin works :)
class FComputeShaderModule : public IModuleInterface
{
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FComputeShaderModule, PixelShader)

void FComputeShaderModule::StartupModule()
{
#if 1
    FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ComputeShader"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/ComputeShader"), PluginShaderDir);
#endif
}

void FComputeShaderModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}
