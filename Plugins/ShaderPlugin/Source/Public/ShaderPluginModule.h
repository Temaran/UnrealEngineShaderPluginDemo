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

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "RenderGraphResources.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderUsageExampleParameters
{
	UTextureRenderTarget2D* RenderTarget;
	FColor StartColor;
	FColor EndColor;
	float SimulationSpeed;
	float ComputeShaderBlend;
	float DeltaTimeSecs;
	float TotalElapsedTimeSecs;
	bool bSaveComputeShaderOutput;
	bool bSavePixelShaderOutput;

	FShaderUsageExampleParameters() { }
	FShaderUsageExampleParameters(UTextureRenderTarget2D* InRenderTarget, float InDeltaTimeSeconds, float InTotalTimeSeconds)
		: RenderTarget(InRenderTarget)
		, StartColor(FColor::White)
		, EndColor(FColor::White)
		, SimulationSpeed(1.0f)
		, DeltaTimeSecs(InDeltaTimeSeconds)
		, TotalElapsedTimeSecs(InTotalTimeSeconds)
		, bSaveComputeShaderOutput(false)
		, bSavePixelShaderOutput(false)
	{
	}
};

/*
 * Since we already have a module interface due to us being in a plugin, it's pretty handy to just use it
 * to interact with the renderer. It gives us the added advantage of being able to decouple any render
 * hooks from game modules as well, allowing for safer and easier cleanup.
 *
 * With the advent of UE4.22 the renderer got an overhaul to more modern usage patterns present in API's
 * like DX12. The biggest differences for us are that we must now wrap all rendering code either in either
 * a "Render Graph" or a "Render Pass". They're good for different things of course.
 *
 * Render graphs:
 * In UE4, you generally want to work with graphs if you are working with larger rendering jobs and working with engine
 * pooled render targets. As the engine almost exclusively uses task graphs now for rendering tasks as well, learning
 * by example is also easier right now if you elect to use them. They are quite hard (if not impossible?) to use when
 * interfacing with UObject render resources like UTextures however, so if you want to do that, you need to use passes instead.
 * Graphs generally perform better if you use them for larger jobs. For smaller work, you yet again want to look at passes instead.
 *
 * Render passes:
 * Passes are very similar to the previous graphics API. Instead of using the RHI command list to for example set render
 * targets though, you wrap this information into a pass instead.
 *
 * There are of course many resources you can leverage to get a better understanding of the new API. A good initial overview
 * can be found in this presentation:
 * See: https://developer.nvidia.com/sites/default/files/akamai/gameworks/blog/GDC16/GDC16_gthomas_adunn_Practical_DX12.pdf
 */
class SHADERPLUGIN_API FShaderPluginModule : public IModuleInterface
{
public:
	static inline FShaderPluginModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FShaderPluginModule>("ShaderPlugin");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ShaderPlugin");
	}

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	void BeginRendering();
	void EndRendering();
	void UpdateParameters(FShaderUsageExampleParameters& DrawParameters);

private:
	FShaderUsageExampleParameters CachedShaderUsageExampleParameters;
	FDelegateHandle OnPostResolvedSceneColorHandle;
	FCriticalSection RenderEveryFrameLock;
	volatile bool bCachedParametersValid;

	void DrawEveryFrame_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
	void Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters);
	void SaveCSScreenshot_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* Texture);
	void SavePSScreenShot_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef CurrentTexture);
};
