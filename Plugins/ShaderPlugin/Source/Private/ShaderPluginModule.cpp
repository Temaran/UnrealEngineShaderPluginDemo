/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2015-2020 Fredrik Lindh
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

#include "ShaderPluginModule.h"

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

IMPLEMENT_MODULE(FShaderPluginModule, ShaderPlugin)

void FShaderPluginModule::StartupModule()
{
	OnPostResolvedSceneColorHandle.Reset();
	bCachedParametersValid = false;

	// Maps virtual shader source directory to the plugin's actual shaders directory.
	FString PluginShaderDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("ShaderPlugin/Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ShaderPlugin"), PluginShaderDir);
}

void FShaderPluginModule::ShutdownModule()
{
	EndRendering();
}

void FShaderPluginModule::BeginRendering()
{
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	bCachedParametersValid = false;

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FShaderPluginModule::DrawEveryFrame_RenderThread);
	}
}

void FShaderPluginModule::EndRendering()
{
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FShaderPluginModule::UpdateParameters(FShaderUsageExampleParameters& DrawParameters)
{
	RenderEveryFrameLock.Lock();
	CachedShaderUsageExampleParameters = DrawParameters;
	bCachedParametersValid = true;
	RenderEveryFrameLock.Unlock();
}

void FShaderPluginModule::DrawEveryFrame_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	if (!bCachedParametersValid)
	{
		return;
	}

	// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
	RenderEveryFrameLock.Lock();
	FShaderUsageExampleParameters Copy = CachedShaderUsageExampleParameters;
	RenderEveryFrameLock.Unlock();

	Draw_RenderThread(Copy);
}

void FShaderPluginModule::Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters)
{
	check(IsInRenderingThread());

	if (!DrawParameters.RenderTarget)
	{
		return;
	}

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	if (!ComputeShaderOutput.IsValid())
	{
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(DrawParameters.GetRenderTargetSize(), PF_R32_UINT, FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		ComputeShaderOutputDesc.DebugName = TEXT("ShaderPlugin_ComputeShaderOutput");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("ShaderPlugin_ComputeShaderOutput"));
	}

	FComputeShaderExample::RunComputeShader_RenderThread(RHICmdList, DrawParameters, ComputeShaderOutput->GetRenderTargetItem().UAV);
	FPixelShaderExample::DrawToRenderTarget_RenderThread(RHICmdList, DrawParameters, ComputeShaderOutput->GetRenderTargetItem().TargetableTexture);
}
