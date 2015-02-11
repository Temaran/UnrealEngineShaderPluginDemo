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

#include "ShaderCopyHelperPrivatePCH.h" 
#include "Developer/DesktopPlatform/public/DesktopPlatformModule.h"
#include "GenericPlatformFile.h"
#include "PlatformFilemanager.h"

DEFINE_LOG_CATEGORY_STATIC(ShaderCopyHelper, Log, All);

void FShaderCopyHelperModule::StartupModule()
{
	UE_LOG(ShaderCopyHelper, Log, TEXT("Shader Copy Helper Plugin loaded!"));

	FString GameShadersDirectory = FPaths::Combine(*FPaths::GameDir(), TEXT("Shaders"));
	FString EngineShadersDirectory = FPaths::Combine(*FPaths::EngineDir(), TEXT("Shaders"));

	ShaderFiles = new FShaderFileVisitor();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.IterateDirectoryRecursively(*GameShadersDirectory, *ShaderFiles);
	
	UE_LOG(ShaderCopyHelper, Log, TEXT("Copying project shader files to Engine/Shaders/"));
	for (int32 ShaderFileIndex = 0; ShaderFileIndex < ShaderFiles->ShaderFilePaths.Num(); ShaderFileIndex++)
	{
		FString CurrentShaderFile = ShaderFiles->ShaderFilePaths[ShaderFileIndex];
		FString GameShaderFullPath = FPaths::Combine(*GameShadersDirectory, *CurrentShaderFile);
		FString EngineShaderFullPath = FPaths::Combine(*EngineShadersDirectory, *CurrentShaderFile);

		if (PlatformFile.CopyFile(*EngineShaderFullPath, *GameShaderFullPath))
		{
			UE_LOG(ShaderCopyHelper, Log, TEXT("Shader file %s copied to %s."), *GameShaderFullPath, *EngineShaderFullPath);
		}
		else
		{
			UE_LOG(ShaderCopyHelper, Warning, TEXT("Could not copy %s to %s!"), *GameShaderFullPath, *EngineShaderFullPath);
		}
	}
}

void FShaderCopyHelperModule::ShutdownModule()
{
	FString EngineShadersDirectory = FPaths::Combine(*FPaths::EngineDir(), TEXT("Shaders"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	UE_LOG(ShaderCopyHelper, Log, TEXT("Deleting project shaders from Engine/Shaders/"));
	for (int32 ShaderFileIndex = 0; ShaderFileIndex < ShaderFiles->ShaderFilePaths.Num(); ShaderFileIndex++)
	{
		FString EngineShaderFullPath = FPaths::Combine(*EngineShadersDirectory, *ShaderFiles->ShaderFilePaths[ShaderFileIndex]);

		if (PlatformFile.DeleteFile(*EngineShaderFullPath))
		{
			UE_LOG(ShaderCopyHelper, Log, TEXT("Shader file %s deleted."), *EngineShaderFullPath);
		}
		else
		{
			UE_LOG(ShaderCopyHelper, Warning, TEXT("Could not delete %s!"), *EngineShaderFullPath);
		}
	}

	delete ShaderFiles;

	UE_LOG(ShaderCopyHelper, Log, TEXT("Shader Copy Helper Plugin unloaded!"));
}

IMPLEMENT_MODULE(FShaderCopyHelperModule, ShaderCopyHelper)