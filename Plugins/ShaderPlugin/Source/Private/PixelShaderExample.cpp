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

// #include "PixelShaderPrivatePCH.h"
// #include "RHIStaticStates.h"
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// //This buffer should contain variables that never, or rarely change
// BEGIN_UNIFORM_BUFFER_STRUCT(FPixelShaderConstantParameters, )
// DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FVector4, StartColor)
// END_UNIFORM_BUFFER_STRUCT(FPixelShaderConstantParameters)
// 
// //This buffer is for variables that change very often (each frame for example)
// BEGIN_UNIFORM_BUFFER_STRUCT(FPixelShaderVariableParameters, )
// DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, TextureParameterBlendFactor)
// DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FVector4, EndColor)
// END_UNIFORM_BUFFER_STRUCT(FPixelShaderVariableParameters)
// 
// typedef TUniformBufferRef<FPixelShaderConstantParameters> FPixelShaderConstantParametersRef;
// typedef TUniformBufferRef<FPixelShaderVariableParameters> FPixelShaderVariableParametersRef;
// 
// /************************************************************************/
// /* This is the type we use as vertices for our fullscreen quad.         */
// /************************************************************************/
// struct FTextureVertex
// {
// 	FVector4 Position;
// 	FVector2D UV;
// };
// 
// /************************************************************************/
// /* We define our vertex declaration to let us get our UV coords into    */
// /* the shader                                                           */
// /************************************************************************/
// class FTextureVertexDeclaration : public FRenderResource
// {
// public:
// 	FVertexDeclarationRHIRef VertexDeclarationRHI;
// 
// 	virtual void InitRHI() override
// 	{
// 		FVertexDeclarationElementList Elements;
// 		uint32 Stride = sizeof(FTextureVertex);
// 		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float4, 0, Stride));
// 		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, UV), VET_Float2, 1, Stride));
// 		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
// 	}
// 
// 	virtual void ReleaseRHI() override
// 	{
// 		VertexDeclarationRHI.SafeRelease();
// 	}
// };
// 
// /************************************************************************/
// /* A simple passthrough vertexshader that we will use.                  */
// /************************************************************************/
// class FVertexShaderExample : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FVertexShaderExample, Global);
// public:
// 
// 	static bool ShouldCache(EShaderPlatform Platform) { return true; }
// 
// 	FVertexShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
// 		FGlobalShader(Initializer)
// 	{}
// 	FVertexShaderExample() {}
// };
// 
// 
// /***************************************************************************/
// /* This class is what encapsulates the shader in the engine.               */
// /* It is the main bridge between the HLSL located in the engine directory  */
// /* and the engine itself.                                                  */
// /***************************************************************************/
// class FPixelShaderDeclaration : public FGlobalShader
// {
// 	DECLARE_SHADER_TYPE(FPixelShaderDeclaration, Global);
// 
// public:
// 
// 	FPixelShaderDeclaration() {}
// 
// 	explicit FPixelShaderDeclaration(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
// 
// 	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }
// 
// 	virtual bool Serialize(FArchive& Ar) override
// 	{
// 		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);
// 
// 		Ar << TextureParameter;
// 
// 		return bShaderHasOutdatedParams;
// 	}
// 
// 	//This function is required to let us bind our runtime surface to the shader using an SRV.
// 	void SetSurfaces(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV);
// 	//This function is required to bind our constant / uniform buffers to the shader.
// 	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParameters& ConstantParameters, FPixelShaderVariableParameters& VariableParameters);
// 	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
// 	void UnbindBuffers(FRHICommandList& RHICmdList);
// 
// private:
// 	//This is how you declare resources that are going to be made available in the HLSL
// 	FShaderResourceParameter TextureParameter;
// };
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// //These are needed to actually implement the constant buffers so they are available inside our shader
// //They also need to be unique over the entire solution since they can in fact be accessed from any shader
// IMPLEMENT_UNIFORM_BUFFER_STRUCT(FPixelShaderConstantParameters,
//                                 TEXT("PSConstant"))
// IMPLEMENT_UNIFORM_BUFFER_STRUCT(FPixelShaderVariableParameters,
//                                 TEXT("PSVariable"))
// 
// FPixelShaderDeclaration::FPixelShaderDeclaration(const
//         ShaderMetaType::CompiledShaderInitializerType& Initializer)
//     : FGlobalShader(Initializer) {
//     //This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
//     TextureParameter.Bind(Initializer.ParameterMap,
//                           TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
// }
// 
// void FPixelShaderDeclaration::SetUniformBuffers(FRHICommandList& RHICmdList,
//         FPixelShaderConstantParameters& ConstantParameters,
//         FPixelShaderVariableParameters& VariableParameters) {
//     FPixelShaderConstantParametersRef ConstantParametersBuffer;
//     FPixelShaderVariableParametersRef VariableParametersBuffer;
// 
//     ConstantParametersBuffer =
//         FPixelShaderConstantParametersRef::CreateUniformBufferImmediate(
//             ConstantParameters, UniformBuffer_SingleDraw);
//     VariableParametersBuffer =
//         FPixelShaderVariableParametersRef::CreateUniformBufferImmediate(
//             VariableParameters, UniformBuffer_SingleDraw);
// 
//     SetUniformBufferParameter(RHICmdList, GetPixelShader(),
//                               GetUniformBufferParameter<FPixelShaderConstantParameters>(),
//                               ConstantParametersBuffer);
//     SetUniformBufferParameter(RHICmdList, GetPixelShader(),
//                               GetUniformBufferParameter<FPixelShaderVariableParameters>(),
//                               VariableParametersBuffer);
// }
// 
// void FPixelShaderDeclaration::SetSurfaces(FRHICommandList& RHICmdList,
//         FShaderResourceViewRHIRef TextureParameterSRV) {
//     FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();
// 
//     if (TextureParameter.IsBound()) { //This actually sets the shader resource view to the texture parameter in the shader :)
//         RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI,
//                 TextureParameter.GetBaseIndex(), TextureParameterSRV);
//     }
// }
// 
// void FPixelShaderDeclaration::UnbindBuffers(FRHICommandList& RHICmdList) {
//     FPixelShaderRHIParamRef PixelShaderRHI = GetPixelShader();
// 
//     if (TextureParameter.IsBound()) {
//         RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI,
//                 TextureParameter.GetBaseIndex(), FShaderResourceViewRHIParamRef());
//     }
// }
// 
// // This will tell the engine to create the shader and where the shader entry point is.
// //                           ShaderType             ShaderFileName          Shader function name       Type
// IMPLEMENT_SHADER_TYPE(, FVertexShaderExample, TEXT("PixelShaderExample"), TEXT("MainVertexShader"), SF_Vertex);
// IMPLEMENT_SHADER_TYPE(, FPixelShaderDeclaration, TEXT("PixelShaderExample"), TEXT("MainPixelShader"), SF_Pixel);
// 
// // Needed to make sure the plugin works :)
// IMPLEMENT_MODULE(FDefaultModuleImpl, PixelShader)
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// //It seems to be the convention to expose all vertex declarations as globals, and then reference them as externs in the headers where they are needed.
// //It kind of makes sense since they do not contain any parameters that change and are purely used as their names suggest, as declarations :)
// TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
// 
// FPixelShaderUsageExample::FPixelShaderUsageExample(FColor StartColor, ERHIFeatureLevel::Type ShaderFeatureLevel)
// {
// 	FeatureLevel = ShaderFeatureLevel;
// 
// 	ConstantParameters = FPixelShaderConstantParameters();
// 	ConstantParameters.StartColor = FVector4(StartColor.R / 255.0, StartColor.G / 255.0, StartColor.B / 255.0, StartColor.A / 255.0);
// 	
// 	VariableParameters = FPixelShaderVariableParameters();
// 	
// 	bMustRegenerateSRV = false;
// 	bIsPixelShaderExecuting = false;
// 	bIsUnloading = false;
// 	bSave = false;
// 
// 	CurrentTexture = NULL;
// 	CurrentRenderTarget = NULL;
// 	TextureParameterSRV = NULL;
// }
// 
// FPixelShaderUsageExample::~FPixelShaderUsageExample()
// {
// 	bIsUnloading = true;
// }
// 
// void FPixelShaderUsageExample::ExecutePixelShader(UTextureRenderTarget2D* RenderTarget, FTexture2DRHIRef InputTexture, FColor EndColor, float TextureParameterBlendFactor)
// {
// 	if (bIsUnloading || bIsPixelShaderExecuting) //Skip this execution round if we are already executing
// 	{
// 		return;
// 	}
// 
// 	bIsPixelShaderExecuting = true;
// 
// 	if (TextureParameter != InputTexture)
// 	{
// 		bMustRegenerateSRV = true;
// 	}
// 
// 	//Now set our runtime parameters!
// 	VariableParameters.EndColor = FVector4(EndColor.R / 255.0, EndColor.G / 255.0, EndColor.B / 255.0, EndColor.A / 255.0);
// 	VariableParameters.TextureParameterBlendFactor = TextureParameterBlendFactor;
// 
// 	CurrentRenderTarget = RenderTarget;
// 	TextureParameter = InputTexture;
// 
// 	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
// 	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
// 	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
// 		FPixelShaderRunner,
// 		FPixelShaderUsageExample*, PixelShader, this,
// 		{
// 			PixelShader->ExecutePixelShaderInternal();
// 		}
// 	);
// }
// 
// void FPixelShaderUsageExample::ExecutePixelShaderInternal()
// {
// 	check(IsInRenderingThread());
// 
// 	if (bIsUnloading) //If we are about to unload, so just clean up the SRV :)
// 	{
// 		if (NULL != TextureParameterSRV)
// 		{
// 			TextureParameterSRV.SafeRelease();
// 			TextureParameterSRV = NULL;
// 		}
// 
// 		return;
// 	}
// 
// 	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
// 
// 	//If our input texture reference has changed, we need to recreate our SRV
// 	if (bMustRegenerateSRV)
// 	{
// 		bMustRegenerateSRV = false;
// 
// 		if (NULL != TextureParameterSRV)
// 		{
// 			TextureParameterSRV.SafeRelease();
// 			TextureParameterSRV = NULL;
// 		}
// 
// 		TextureParameterSRV = RHICreateShaderResourceView(TextureParameter, 0);
// 	}
// 
// 	//This is where the magic happens
// 	CurrentTexture = CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
// 	SetRenderTarget(RHICmdList, CurrentTexture, FTextureRHIRef());
// 	RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
// 	RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
// 	RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());
// 	
// 	static FGlobalBoundShaderState BoundShaderState;
// 	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(FeatureLevel));
// 	TShaderMapRef<FPixelShaderDeclaration> PixelShader(GetGlobalShaderMap(FeatureLevel));
// 
// 	SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState, GTextureVertexDeclaration.VertexDeclarationRHI, *VertexShader, *PixelShader);
// 
// 	PixelShader->SetSurfaces(RHICmdList, TextureParameterSRV);
// 	PixelShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
// 
// 	// Draw a fullscreen quad that we can run our pixel shader on
// 	FTextureVertex Vertices[4];
// 	Vertices[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
// 	Vertices[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
// 	Vertices[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
// 	Vertices[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
// 	Vertices[0].UV = FVector2D(0, 0);
// 	Vertices[1].UV = FVector2D(1, 0);
// 	Vertices[2].UV = FVector2D(0, 1);
// 	Vertices[3].UV = FVector2D(1, 1);
// 
// 	DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
// 	
// 	PixelShader->UnbindBuffers(RHICmdList);
// 	
// 	if (bSave) //Save to disk if we have a save request!
// 	{
// 		bSave = false;
// 
// 		SaveScreenshot(RHICmdList);
// 	}
// 
// 	bIsPixelShaderExecuting = false;
// }
// 
// void FPixelShaderUsageExample::SaveScreenshot(FRHICommandListImmediate& RHICmdList)
// {
// 	check(IsInRenderingThread());
// 
// 	TArray<FColor> Bitmap;
// 
// 	FReadSurfaceDataFlags ReadDataFlags;
// 	ReadDataFlags.SetLinearToGamma(false);
// 	ReadDataFlags.SetOutputStencil(false);
// 	ReadDataFlags.SetMip(0); //No mip supported ofc!
// 	
// 	//This is pretty straight forward. Since we are using a standard format, we can use this convenience function instead of having to lock rect.
// 	RHICmdList.ReadSurfaceData(CurrentTexture, FIntRect(0, 0, CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY()), Bitmap, ReadDataFlags);
// 
// 	// if the format and texture type is supported
// 	if (Bitmap.Num())
// 	{
// 		// Create screenshot folder if not already present.
// 		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
// 
// 		const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("VisualizeTexture"));
// 
// 		uint32 ExtendXWithMSAA = Bitmap.Num() / CurrentTexture->GetSizeY();
// 
// 		// Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
// 		FFileHelper::CreateBitmap(*ScreenFileName, ExtendXWithMSAA, CurrentTexture->GetSizeY(), Bitmap.GetData());
// 
// 		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
// 	}
// 	else
// 	{
// 		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
// 	}
// }
