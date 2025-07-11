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
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "LearningAgentsManager.h"

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

void ACapStoneCharacter::RLRightPointMove(FVector RightOffset){
	// FVector Direction(
    //     FMath::Sign(RightOffset.X),
    //     FMath::Sign(RightOffset.Y),
    //     FMath::Sign(RightOffset.Z)
    // );

    // FVector Origin = GetMesh()->GetSocketLocation("neck_01");
    // FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(Direction);
    // float NewDistance = FVector::Dist(Origin, NewLocation);

    // if (NewDistance <= MaxRange)
    // {
    //     RightPoint->AddLocalOffset(Direction);
    // }
	
	FVector Origin = GetMesh()->GetSocketLocation("neck_01");

	FVector NewLocation = RightPoint->GetComponentLocation() + RightPoint->GetComponentTransform().TransformVector(RightOffset);
	float NewDistance = FVector::Dist(Origin, NewLocation);
	if (NewDistance <= MaxRange)
	{
		RightPoint->AddLocalOffset(RightOffset);
	}
}

void ACapStoneCharacter::RLLeftPointMove(FVector LeftOffset){
	FVector Origin = GetMesh()->GetSocketLocation("neck_01");

	FVector NewLocation = LeftPoint->GetComponentLocation() + LeftPoint->GetComponentTransform().TransformVector(LeftOffset);
	float NewDistance = FVector::Dist(Origin, NewLocation);
	if (NewDistance <= MaxRange)
	{
		LeftPoint->AddLocalOffset(LeftOffset);
	}
}

void ACapStoneCharacter::MakeEnemyInformation()
{
	EnemyCharacters.Empty();
	EnemyLocation.Empty();
	EnemyDirection.Empty();

	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), ACapStoneCharacter::StaticClass(), EnemyActors);

	struct FEnemyInfo
	{
		ACapStoneCharacter* EnemyChar;
		FVector Location;
		FVector Direction;
		float Distance;

		FEnemyInfo(ACapStoneCharacter* InChar, const FVector& InLoc, const FVector& InDir, float InDist)
			: EnemyChar(InChar), Location(InLoc), Direction(InDir), Distance(InDist) {}
	};

	TArray<FEnemyInfo> EnemyInfoList;

	for (AActor* Actor : EnemyActors)
	{
		ACapStoneCharacter* OtherChar = Cast<ACapStoneCharacter>(Actor);
		if (OtherChar && OtherChar != this && OtherChar->TeamID != this->TeamID)
		{
			FVector Location = OtherChar->GetActorLocation();
			float Distance = FVector::Dist(GetActorLocation(), Location);

			EnemyInfoList.Add(FEnemyInfo(OtherChar, Location, OtherChar->GetActorForwardVector(), Distance));
		}
	}

	// 거리 기준으로 정렬 (오름차순)
	EnemyInfoList.Sort([](const FEnemyInfo& A, const FEnemyInfo& B)
	{
		return A.Distance < B.Distance;
	});

	// 정렬된 정보 복사
	for (const FEnemyInfo& Info : EnemyInfoList)
	{
		EnemyCharacters.Add(Info.EnemyChar);
		EnemyLocation.Add(Info.Location);
		EnemyDirection.Add(Info.Direction);
	}
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

	// LearningAgentsManager 찾기
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(), ManagerTag, Managers
	);
	for (AActor* Actor : Managers)
    {
        ULearningAgentsManager* Manager = 
		Cast<ULearningAgentsManager>(Actor->GetComponentByClass(ULearningAgentsManager::StaticClass()));
		if (Manager)
		{
			Manager->AddAgent(this);
			FoundManager = true;
		}
    }
	if(!FoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find Learning Agents manager."));
	}

	// Origin 찾기
	TArray<AActor*> Origins;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(), OriginTag, Origins
	);
	float Distance = 10000.0f;
	AActor* NearestOrigin = 
	UGameplayStatics::FindNearestActor(GetActorLocation(), Origins, Distance);
	if (NearestOrigin)
	{
		OriginLocation = NearestOrigin->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("OriginLocation: %s"), *OriginLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No nearby origin found within %.2f units"), Distance);
	}

	InitSimulatePhysics();
	InitPointHandle();

    CalculateMaxRange();

    MakeEnemyInformation();
	if(IsTraining)
	{
		RLResetCharacter();
	}
}

void ACapStoneCharacter::CalculateMaxRange()
{
    const FVector HandLoc = GetMesh()->GetSocketLocation("hand_r");
    const FVector LowerarmLoc = GetMesh()->GetSocketLocation("lowerarm_r");
    const FVector UpperarmLoc = GetMesh()->GetSocketLocation("upperarm_r");

    float ArmLength1 = FVector::Dist(LowerarmLoc, HandLoc);
    float ArmLength2 = FVector::Dist(LowerarmLoc, UpperarmLoc);
    float ArmLength = ArmLength1 + ArmLength2;

    MaxRange = ArmLength * 4.f;
}

void ACapStoneCharacter::InitSimulatePhysics()
{
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
}

void ACapStoneCharacter::InitPointHandle()
{
	RightHandle->ReleaseComponent();
	LeftHandle->ReleaseComponent();

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

	ShowDebugSphere();
    ShowRightHandAngle();
}

void ACapStoneCharacter::ShowDebugSphere()
{
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

    // // UPhysicsHandleComponent 이동 가능 범위
    // DrawDebugSphere(
    //     GetWorld(),                              // 월드 컨텍스트
    //     GetMesh()->GetSocketLocation("neck_01"), // 중심 위치
    //     MaxRange,                                // 반지름
    //     32,                                      // 세그먼트 수 (자세함 정도)
    //     FColor::Green,                           // 색상
    //     false,                                   // 지속 여부 (true면 계속 표시)
    //     0.0f,                                    // 지속 시간 (false일 때만 유효)
    //     0,                                       // 깊이 우선순위
    //     0.0f                                     // 선 두께
    // );
}

void ACapStoneCharacter::ShowRightHandAngle()
{
    FTransform HandRightTransform = GetMesh()->GetSocketTransform(hand_r, ERelativeTransformSpace::RTS_World);
    FVector BoneLocation = HandRightTransform.GetLocation();
    FRotator BoneRotation = HandRightTransform.GetRotation().Rotator();

    FRotationMatrix RotMatrix(BoneRotation);

    FVector Forward = RotMatrix.GetUnitAxis(EAxis::X); // 빨강: +X (Forward)
    FVector Right = RotMatrix.GetUnitAxis(EAxis::Y);   // 초록: +Y (Right)
    FVector Up = RotMatrix.GetUnitAxis(EAxis::Z);      // 파랑: +Z (Up)

    float LineLength = 20.f;

    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Forward * LineLength, FColor::Red, false, -1.f, 0, 2.f);
    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Right * LineLength, FColor::Green, false, -1.f, 0, 2.f);
    DrawDebugLine(GetWorld(), BoneLocation, BoneLocation + Up * LineLength, FColor::Blue, false, -1.f, 0, 2.f);
}

void ACapStoneCharacter::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
    {
		FName HitBone = Hit.BoneName;
        if (HitBone != "hand_r" && HitBone != "hand_l")
        {
			AActor* HitActor = Hit.GetActor();
			if(HitActor != nullptr)
			{
				FVector ShotDirection;
				AController* OwnerController = nullptr;

				FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
				APawn* OwnerPawn = Cast<APawn>(GetOwner());
				if(OwnerPawn != nullptr)
				{
					OwnerController = OwnerPawn->GetController();
				}
				HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
			}

			UE_LOG(LogTemp, Warning, TEXT("=== OnMeshHit Called ==="));
			// UE_LOG(LogTemp, Warning, TEXT("HitComp: %s"), *HitComp->GetName());
			// UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
			// UE_LOG(LogTemp, Warning, TEXT("Hit BoneName: %s"), *Hit.BoneName.ToString());
			// UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *GetNameSafe(Hit.GetActor()));
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

float ACapStoneCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApplied = FMath::Min(Health, DamageToApplied);
	Health = Health - DamageToApplied;

	UE_LOG(LogTemp, Warning, TEXT("Health : %f"), Health);

	if(Health<=0)
	{
		IsDead = true;
		UE_LOG(LogTemp, Warning, TEXT("Dead"));

		if(!IsTraining){
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetMesh()->WakeAllRigidBodies();
		}
	}

	return DamageToApplied;
}

void ACapStoneCharacter::RLResetCharacter()
{
	if(EnemyCharacters.Num() <= 0)
	{
		return;
	}
	if(OriginLocation == FVector::ZeroVector)
	{
		return;
	}
	
	GetMesh()->SetSimulatePhysics(false);

	float RandomRadian = FMath::FRandRange(0.f, 6.28f);
	float RandomDistance = FMath::FRandRange(0.5f, 1.f) * ResetDistance;

	float ResetLocationX = 
	EnemyLocation[0].X + FMath::Cos(RandomRadian) * RandomDistance;
	float ResetLocationY = 
	EnemyLocation[0].Y + FMath::Sin(RandomRadian) * RandomDistance;
	FVector ResetLocation = 
	FVector(ResetLocationX, ResetLocationY, EnemyLocation[0].Z);
	
	// Origin 기준 거리 계산
	FVector Direction = ResetLocation - OriginLocation;
	float Distance = Direction.Size();
	// ResetLocation이 Origin으로부터 너무 멀면 반대편으로 보정
	if (Distance > MaxRadius)
	{
		// Direction = Direction.GetSafeNormal();
		// ResetLocation = OriginLocation + Direction * MaxRadius;

		FVector2D XYDirection = FVector2D(Direction.X, Direction.Y).GetSafeNormal();
		FVector2D OppositeXY = -XYDirection * MaxRadius;
		ResetLocation = FVector(OriginLocation.X + OppositeXY.X,
								OriginLocation.Y + OppositeXY.Y,
								ResetLocation.Z); 
	}

	// Enemy와의 거리가 너무 떨어져 있으면 Enemy 방향으로 캐릭터를 위치시킨다
	float EnemyDistance = FVector::Dist(ResetLocation, EnemyLocation[0]);
	if (EnemyDistance > MaxEnemyDistance)
	{
		FVector ToEnemy = (EnemyLocation[0] - ResetLocation).GetSafeNormal();
		float PullAmount = EnemyDistance - MaxEnemyDistance + 100.f;

		ResetLocation += ToEnemy * PullAmount;
	}

	SetActorLocation(ResetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	EnemyCharacters[0]->SetIsDead(false);
	EnemyCharacters[0]->SetHealth(100.0);
	Stamina = 0;

	InitSimulatePhysics();
	InitPointHandle();
}

//////////////////////////////////////////////////////////////////////////
// getter, setter
const TArray<ACapStoneCharacter*>& ACapStoneCharacter::GetEnemyCharacters() const
{
    return EnemyCharacters;
}
const TArray<FVector>& ACapStoneCharacter::GetEnemyLocation() const
{
    return EnemyLocation;
}
const TArray<FVector>& ACapStoneCharacter::GetEnemyDirection() const
{
    return EnemyDirection;
}
USceneComponent* ACapStoneCharacter::GetRightPoint() const
{
    return RightPoint;
}
USceneComponent* ACapStoneCharacter::GetLeftPoint() const
{
    return LeftPoint;
}
int32 ACapStoneCharacter::GetMaxStamina() const
{
    return MaxStamina;
}
int32 ACapStoneCharacter::GetStamina() const
{
    return Stamina;
}
void ACapStoneCharacter::SetStamina(int32 NewStamina)
{
    Stamina = NewStamina;
}
bool ACapStoneCharacter::IsHit() const
{
    return bIsHit;
}
void ACapStoneCharacter::SetIsHit(bool bHit)
{
    bIsHit = bHit;
}
bool ACapStoneCharacter::GetIsDead() const
{
    return IsDead;
}
void ACapStoneCharacter::SetIsDead(bool bDead)
{
    IsDead = bDead;
}
