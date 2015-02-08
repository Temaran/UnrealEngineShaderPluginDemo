#pragma once
#include "GameFramework/Character.h"
#include "PixelShaderUsageExample.h"
#include "ComputeShaderUsageExample.h"
#include "ShaderPluginDemoCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AShaderPluginDemoCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	AShaderPluginDemoCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

protected:	
	void OnFire();
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	// End of APawn interface

	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/************************************************************************/
	/* Plugin Shader Demo variables!                                        */
	/************************************************************************/
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		FColor PixelShaderTopLeftColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		float ComputeShaderSimulationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		UMaterialInterface* MaterialToApplyToClickedObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		UTextureRenderTarget2D* RenderTarget;

private:
	FPixelShaderUsageExample* PixelShading;
	FComputeShaderUsageExample* ComputeShading;

	float EndColorBuildup;
	float EndColorBuildupDirection;
	float ComputeShaderBlendScalar;
	float ComputeShaderBlend;
	float TotalElapsedTime;

	void ModifyComputeShaderBlend(float NewScalar);
	void SavePixelShaderOutput();
	void SaveComputeShaderOutput();
};

