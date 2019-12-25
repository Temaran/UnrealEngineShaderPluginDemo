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
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

// This struct contains all the data we need to pass from the game thread to draw our effect.
struct FShaderUsageExampleParameters
{
	UTextureRenderTarget2D* RenderTarget;
	FColor StartColor;
	FColor EndColor;
	float SimulationState;
	float ComputeShaderBlend;
	
	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FShaderUsageExampleParameters()	{ }
	FShaderUsageExampleParameters(UTextureRenderTarget2D* InRenderTarget)
		: RenderTarget(InRenderTarget)
		, StartColor(FColor::White)
		, EndColor(FColor::White)
		, SimulationState(1.0f)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
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
 * Passes are very similar to the previous graphics API and are now used when using the rasterizer. 
 * Instead of setting the render target for a rasterization operation, you now set that up when beginning a render pass instead.
 * Operations that don't use the rasterizer (like compute, copy and other operations) simply use the RHICommandList directly like before.
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
	// Call this when you want to hook onto the renderer and start drawing. The shader will be executed once per frame.
	void BeginRendering();

	// When you are done, call this to stop drawing.
	void EndRendering();
	
	// Call this whenever you have new parameters to share. You could set this up to update different sets of properties at
	// different intervals to save on locking and GPU transfer time.
	void UpdateParameters(FShaderUsageExampleParameters& DrawParameters);

private:
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
	FShaderUsageExampleParameters CachedShaderUsageExampleParameters;
	FDelegateHandle OnPostResolvedSceneColorHandle;
	FCriticalSection RenderEveryFrameLock;
	volatile bool bCachedParametersValid;

	void DrawEveryFrame_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
	void Draw_RenderThread(const FShaderUsageExampleParameters& DrawParameters);
};
