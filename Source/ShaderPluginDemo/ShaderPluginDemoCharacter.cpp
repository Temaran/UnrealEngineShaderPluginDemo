#include "ShaderPluginDemo.h"
#include "ShaderPluginDemoCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "SVirtualJoystick.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AShaderPluginDemoCharacter::AShaderPluginDemoCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	//ShaderPluginDemo variables
	EndColorBuildup = 0;
	EndColorBuildupDirection = 1;
	PixelShaderTopLeftColor = FColor::Green;
	ComputeShaderSimulationSpeed = 1.0;
	ComputeShaderBlend = 0.5f;
	ComputeShaderBlendScalar = 0;
	TotalElapsedTime = 0;
}

//Since we need the featurelevel, we need to create the shaders from beginplay, and not in the ctor.
void AShaderPluginDemoCharacter::BeginPlay()
{
	PixelShading = new FPixelShaderUsageExample(PixelShaderTopLeftColor, GetWorld()->Scene->GetFeatureLevel());
	ComputeShading = new FComputeShaderUsageExample(ComputeShaderSimulationSpeed, 1024, 1024, GetWorld()->Scene->GetFeatureLevel());
}

//Do not forget cleanup :)
void AShaderPluginDemoCharacter::BeginDestroy()
{
	Super::BeginDestroy();

	if (NULL != PixelShading)
	{
		delete PixelShading;
	}
	if (NULL != ComputeShading)
	{
		delete ComputeShading;
	}
}

//The idea here is to run the execute functions once per frame. Since we need to pass the compute shader through our pixel shader to get a format that the higher level
//system can understand, we do that here.
void AShaderPluginDemoCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TotalElapsedTime += DeltaSeconds;

	if (NULL != PixelShading)
	{
		EndColorBuildup = FMath::Clamp(EndColorBuildup + DeltaSeconds * EndColorBuildupDirection, 0.0f, 1.0f);
		if (EndColorBuildup >= 1.0 || EndColorBuildup <= 0)
		{
			EndColorBuildupDirection *= -1;
		}
		

		FTexture2DRHIRef InputTexture = NULL;

		if (NULL != ComputeShading)
		{
			ComputeShading->ExecuteComputeShader(TotalElapsedTime);
			InputTexture = ComputeShading->GetTexture(); //This is the output texture from the compute shader that we will pass to the pixel shader.
		}

		ComputeShaderBlend = FMath::Clamp(ComputeShaderBlend + ComputeShaderBlendScalar * DeltaSeconds, 0.0f, 1.0f);
		PixelShading->ExecutePixelShader(RenderTarget, InputTexture, FColor(EndColorBuildup * 255, 0, 0, 255), ComputeShaderBlend);
	}
}

void AShaderPluginDemoCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	//ShaderPluginDemo Specific input mappings
	InputComponent->BindAction("SavePixelShaderOutput", IE_Pressed, this, &AShaderPluginDemoCharacter::SavePixelShaderOutput);
	InputComponent->BindAction("SaveComputeShaderOutput", IE_Pressed, this, &AShaderPluginDemoCharacter::SaveComputeShaderOutput);
	InputComponent->BindAxis("ComputeShaderBlend", this, &AShaderPluginDemoCharacter::ModifyComputeShaderBlend);
	//ShaderPluginDemo Specific input mappings

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	if (EnableTouchscreenMovement(InputComponent) == false)
	{
		InputComponent->BindAction("Fire", IE_Pressed, this, &AShaderPluginDemoCharacter::OnFire);
	}

	InputComponent->BindAxis("MoveForward", this, &AShaderPluginDemoCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AShaderPluginDemoCharacter::MoveRight);

	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AShaderPluginDemoCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AShaderPluginDemoCharacter::LookUpAtRate);
}

//Saving functions
void AShaderPluginDemoCharacter::SavePixelShaderOutput()
{
	PixelShading->Save();
}
void AShaderPluginDemoCharacter::SaveComputeShaderOutput()
{
	ComputeShading->Save();
}

void AShaderPluginDemoCharacter::ModifyComputeShaderBlend(float NewScalar)
{
	ComputeShaderBlendScalar = NewScalar;
}

void AShaderPluginDemoCharacter::OnFire()
{
	//Try to set a texture to the object we hit!
	FHitResult HitResult; 
	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
	FRotator Direction = FirstPersonCameraComponent->GetComponentRotation();
	FVector EndLocation = StartLocation + Direction.Vector() * 10000;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingle(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		TArray<UStaticMeshComponent*> StaticMeshComponents = TArray<UStaticMeshComponent*>();
		AActor* HitActor = HitResult.GetActor();

		if (NULL != HitActor)
		{
			HitActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
			for (int32 i = 0; i < StaticMeshComponents.Num(); i++)
			{
				UStaticMeshComponent* CurrentStaticMeshPtr = StaticMeshComponents[i];
				CurrentStaticMeshPtr->SetMaterial(0, MaterialToApplyToClickedObject);
				UMaterialInstanceDynamic* MID = CurrentStaticMeshPtr->CreateAndSetMaterialInstanceDynamic(0);
				UTexture* CastedRenderTarget = Cast<UTexture>(RenderTarget);
				MID->SetTextureParameterValue("InputTexture", CastedRenderTarget);
			}
		}
	}
	
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void AShaderPluginDemoCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AShaderPluginDemoCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AShaderPluginDemoCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (ScaledDelta.X != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (ScaledDelta.Y != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y* BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void AShaderPluginDemoCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AShaderPluginDemoCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AShaderPluginDemoCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShaderPluginDemoCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AShaderPluginDemoCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AShaderPluginDemoCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AShaderPluginDemoCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AShaderPluginDemoCharacter::TouchUpdate);
	}
	return bResult;
}
