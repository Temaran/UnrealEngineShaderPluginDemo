// Copyright 2016-2020 Cadic AB. All Rights Reserved.
// @Author	Fredrik Lindh [Temaran] (temaran@gmail.com) {https://github.com/Temaran}
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/*
 * This module contains demonstration code for using the classes in the ShaderDeclarationDemo module.
 */
class SHADERUSAGEDEMO_API FShaderUsageDemoModule : public IModuleInterface
{
public:
	static inline FShaderUsageDemoModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FShaderUsageDemoModule>("ShaderUsageDemo");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ShaderUsageDemo");
	}
};
