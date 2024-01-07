// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "ShaderDeclarationDemoModule.h"

/**************************************************************************************/
/* This is just an interface we use to keep all the pixel shading code in one file.   */
/**************************************************************************************/
class FVertexAndPixelShader_FullscreenTexturedQuadExample
{
public:
	static void DrawToRenderTarget_RenderThread(FRDGBuilder& RDGBuilder, const FShaderUsageExampleParameters& DrawParameters, FRDGTextureSRVRef OutputTexture);
};
