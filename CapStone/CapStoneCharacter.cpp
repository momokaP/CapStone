// Copyright Epic Games, Inc. All Rights Reserved.

#include "CapStoneCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/PoseableMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Weapon.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACapStoneCharacter

ACapStoneCharacter::ACapStoneCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	RightPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightPoint"));
	RightPoint->SetupAttachment(GetMesh());
	LeftPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftPoint"));
	LeftPoint->SetupAttachment(GetMesh());


	RightHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("RightHandle"));
	LeftHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("LeftHandle"));
    PhysicalAnim = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnim"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACapStoneCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACapStoneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACapStoneCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACapStoneCharacter::Look);
		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACapStoneCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACapStoneCharacter::StopSprint);
	
		// Number
		EnhancedInputComponent->BindAction(NumAction, ETriggerEvent::Triggered, this, &ACapStoneCharacter::HandleRotationInput);
		// +, -
		EnhancedInputComponent->BindAction(PlusMinusAction, ETriggerEvent::Started, this, &ACapStoneCharacter::HandlePlusMinus);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACapStoneCharacter::RLMove(FVector2D MovementVector)
{
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ACapStoneCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACapStoneCharacter::RLLook(FVector2D LookAxisVector)
{
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACapStoneCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACapStoneCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;  // 이동 속도를 SprintSpeed(800.f)로 변경
}

void ACapStoneCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;  // 다시 원래 속도(500.f)로 복귀
}

void ACapStoneCharacter::HandleRotationInput(const FInputActionValue& Value)
{
    // 입력 값 가져오기
    float InputValue = Value.Get<float>();

    // 입력이 0이면 (누르지 않은 상태) 무시
    if (FMath::IsNearlyZero(InputValue))
    {
        return;
    }

	// 문자열을 숫자로 변환하는 맵
    static TMap<FString, int32> KeyMap = {
        { "One", 1 }, { "Two", 2 }, { "Three", 3 },
        { "Four", 4 }, { "Five", 5 }, { "Six", 6 },
        { "Seven", 7 }, { "Eight", 8 }, { "Nine", 9 }
    };

    if (const APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            // NumAction에 매핑된 모든 키 가져오기
            TArray<FKey> BoundKeys = InputSubsystem->QueryKeysMappedToAction(NumAction);

            for (const FKey& Key : BoundKeys)
            {
                if (PC->IsInputKeyDown(Key))
                {
					FString KeyName = Key.ToString();
					if (KeyMap.Contains(KeyName))
                    {
                        int32 KeyIndex = KeyMap[KeyName];

						FVector Origin = GetMesh()->GetSocketLocation("neck_01");
						FVector RightOffset;
						FVector LeftOffset;

						bool bApplyRightOffset = false;
						bool bApplyLeftOffset = false;

						switch (KeyIndex)
                        {
						case 1: RightOffset = FVector(MoveAmount, 0, 0); bApplyRightOffset = true; break;
						case 2: RightOffset = FVector(0, MoveAmount, 0); bApplyRightOffset = true; break;
						case 3: RightOffset = FVector(0, 0, MoveAmount); bApplyRightOffset = true; break;

                        case 4: LeftOffset = FVector(MoveAmount, 0, 0); bApplyLeftOffset = true; break;
                        case 5: LeftOffset = FVector(0, MoveAmount, 0); bApplyLeftOffset = true; break;
                        case 6: LeftOffset = FVector(0, 0, MoveAmount); bApplyLeftOffset = true; break;

						case 7: RightPoint->AddLocalRotation(FRotator(MoveAmount, 0, 0)); break;
						case 8: RightPoint->AddLocalRotation(FRotator(0, MoveAmount, 0)); break;
						case 9: RightPoint->AddLocalRotation(FRotator(0, 0, MoveAmount)); break;

                        default:
                            UE_LOG(LogTemp, Warning, TEXT("Invalid Key: %s"), *KeyName);
                            break;
                        }

						if (bApplyRightOffset)
						{
							FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(RightOffset);
							float NewDistance = FVector::Dist(Origin, NewLocation);
							if (NewDistance <= MaxRange)
							{
								RightPoint->AddLocalOffset(RightOffset);
							}
						}

						if (bApplyLeftOffset)
						{
							FVector NewLocation = LeftPoint->GetComponentLocation() + LeftPoint->GetComponentTransform().TransformVector(LeftOffset);
							float NewDistance = FVector::Dist(Origin, NewLocation);
							if (NewDistance <= MaxRange)
							{
								LeftPoint->AddLocalOffset(LeftOffset);
							}
						}
					}
                }
            }
        }
    }
}

void ACapStoneCharacter::HandlePlusMinus(const FInputActionValue& Value){
	float InputValue = Value.Get<float>();
	UE_LOG(LogTemp, Log, TEXT("MoveAmount: %f"), MoveAmount);

	MoveAmount = -MoveAmount;
}


void ACapStoneCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// GetMesh()->SetNotifyRigidBodyCollision(true);
	// GetMesh()->OnComponentHit.AddDynamic(this, &ACapStoneCharacter::OnMeshHit);

	FTransform HandRightTransform = GetMesh()->GetSocketTransform(hand_r, ERelativeTransformSpace::RTS_World);
	FVector HandRightLocation = HandRightTransform.GetLocation();
    FRotator HandRightRotation = HandRightTransform.GetRotation().Rotator();

	FTransform HandLeftTransform = GetMesh()->GetSocketTransform(hand_l, ERelativeTransformSpace::RTS_World);
	FVector HandLeftLocation = HandLeftTransform.GetLocation();
    FRotator HandLeftRotation = HandLeftTransform.GetRotation().Rotator();

	FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

	if (HandRight)
	{
		AWeapon* HandRightActor = GetWorld()->SpawnActor<AWeapon>(HandRight, HandRightLocation, HandRightRotation, SpawnParams);
		if (HandRightActor)
		{
			HandRightActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("hand_rSocket"));
		}
		
		if (HandRightActor->BoxComponent)
        {
            HandRightActor->BoxComponent->OnComponentHit.AddDynamic(this, &ACapStoneCharacter::OnMeshHit);
        }
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandRight is invalid or not an Actor class"));
	}

	if (HandLeft)
	{
		AWeapon* HandLeftActor = GetWorld()->SpawnActor<AWeapon>(HandLeft, HandLeftLocation, HandLeftRotation, SpawnParams);
		if (HandLeftActor)
		{
			HandLeftActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("hand_lSocket"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandLeft is invalid or not an Actor class"));
	}

	RightPoint->SetWorldLocation(GetMesh()->GetSocketLocation(hand_rSocket));
	RightPoint->SetWorldRotation(FRotator::ZeroRotator);

	RightHandle->GrabComponentAtLocationWithRotation(
		GetMesh(),
		hand_r,
		RightPoint->GetComponentLocation(),
		RightPoint->GetComponentRotation()
	);

	LeftPoint->SetWorldLocation(GetMesh()->GetSocketLocation(hand_lSocket));
	LeftPoint->SetWorldRotation(FRotator::ZeroRotator);

	LeftHandle->GrabComponentAtLocationWithRotation(
		GetMesh(),
		hand_l,
		LeftPoint->GetComponentLocation(),
		LeftPoint->GetComponentRotation()
	);

	FName Pelvis = TEXT("pelvis");
	FName ProfileTest = TEXT("Test");
	PhysicalAnim->SetSkeletalMeshComponent(GetMesh());
	PhysicalAnim->ApplyPhysicalAnimationProfileBelow(Pelvis, ProfileTest, true, false);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(Pelvis, true, false); 

	FName foot_r = TEXT("foot_r");
	FName foot_l = TEXT("foot_l");
	FName spine_03 = TEXT("spine_03");
	FName neck_02 = TEXT("neck_02");
	GetMesh()->SetBodySimulatePhysics(foot_r, false);
	GetMesh()->SetBodySimulatePhysics(foot_l, false);
	GetMesh()->SetBodySimulatePhysics(spine_03, false);
	GetMesh()->SetBodySimulatePhysics(neck_02, false);

	const FVector HandLoc = GetMesh()->GetSocketLocation("hand_r"); 
	const FVector LowerarmLoc = GetMesh()->GetSocketLocation("lowerarm_r");
	const FVector UpperarmLoc = GetMesh()->GetSocketLocation("upperarm_r");

	float ArmLength1 = FVector::Dist(LowerarmLoc, HandLoc);
	float ArmLength2 = FVector::Dist(LowerarmLoc, UpperarmLoc);
	float ArmLength = ArmLength1 + ArmLength2;

	MaxRange = ArmLength * 4.f;

}

// Called every frame
void ACapStoneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RightHandle->SetTargetLocationAndRotation(
		RightPoint->GetComponentLocation(),
		RightPoint->GetComponentRotation()
	);

	LeftHandle->SetTargetLocationAndRotation(
		LeftPoint->GetComponentLocation(),
		LeftPoint->GetComponentRotation()
	);

	DrawDebugSphere(
		GetWorld(),             // 월드 컨텍스트
		LeftPoint->GetComponentLocation(),     // 중심 위치
		5.0f,                  // 반지름
		12,                    // 세그먼트 수 (자세함 정도)
		FColor::Blue,            // 색상
		false,                  // 지속 여부 (true면 계속 표시)
		0.0f,                   // 지속 시간 (false일 때만 유효)
		0,                      // 깊이 우선순위
		0.0f                    // 선 두께
	);

	DrawDebugSphere(
		GetWorld(),             // 월드 컨텍스트
		RightPoint->GetComponentLocation(),     // 중심 위치
		5.0f,                  // 반지름
		12,                    // 세그먼트 수 (자세함 정도)
		FColor::Red,            // 색상
		false,                  // 지속 여부 (true면 계속 표시)
		0.0f,                   // 지속 시간 (false일 때만 유효)
		0,                      // 깊이 우선순위
		0.0f                    // 선 두께
	);

	// UPhysicsHandleComponent 이동 가능 범위
	// DrawDebugSphere(
	// 	GetWorld(),             // 월드 컨텍스트
	// 	GetMesh()->GetSocketLocation("neck_01"),     // 중심 위치
	// 	MaxRange,                  // 반지름
	// 	32,                    // 세그먼트 수 (자세함 정도)
	// 	FColor::Green,            // 색상
	// 	false,                  // 지속 여부 (true면 계속 표시)
	// 	0.0f,                   // 지속 시간 (false일 때만 유효)
	// 	0,                      // 깊이 우선순위
	// 	0.0f                    // 선 두께
	// );

	FTransform HandRightTransform = GetMesh()->GetSocketTransform(hand_r, ERelativeTransformSpace::RTS_World);
	FVector BoneLocation = HandRightTransform.GetLocation();
    FRotator BoneRotation = HandRightTransform.GetRotation().Rotator();

    FRotationMatrix RotMatrix(BoneRotation);

    FVector Forward = RotMatrix.GetUnitAxis(EAxis::X); // 빨강: +X (Forward)
    FVector Right   = RotMatrix.GetUnitAxis(EAxis::Y); // 초록: +Y (Right)
    FVector Up      = RotMatrix.GetUnitAxis(EAxis::Z); // 파랑: +Z (Up)

    float LineLength = 20.f;

    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Forward * LineLength, FColor::Red, false, -1.f, 0, 2.f);
    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Right   * LineLength, FColor::Green, false, -1.f, 0, 2.f);
    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Up      * LineLength, FColor::Blue, false, -1.f, 0, 2.f);

	if(bIsHit)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("충돌 발생!"));
		}
	}

}

void ACapStoneCharacter::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
    {
		FName HitBone = Hit.BoneName;
        if (HitBone != "hand_r" && HitBone != "hand_l")
        {
			UE_LOG(LogTemp, Warning, TEXT("=== OnMeshHit Called ==="));
			UE_LOG(LogTemp, Warning, TEXT("HitComp: %s"), *HitComp->GetName());
			UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Hit BoneName: %s"), *Hit.BoneName.ToString());
			// UE_LOG(LogTemp, Warning, TEXT("Hit !!!!!"));
			// UE_LOG(LogTemp, Warning, TEXT("BoneName: %s"), *GetMesh()->FindClosestBone(Hit.ImpactPoint).ToString());
			
			/*
			UE_LOG(LogTemp, Warning, TEXT("OtherComp: %s"), *OtherComp->GetName());
			UE_LOG(LogTemp, Warning, TEXT("NormalImpulse: %s"), *NormalImpulse.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Hit Location: %s"), *Hit.ImpactPoint.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *GetNameSafe(Hit.GetComponent()));
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *GetNameSafe(Hit.GetActor()));
			*/

			bIsHit = true;
			GetWorld()->GetTimerManager().ClearTimer(HitResetTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				HitResetTimerHandle, 
				this, 
				&ACapStoneCharacter::ResetHitState, 
				HitDuration, 
				false
			);
		}
    }
}

void ACapStoneCharacter::ResetHitState()
{
    bIsHit = false;
}