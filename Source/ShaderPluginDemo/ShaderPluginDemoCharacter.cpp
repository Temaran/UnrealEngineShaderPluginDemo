// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ShaderPluginDemoCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AShaderPluginDemoCharacter

AShaderPluginDemoCharacter::AShaderPluginDemoCharacter() {
    // Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);












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
void AShaderPluginDemoCharacter::BeginPlay() {
    // Call the base class
    Super::BeginPlay();
    FP_Gun->AttachToComponent(Mesh1P,
                              FAttachmentTransformRules::SnapToTargetIncludingScale,
                              TEXT("GripPoint")); //Attach gun mesh component to Skeleton, doing it here because the skelton is not yet created in the constructor

//     PixelShading = new FPixelShaderUsageExample(PixelShaderTopLeftColor,
//             GetWorld()->Scene->GetFeatureLevel());
//     ComputeShading = new FComputeShaderUsageExample(ComputeShaderSimulationSpeed,
//             1024, 1024, GetWorld()->Scene->GetFeatureLevel());
}

//Do not forget cleanup :)
void AShaderPluginDemoCharacter::BeginDestroy() {
    Super::BeginDestroy();

//     if (PixelShading) {
//         delete PixelShading;
//     }
// 
//     if (ComputeShading) {
//         delete ComputeShading;
//     }
}

//Saving functions
void AShaderPluginDemoCharacter::SavePixelShaderOutput() {
    //PixelShading->Save();
}
void AShaderPluginDemoCharacter::SaveComputeShaderOutput() {
    //ComputeShading->Save();
}

void AShaderPluginDemoCharacter::ModifyComputeShaderBlend(float NewScalar) {
    ComputeShaderBlendScalar = NewScalar;
}

void AShaderPluginDemoCharacter::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    TotalElapsedTime += DeltaSeconds;

//     if (PixelShading) {
//         EndColorBuildup = FMath::Clamp(EndColorBuildup + DeltaSeconds *
//                                        EndColorBuildupDirection, 0.0f, 1.0f);
// 
//         if (EndColorBuildup >= 1.0 || EndColorBuildup <= 0) {
//             EndColorBuildupDirection *= -1;
//         }
// 
// 
//         FTexture2DRHIRef InputTexture = NULL;
// 
//         if (ComputeShading) {
//             ComputeShading->ExecuteComputeShader(TotalElapsedTime);
//             InputTexture =
//                 ComputeShading->GetTexture(); //This is the output texture from the compute shader that we will pass to the pixel shader.
//         }
// 
//         ComputeShaderBlend = FMath::Clamp(ComputeShaderBlend +
//                                           ComputeShaderBlendScalar * DeltaSeconds, 0.0f, 1.0f);
//         PixelShading->ExecutePixelShader(RenderTarget, InputTexture,
//                                          FColor(EndColorBuildup * 255, 0, 0, 255), ComputeShaderBlend);
//     }
}

void AShaderPluginDemoCharacter::OnFire() {
    //Try to set a texture to the object we hit!
//     FHitResult HitResult;
//     FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
//     FRotator Direction = FirstPersonCameraComponent->GetComponentRotation();
//     FVector EndLocation = StartLocation + Direction.Vector() * 10000;
//     FCollisionQueryParams QueryParams;
//     QueryParams.AddIgnoredActor(this);
// 
//     if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation,
//             ECC_Visibility, QueryParams)) {
//         TArray<UStaticMeshComponent*> StaticMeshComponents =
//             TArray<UStaticMeshComponent*>();
//         AActor* HitActor = HitResult.GetActor();
// 
//         if (NULL != HitActor) {
//             HitActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
// 
//             for (int32 i = 0; i < StaticMeshComponents.Num(); i++) {
//                 UStaticMeshComponent* CurrentStaticMeshPtr = StaticMeshComponents[i];
//                 CurrentStaticMeshPtr->SetMaterial(0, MaterialToApplyToClickedObject);
//                 UMaterialInstanceDynamic* MID =
//                     CurrentStaticMeshPtr->CreateAndSetMaterialInstanceDynamic(0);
//                 UTexture* CastedRenderTarget = Cast<UTexture>(RenderTarget);
//                 MID->SetTextureParameterValue("InputTexture", CastedRenderTarget);
//             }
//         }
//     }
// 
//     // try and play the sound if specified
//     if (FireSound != NULL) {
//         UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
//     }
// 
//     // try and play a firing animation if specified
//     if (FireAnimation != NULL) {
//         // Get the animation object for the arms mesh
//         UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
// 
//         if (AnimInstance != NULL) {
//             AnimInstance->Montage_Play(FireAnimation, 1.f);
//         }
//     }
}

void AShaderPluginDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShaderPluginDemoCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShaderPluginDemoCharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShaderPluginDemoCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AShaderPluginDemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShaderPluginDemoCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShaderPluginDemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShaderPluginDemoCharacter::LookUpAtRate);

    //ShaderPluginDemo Specific input mappings
	PlayerInputComponent->BindAction("SavePixelShaderOutput", IE_Pressed, this,
                               &AShaderPluginDemoCharacter::SavePixelShaderOutput);
	PlayerInputComponent->BindAction("SaveComputeShaderOutput", IE_Pressed, this,
                               &AShaderPluginDemoCharacter::SaveComputeShaderOutput);
	PlayerInputComponent->BindAxis("ComputeShaderBlend", this,
                             &AShaderPluginDemoCharacter::ModifyComputeShaderBlend);
    //ShaderPluginDemo Specific input mappings


}

void AShaderPluginDemoCharacter::MoveForward(float Value) {
    if (Value != 0.0f) {
        // add movement in that direction
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void AShaderPluginDemoCharacter::MoveRight(float Value) {
    if (Value != 0.0f) {
        // add movement in that direction
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void AShaderPluginDemoCharacter::TurnAtRate(float Rate) {
    // calculate delta for this frame from the rate information
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShaderPluginDemoCharacter::LookUpAtRate(float Rate) {
    // calculate delta for this frame from the rate information
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
