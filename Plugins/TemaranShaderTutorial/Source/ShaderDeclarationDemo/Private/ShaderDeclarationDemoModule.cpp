// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#include "ShaderDeclarationDemoModule.h"

#include "ComputeShaderExample.h"
#include "PixelShaderExample.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

IMPLEMENT_MODULE(FShaderDeclarationDemoModule, ShaderDeclarationDemo)

// Declare some GPU stats so we can track them later
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Render, TEXT("ShaderPlugin: Root Render"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Compute, TEXT("ShaderPlugin: Render Compute Shader"));
DECLARE_GPU_STAT_NAMED(ShaderPlugin_Pixel, TEXT("ShaderPlugin: Render Pixel Shader"));

void FShaderDeclarationDemoModule::StartupModule()
{
	PreRenderHandle.Reset();
	bCachedParametersValid = false;

	// Maps virtual shader source directory to the plugin's actual shaders directory.
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("TemaranShaderTutorial"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/TutorialShaders"), PluginShaderDir);
}

void FShaderDeclarationDemoModule::ShutdownModule()
{
	EndRendering();
}

void FShaderDeclarationDemoModule::BeginRendering()
{
	if (PreRenderHandle.IsValid() || !GEngine)
	{
		return;
	}

	bCachedParametersValid = false;
	PreRenderHandle = GEngine->GetPreRenderDelegateEx().AddRaw(this, &FShaderDeclarationDemoModule::HandlePreRender_RenderThread);
}

void FShaderDeclarationDemoModule::EndRendering()
{
	if (!PreRenderHandle.IsValid() || !GEngine)
	{
		return;
	}

	GEngine->GetPreRenderDelegateEx().Remove(PreRenderHandle);
	PreRenderHandle.Reset();
}

void FShaderDeclarationDemoModule::UpdateParameters(FShaderUsageExampleParameters& DrawParameters)
{
	FScopeLock UpdateLock(&RenderEveryFrameLock);

	CachedShaderUsageExampleParameters = DrawParameters;
	bCachedParametersValid = true;
}

void FShaderDeclarationDemoModule::HandlePreRender_RenderThread(FRDGBuilder& RDGBuilder)
{
	check(IsInRenderingThread());

	if (!bCachedParametersValid)
	{
		return;
	}

	// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
	RenderEveryFrameLock.Lock();
	FShaderUsageExampleParameters DrawParameters = CachedShaderUsageExampleParameters;
	RenderEveryFrameLock.Unlock();

	// Let's draw our effects!
	if (!DrawParameters.RenderTarget)
	{
		return;
	}

	QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_Render); // Used to gather CPU profiling data for Unreal Insights! Insights is a really powerful tool for debugging CPU stats, memory and networking.
	SCOPED_DRAW_EVENT(RDGBuilder.RHICmdList, ShaderPlugin_Render); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	// The graph will help us figure out when the GPU memory is needed, and only have it allocated from then, so this makes memory management a lot easier and nicer!
	FRDGTextureDesc ComputeShaderOutputDesc = FRDGTextureDesc::Create2D(DrawParameters.GetRenderTargetSize(), EPixelFormat::PF_R32_UINT, FClearValueBinding::None, ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::UAV | ETextureCreateFlags::ShaderResource);
	FRDGTextureRef OutputTexture = RDGBuilder.CreateTexture(ComputeShaderOutputDesc, TEXT("ShaderPlugin_ComputeShaderOutput"), ERDGTextureFlags::None);

	FComputeShaderExample::RunComputeShader_RenderThread(RDGBuilder, DrawParameters, RDGBuilder.CreateUAV(OutputTexture));
	FPixelShaderExample::DrawToRenderTarget_RenderThread(RDGBuilder, DrawParameters, RDGBuilder.CreateSRV(OutputTexture));
}
