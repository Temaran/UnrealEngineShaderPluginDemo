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

#include "ComputeShaderPrivatePCH.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32 //This has to be the same as in the compute shader's spec [X, X, 1]

FComputeShaderUsageExample::FComputeShaderUsageExample(float SimulationSpeed, int32 SizeX, int32 SizeY, ERHIFeatureLevel::Type ShaderFeatureLevel)
{
	FeatureLevel = ShaderFeatureLevel;

	ConstantParameters.SimulationSpeed = SimulationSpeed;

	VariableParameters = FComputeShaderVariableParameters();

	bIsComputeShaderExecuting = false;
	bIsUnloading = false;
	bSave = false;

	//There are only a few different texture formats we can use if we want to use the output texture as input in a pixel shader later
	//I would have loved to go with the R8G8B8A8_UNORM approach, but unfortunately, it seems UE4 does not support this in an obvious way, which is why I chose the UINT format using packing instead :)
	//There is some excellent information on this topic in the following links:
    //http://www.gamedev.net/topic/605356-r8g8b8a8-texture-format-in-compute-shader/
	//https://msdn.microsoft.com/en-us/library/ff728749(v=vs.85).aspx
	FRHIResourceCreateInfo CreateInfo;
	Texture = RHICreateTexture2D(SizeX, SizeY, PF_R32_UINT, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	TextureUAV = RHICreateUnorderedAccessView(Texture);
}

FComputeShaderUsageExample::~FComputeShaderUsageExample()
{
	bIsUnloading = true;
}

void FComputeShaderUsageExample::ExecuteComputeShader(float TotalElapsedTimeSeconds)
{
	if (bIsUnloading || bIsComputeShaderExecuting) //Skip this execution round if we are already executing
	{
		return;
	}

	bIsComputeShaderExecuting = true;

	//Now set our runtime parameters!
	VariableParameters.TotalTimeElapsedSeconds = TotalElapsedTimeSeconds;

	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FComputeShaderRunner,
		FComputeShaderUsageExample*, ComputeShader, this,
		{
		ComputeShader->ExecuteComputeShaderInternal();
	}
	);
}

void FComputeShaderUsageExample::ExecuteComputeShaderInternal()
{
	check(IsInRenderingThread());
	
	if (bIsUnloading) //If we are about to unload, so just clean up the UAV :)
	{
		if (NULL != TextureUAV)
		{
			TextureUAV.SafeRelease();
			TextureUAV = NULL;
		}

		return;
	}

	/* Get global RHI command list */
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();

	/** Compute shader calculation */
	TShaderMapRef<FComputeShaderDeclaration> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader->GetComputeShader());

	/* Set inputs/outputs and dispatch compute shader */
	ComputeShader->SetSurfaces(RHICmdList, TextureUAV);
	ComputeShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
	DispatchComputeShader(RHICmdList, *ComputeShader, Texture->GetSizeX() / NUM_THREADS_PER_GROUP_DIMENSION, Texture->GetSizeY() / NUM_THREADS_PER_GROUP_DIMENSION, 1);
	ComputeShader->UnbindBuffers(RHICmdList);

	if (bSave) //Save to disk if we have a save request!
	{
		bSave = false;

		SaveScreenshot(RHICmdList);
	}

	bIsComputeShaderExecuting = false;
}

void FComputeShaderUsageExample::SaveScreenshot(FRHICommandListImmediate& RHICmdList)
{
	TArray<FColor> Bitmap;

	//To access our resource we do a custom read using lockrect
	uint32 LolStride = 0;
	char* TextureDataPtr = (char*)RHICmdList.LockTexture2D(Texture, 0, EResourceLockMode::RLM_ReadOnly, LolStride, false);

	for (uint32 Row = 0; Row < Texture->GetSizeY(); ++Row)
	{
		uint32* PixelPtr = (uint32*)TextureDataPtr;
		
		//Since we are using our custom UINT format, we need to unpack it here to access the actual colors
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

		// move to next row:
		TextureDataPtr += LolStride;
	}

	RHICmdList.UnlockTexture2D(Texture, 0, false);

	// if the format and texture type is supported
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
