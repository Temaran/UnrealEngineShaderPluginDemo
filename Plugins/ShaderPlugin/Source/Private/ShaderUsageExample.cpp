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
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

FShaderUsageExample::FShaderUsageExample()
	: bIsShaderExecuting(false)
{
}

FShaderUsageExample::~FShaderUsageExample()
{
}

void FShaderUsageExample::Draw(FShaderUsageExampleParameters& DrawParameters)
{
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

void FShaderUsageExample::Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters)
{
	check(IsInRenderingThread());
	
	FShaderUsageExampleResources Resources;

	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	FRDGBuilder GraphBuilder(RHICmdList);
		
	FComputeShaderExample::AddPass_RenderThread(GraphBuilder, DrawParameters, Resources);

	GraphBuilder.Execute();
	
	if (DrawParameters.bSaveComputeShaderOutput)
	{
		SaveCSScreenshot_RenderThread(RHICmdList, (FRHITexture2D*)Resources.ComputeShaderOutput->GetRHI());
	}

	if (DrawParameters.bSavePixelShaderOutput && DrawParameters.RenderTarget)
	{
		SavePSScreenShot_RenderThread(RHICmdList, DrawParameters.RenderTarget->GetRenderTargetResource()->GetRenderTargetTexture());
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
	ReadDataFlags.SetMip(0); //No mip supported ofc!
	
	//This is pretty straight forward. Since we are using a standard format, we can use this convenience function instead of having to lock rect.
	RHICmdList.ReadSurfaceData(CurrentTexture, FIntRect(0, 0, CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY()), Bitmap, ReadDataFlags);

	// if the format and texture type is supported
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