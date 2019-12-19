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

#include "PixelShaderDeclaration.h"
#include "PixelShaderPrivatePCH.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "CoreMinimal.h"
#include "Misc/Paths.h"
//#include "ShaderCore.h"


//These are needed to actually implement the constant buffers so they are available inside our shader
//They also need to be unique over the entire solution since they can in fact be accessed from any shader
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FPixelShaderConstantParameters,
                                TEXT("PSConstant"))
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FPixelShaderVariableParameters,
                                TEXT("PSVariable"))

FPixelShaderDeclaration::FPixelShaderDeclaration(const
        ShaderMetaType::CompiledShaderInitializerType& Initializer)
    : FGlobalShader(Initializer) {
    //This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
    TextureParameter.Bind(Initializer.ParameterMap,
                          TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
}

void FPixelShaderDeclaration::SetUniformBuffers(FRHICommandList& RHICmdList,
        FPixelShaderConstantParameters& ConstantParameters,
        FPixelShaderVariableParameters& VariableParameters) {
    FPixelShaderConstantParametersRef ConstantParametersBuffer;
    FPixelShaderVariableParametersRef VariableParametersBuffer;

    ConstantParametersBuffer =
        FPixelShaderConstantParametersRef::CreateUniformBufferImmediate(
            ConstantParameters, UniformBuffer_SingleDraw);
    VariableParametersBuffer =
        FPixelShaderVariableParametersRef::CreateUniformBufferImmediate(
            VariableParameters, UniformBuffer_SingleDraw);

    SetUniformBufferParameter(RHICmdList, GetPixelShader(),
                              GetUniformBufferParameter<FPixelShaderConstantParameters>(),
                              ConstantParametersBuffer);
    SetUniformBufferParameter(RHICmdList, GetPixelShader(),
                              GetUniformBufferParameter<FPixelShaderVariableParameters>(),
                              VariableParametersBuffer);
}

void FPixelShaderDeclaration::SetSurfaces(FRHICommandList& RHICmdList,
        FShaderResourceViewRHIRef TextureParameterSRV) {
    FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

    if (TextureParameter.IsBound()) { //This actually sets the shader resource view to the texture parameter in the shader :)
        RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI,
                TextureParameter.GetBaseIndex(), TextureParameterSRV);
    }
}

void FPixelShaderDeclaration::UnbindBuffers(FRHICommandList& RHICmdList) {
    FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();

    if (TextureParameter.IsBound()) {
        RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI,
                TextureParameter.GetBaseIndex(), FShaderResourceViewRHIParamRef());
    }
}

bool FPixelShaderDeclaration::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return true;
}

//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FVertexShaderExample, TEXT("/Plugin/PixelShader/Private/PixelShaderExample.usf"),
                      TEXT("MainVertexShader"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPixelShaderDeclaration, TEXT("/Plugin/PixelShader/Private/PixelShaderExample.usf"),
                      TEXT("MainPixelShader"), SF_Pixel);

//Needed to make sure the plugin works :)
class FPixelShaderModule : public IPixelShader
{
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FPixelShaderModule, PixelShader)

void FPixelShaderModule::StartupModule()
{
    FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("PixelShader"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/PixelShader"), PluginShaderDir);
}

void FPixelShaderModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}


bool FVertexShaderExample::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return true;
}
