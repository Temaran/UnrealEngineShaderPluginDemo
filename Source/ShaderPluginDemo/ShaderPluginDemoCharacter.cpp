// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "ShaderPluginDemo.h"
#include "ShaderPluginDemoCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

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
    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>
                                 (TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f,
            64.f); // Position the camera
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>
             (TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow = false;
    Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
    Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

    // Create a gun mesh component
    FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
    FP_Gun->SetOnlyOwnerSee(
        true);          // only the owning player will see this mesh
    FP_Gun->bCastDynamicShadow = false;
    FP_Gun->CastShadow = false;
    //FP_Gun->AttachTo(Mesh1P, TEXT("GripPoint"), EAttachLocation::SnapToTargetIncludingScale, true);

    FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>
                        (TEXT("MuzzleLocation"));
    FP_MuzzleLocation->SetupAttachment(FP_Gun);
    FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

    // Default offset from the character location for projectiles to spawn
    GunOffset = FVector(100.0f, 30.0f, 10.0f);

    // Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
    // derived blueprint asset named MyCharacter (to avoid direct content references in C++)












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

    PixelShading = new FPixelShaderUsageExample(PixelShaderTopLeftColor,
            GetWorld()->Scene->GetFeatureLevel());
    ComputeShading = new FComputeShaderUsageExample(ComputeShaderSimulationSpeed,
            1024, 1024, GetWorld()->Scene->GetFeatureLevel());
}

//Do not forget cleanup :)
void AShaderPluginDemoCharacter::BeginDestroy() {
    Super::BeginDestroy();

    if (PixelShading) {
        delete PixelShading;
    }

    if (ComputeShading) {
        delete ComputeShading;
    }
}

//Saving functions
void AShaderPluginDemoCharacter::SavePixelShaderOutput() {
    PixelShading->Save();
}
void AShaderPluginDemoCharacter::SaveComputeShaderOutput() {
    ComputeShading->Save();
}

void AShaderPluginDemoCharacter::ModifyComputeShaderBlend(float NewScalar) {
    ComputeShaderBlendScalar = NewScalar;
}

void AShaderPluginDemoCharacter::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    TotalElapsedTime += DeltaSeconds;

    if (PixelShading) {
        EndColorBuildup = FMath::Clamp(EndColorBuildup + DeltaSeconds *
                                       EndColorBuildupDirection, 0.0f, 1.0f);

        if (EndColorBuildup >= 1.0 || EndColorBuildup <= 0) {
            EndColorBuildupDirection *= -1;
        }


        FTexture2DRHIRef InputTexture = NULL;

        if (ComputeShading) {
            ComputeShading->ExecuteComputeShader(TotalElapsedTime);
            InputTexture =
                ComputeShading->GetTexture(); //This is the output texture from the compute shader that we will pass to the pixel shader.
        }

        ComputeShaderBlend = FMath::Clamp(ComputeShaderBlend +
                                          ComputeShaderBlendScalar * DeltaSeconds, 0.0f, 1.0f);
        PixelShading->ExecutePixelShader(RenderTarget, InputTexture,
                                         FColor(EndColorBuildup * 255, 0, 0, 255), ComputeShaderBlend);
    }
}

void AShaderPluginDemoCharacter::OnFire() {
    //Try to set a texture to the object we hit!
    FHitResult HitResult;
    FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
    FRotator Direction = FirstPersonCameraComponent->GetComponentRotation();
    FVector EndLocation = StartLocation + Direction.Vector() * 10000;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation,
            ECC_Visibility, QueryParams)) {
        TArray<UStaticMeshComponent*> StaticMeshComponents =
            TArray<UStaticMeshComponent*>();
        AActor* HitActor = HitResult.GetActor();

        if (NULL != HitActor) {
            HitActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

            for (int32 i = 0; i < StaticMeshComponents.Num(); i++) {
                UStaticMeshComponent* CurrentStaticMeshPtr = StaticMeshComponents[i];
                CurrentStaticMeshPtr->SetMaterial(0, MaterialToApplyToClickedObject);
                UMaterialInstanceDynamic* MID =
                    CurrentStaticMeshPtr->CreateAndSetMaterialInstanceDynamic(0);
                UTexture* CastedRenderTarget = Cast<UTexture>(RenderTarget);
                MID->SetTextureParameterValue("InputTexture", CastedRenderTarget);
            }
        }
    }

    // try and play the sound if specified
    if (FireSound != NULL) {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    // try and play a firing animation if specified
    if (FireAnimation != NULL) {
        // Get the animation object for the arms mesh
        UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

        if (AnimInstance != NULL) {
            AnimInstance->Montage_Play(FireAnimation, 1.f);
        }
    }
}

void AShaderPluginDemoCharacter::SetupPlayerInputComponent(
    class UInputComponent* InputComponent) {
    // set up gameplay key bindings
    check(InputComponent);

    //ShaderPluginDemo Specific input mappings
    InputComponent->BindAction("SavePixelShaderOutput", IE_Pressed, this,
                               &AShaderPluginDemoCharacter::SavePixelShaderOutput);
    InputComponent->BindAction("SaveComputeShaderOutput", IE_Pressed, this,
                               &AShaderPluginDemoCharacter::SaveComputeShaderOutput);
    InputComponent->BindAxis("ComputeShaderBlend", this,
                             &AShaderPluginDemoCharacter::ModifyComputeShaderBlend);
    //ShaderPluginDemo Specific input mappings



























    InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    //InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AShaderPluginDemoCharacter::TouchStarted);
    if (EnableTouchscreenMovement(InputComponent) == false) {
        InputComponent->BindAction("Fire", IE_Pressed, this,
                                   &AShaderPluginDemoCharacter::OnFire);
    }

    InputComponent->BindAxis("MoveForward", this,
                             &AShaderPluginDemoCharacter::MoveForward);
    InputComponent->BindAxis("MoveRight", this,
                             &AShaderPluginDemoCharacter::MoveRight);

    // We have 2 versions of the rotation bindings to handle different kinds of devices differently
    // "turn" handles devices that provide an absolute delta, such as a mouse.
    // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
    InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    InputComponent->BindAxis("TurnRate", this,
                             &AShaderPluginDemoCharacter::TurnAtRate);
    InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    InputComponent->BindAxis("LookUpRate", this,
                             &AShaderPluginDemoCharacter::LookUpAtRate);
}

void AShaderPluginDemoCharacter::BeginTouch(const ETouchIndex::Type FingerIndex,
        const FVector Location) {
    if (TouchItem.bIsPressed == true) {
        return;
    }

    TouchItem.bIsPressed = true;
    TouchItem.FingerIndex = FingerIndex;
    TouchItem.Location = Location;
    TouchItem.bMoved = false;
}

void AShaderPluginDemoCharacter::EndTouch(const ETouchIndex::Type FingerIndex,
        const FVector Location) {
    if (TouchItem.bIsPressed == false) {
        return;
    }

    if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false)) {
        OnFire();
    }

    TouchItem.bIsPressed = false;
}

void AShaderPluginDemoCharacter::TouchUpdate(const ETouchIndex::Type
        FingerIndex, const FVector Location) {
    if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex)) {
        if (TouchItem.bIsPressed) {
            if (GetWorld() != nullptr) {
                UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();

                if (ViewportClient != nullptr) {
                    FVector MoveDelta = Location - TouchItem.Location;
                    FVector2D ScreenSize;
                    ViewportClient->GetViewportSize(ScreenSize);
                    FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;

                    if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X) {
                        TouchItem.bMoved = true;
                        float Value = ScaledDelta.X * BaseTurnRate;
                        AddControllerYawInput(Value);
                    }

                    if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y) {
                        TouchItem.bMoved = true;
                        float Value = ScaledDelta.Y * BaseTurnRate;
                        AddControllerPitchInput(Value);
                    }

                    TouchItem.Location = Location;
                }

                TouchItem.Location = Location;
            }
        }
    }
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

bool AShaderPluginDemoCharacter::EnableTouchscreenMovement(
    class UInputComponent* InputComponent) {
    bool bResult = false;

    if (FPlatformMisc::GetUseVirtualJoysticks() ||
            GetDefault<UInputSettings>()->bUseMouseForTouch) {
        bResult = true;
        InputComponent->BindTouch(EInputEvent::IE_Pressed, this,
                                  &AShaderPluginDemoCharacter::BeginTouch);
        InputComponent->BindTouch(EInputEvent::IE_Released, this,
                                  &AShaderPluginDemoCharacter::EndTouch);
        InputComponent->BindTouch(EInputEvent::IE_Repeat, this,
                                  &AShaderPluginDemoCharacter::TouchUpdate);
    }

    return bResult;
}




