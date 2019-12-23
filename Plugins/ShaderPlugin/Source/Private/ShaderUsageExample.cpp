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

#include "ShaderUsageExample.h"
#include "ComputeShaderExample.h"
#include "PixelShaderExample.h"

#include "RHI.h"
#include "GlobalShader.h"
#include "RHICommandList.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"

FShaderUsageExample::FShaderUsageExample(bool bRenderEveryFrame, FIntPoint InTextureSize)
	: RenderEveryFrameLock()
	, TextureSize(InTextureSize)
	, bIsShaderExecuting(false)
	, bCachedParametersValid(false)
	, bIsRenderingEveryFrame(bRenderEveryFrame)
{
	if (bIsRenderingEveryFrame)
	{
		const FName RendererModuleName("Renderer");
		IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
		if (RendererModule)
		{
			OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FShaderUsageExample::DrawEveryFrame_RenderThread);
		}
	}
}

FShaderUsageExample::~FShaderUsageExample()
{
	if (bIsRenderingEveryFrame)
	{
		const FName RendererModuleName("Renderer");
		IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
		if (RendererModule)
		{
			RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
		}
	}
}

void FShaderUsageExample::UpdateParameters(FShaderUsageExampleParameters& DrawParameters)
{
	checkf(bIsRenderingEveryFrame, TEXT("Don't call this function if you are not aiming to render every frame! Call Draw() instead."));

	RenderEveryFrameLock.Lock();
	CachedShaderUsageExampleParameters = DrawParameters;
	bCachedParametersValid = true;
	RenderEveryFrameLock.Unlock();
}

void FShaderUsageExample::Draw(FShaderUsageExampleParameters& DrawParameters)
{
	checkf(!bIsRenderingEveryFrame, TEXT("Don't call this function if you are aiming to render every frame! Call UpdateParameters() instead."));
	
	if (bIsShaderExecuting) // Skip this execution round if we are already executing
	{
		return;
	}
	
	bIsShaderExecuting = true;
	
	// This macro sends the function we declare inside to be run on the render thread. 
	// What we do is essentially just send this class and tell the render thread to run the render function as soon as it can.
	// The renderer will check periodically during its execution if there are any queued functions to run.
	ENQUEUE_RENDER_COMMAND(FShaderUsageExampleRunner)(
	[this, DrawParameters](FRHICommandListImmediate& RHICmdList)
	{
		this->Draw_RenderThread(DrawParameters);
	});
}

void FShaderUsageExample::DrawEveryFrame_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	// Just forward to our main draw function
	if (bIsRenderingEveryFrame && bCachedParametersValid)
	{
		// Depending on your data, you might not have to lock here, just added this code to show how you can do it if you have to.
		RenderEveryFrameLock.Lock();
		FShaderUsageExampleParameters Copy = CachedShaderUsageExampleParameters;
		RenderEveryFrameLock.Unlock();

		Draw_RenderThread(Copy);
	}
}

void FShaderUsageExample::Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters)
{
	check(IsInRenderingThread());
	
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	if (!RenderTarget.IsValid())
	{
		FPooledRenderTargetDesc RenderTargetDesc = FPooledRenderTargetDesc::Create2DDesc(TextureSize, 
			PF_R8G8B8A8, FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false);
		
		GRenderTargetPool.FindFreeElement(RHICmdList, RenderTargetDesc, RenderTarget, TEXT("ShaderPlugin_Output"));
	}

	FRDGBuilder GraphBuilder(RHICmdList);

	FShaderUsageExampleResources DrawResources(RenderTarget);
	FComputeShaderExample::AddPass_RenderThread(GraphBuilder, DrawParameters, DrawResources);
	FPixelShaderExample::AddPass_RenderThread(GraphBuilder, DrawParameters, DrawResources);

	GraphBuilder.Execute();
	
	if (DrawParameters.bSaveComputeShaderOutput)
	{
		SaveCSScreenshot_RenderThread(RHICmdList, (FRHITexture2D*)DrawResources.ComputeShaderOutput->GetRHI());
	}

	if (DrawParameters.bSavePixelShaderOutput && RenderTarget.IsValid())
	{
		SavePSScreenShot_RenderThread(RHICmdList, RenderTarget->GetRenderTargetItem().TargetableTexture->GetTexture2D());
	}

	bIsShaderExecuting = false;
}

void FShaderUsageExample::SaveCSScreenshot_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* Texture)
{
	TArray<FColor> Bitmap;

	// To access our resource we do a custom read using lockrect
	uint32 LolStride = 0;
	char* TextureDataPtr = (char*)RHICmdList.LockTexture2D(Texture, 0, EResourceLockMode::RLM_ReadOnly, LolStride, false);

	for (uint32 Row = 0; Row < Texture->GetSizeY(); ++Row)
	{
		uint32* PixelPtr = (uint32*)TextureDataPtr;

		// Since we are using our custom UINT format, we need to unpack it here to access the actual colors
		for (uint32 Col = 0; Col < Texture->GetSizeX(); ++Col)
		{
			uint32 EncodedPixel = *PixelPtr;
			uint8 r = (EncodedPixel & 0x000000FF);
			uint8 g = (EncodedPixel & 0x0000FF00) >> 8;
			uint8 b = (EncodedPixel & 0x00FF0000) >> 16;
			uint8 a = (EncodedPixel & 0xFF000000) >> 24;
			Bitmap.Add(FColor(r, g, b, a));

			PixelPtr++;
		}

		TextureDataPtr += LolStride;
	}

	RHICmdList.UnlockTexture2D(Texture, 0, false);

	if (Bitmap.Num())
	{
		// Create screenshot folder if not already present.
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

		const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));

		uint32 ExtendXWithMSAA = Bitmap.Num() / Texture->GetSizeY();

		// Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
		FFileHelper::CreateBitmap(*ScreenFileName, ExtendXWithMSAA, Texture->GetSizeY(), Bitmap.GetData());

		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
	}
	else
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
	}
}

void FShaderUsageExample::SavePSScreenShot_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef CurrentTexture)
{
	check(IsInRenderingThread());

	TArray<FColor> Bitmap;

	FReadSurfaceDataFlags ReadDataFlags;
	ReadDataFlags.SetLinearToGamma(false);
	ReadDataFlags.SetOutputStencil(false);
	ReadDataFlags.SetMip(0);
	
	// This is pretty straight forward. Since we are using a standard format, we can use this convenience function instead of having to lock rect.
	RHICmdList.ReadSurfaceData(CurrentTexture, FIntRect(0, 0, CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY()), Bitmap, ReadDataFlags);

	// If the format and texture type is supported
	if (Bitmap.Num())
	{
		// Create screenshot folder if not already present.
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

		const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));

		uint32 ExtendXWithMSAA = Bitmap.Num() / CurrentTexture->GetSizeY();

		// Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
		FFileHelper::CreateBitmap(*ScreenFileName, ExtendXWithMSAA, CurrentTexture->GetSizeY(), Bitmap.GetData());

		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
	}
	else
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
	}
}