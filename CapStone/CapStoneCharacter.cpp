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
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
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
	
	// TestWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TestWeapon"));

	// static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_WEAPON(
	// 	TEXT("/Game/Fab/Medieval_long_sword/SKM_medieval_long_sword.SKM_medieval_long_sword"));
	
	// if(SK_WEAPON.Succeeded()){
	// 	TestWeapon->SetSkeletalMesh(SK_WEAPON.Object);
	// 	TestWeapon->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f)); 
	// }

	// Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	// Capsule->SetupAttachment(TestWeapon);
	// Capsule->SetRelativeLocation(FVector(0.f, 0.f, 460.f));
	// Capsule->SetRelativeScale3D(FVector(1.f, 1.f, 5.75f));

	// Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	// Box->SetupAttachment(TestWeapon);
	// Box->SetRelativeLocation(FVector(0.f, 0.f, 210.f));
	// Box->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	// Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Sphere->SetupAttachment(TestWeapon);
	// Sphere->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
	// Sphere->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

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

						switch (KeyIndex)
                        {
                        case 1: RotateUpperArm_r(RotationAmount, 0, 0); break;
                        case 2: RotateUpperArm_r(0, RotationAmount, 0); break;
                        case 3: RotateUpperArm_r(0, 0, RotationAmount); break;

                        case 4: RotateLowerArm_r(RotationAmount, 0, 0); break;
                        case 5: RotateLowerArm_r(0, RotationAmount, 0); break;
                        case 6: RotateLowerArm_r(0, 0, RotationAmount); break;

                        case 7: RotateHand_r(RotationAmount, 0, 0); break;
                        case 8: RotateHand_r(0, RotationAmount, 0); break;
                        case 9: RotateHand_r(0, 0, RotationAmount); break;
                        default:
                            UE_LOG(LogTemp, Warning, TEXT("Invalid Key: %s"), *KeyName);
                            break;
                        }
						//UE_LOG(LogTemp, Log, TEXT("UpperArm_r: %s"), *UpperArm_r.ToString());
						//UE_LOG(LogTemp, Log, TEXT("LowerArm_r: %s"), *LowerArm_r.ToString());
						//UE_LOG(LogTemp, Log, TEXT("Hand_r: %s"), *Hand_r.ToString());
						
						//UE_LOG(LogTemp, Log, TEXT("Pressed Key: %d"), KeyIndex);
					}
                }
            }
        }
    }
}

void ACapStoneCharacter::HandlePlusMinus(const FInputActionValue& Value){
	float InputValue = Value.Get<float>();
	UE_LOG(LogTemp, Log, TEXT("RotationAmount: %f"), RotationAmount);

	RotationAmount = -RotationAmount;
}

// Called every frame
void ACapStoneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACapStoneCharacter::BeginPlay()
{
	Super::BeginPlay();

	FTransform BoneTransform1 = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_Component);
	FRotator LocalRot1 = BoneTransform1.Rotator();
	UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot (RTS_Component): %s"), *LocalRot1.ToString());

	FTransform BoneTransform2 = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_ParentBoneSpace);
	FRotator LocalRot2 = BoneTransform2.Rotator();
	UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot (RTS_ParentBoneSpace): %s"), *LocalRot2.ToString());

	FTransform BoneTransform3 = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_Actor);
	FRotator LocalRot3 = BoneTransform3.Rotator();
	UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot (RTS_Actor): %s"), *LocalRot3.ToString());

	FRotator BoneRotation1 = GetMesh()->GetSocketRotation(FName("upperarm_r"));
	UE_LOG(LogTemp, Log, TEXT("upperarm_r: %s"), *BoneRotation1.ToString());

	FRotator BoneRotation2 = GetMesh()->GetSocketRotation(FName("lowerarm_r"));
	UE_LOG(LogTemp, Log, TEXT("lowerarm_r: %s"), *BoneRotation2.ToString());

	FRotator BoneRotation3 = GetMesh()->GetSocketRotation(FName("hand_r"));
	UE_LOG(LogTemp, Log, TEXT("hand_r: %s"), *BoneRotation3.ToString());

	FName WeaponSocket(TEXT("hand_rSocket"));
	// if(GetMesh()->DoesSocketExist(WeaponSocket))
	// {
	// 	if (TestWeapon){
	// 		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	// 		TestWeapon->AttachToComponent(GetMesh(), TransformRules, WeaponSocket);\
	// 		UE_LOG(LogTemp, Warning, TEXT("TestWeapon is"));
	// 	}
	// 	else{
	// 		UE_LOG(LogTemp, Warning, TEXT("no TestWeapon"));
	// 	}
		
	// 	UE_LOG(LogTemp, Warning, TEXT("Socket is"));
	// }
	// else{
	// 	UE_LOG(LogTemp, Warning, TEXT("no Socket"));
	// }

	// Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	// Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Box->SetCollisionObjectType(ECC_WorldDynamic);
	// Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	// Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Capsule->SetGenerateOverlapEvents(true);
	// Box->SetGenerateOverlapEvents(true);
	// Sphere->SetGenerateOverlapEvents(true);

	// Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACapStoneCharacter::OnWeaponOverlap);
	// Box->OnComponentBeginOverlap.AddDynamic(this, &ACapStoneCharacter::OnWeaponOverlap);
	// Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACapStoneCharacter::OnWeaponOverlap);

}

void ACapStoneCharacter::OnWeaponOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    UE_LOG(LogTemp, Log, TEXT("Weapon overlapped with: %s"), *OtherActor->GetName());
}

void ACapStoneCharacter::OnWeaponHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    if (OtherActor && OtherActor != this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon HIT with: %s"), *OtherActor->GetName());
    }
    else if (OtherActor == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon HIT with SELF!")); 
    }
}

void ACapStoneCharacter::RotateUpperArm_r(float x, float y, float z)
{
	// 현재 Bone의 로컬 회전값 확인 (회전 적용 전 기준)
	FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_ParentBoneSpace);
	FRotator LocalRot = BoneTransform.Rotator();

	// 허용 범위 설정
	// const float MaxRoll = 60.0f, MinRoll = -60.0f;
	// const float MaxPitch = 85.0f, MinPitch = -85.0f;
	// const float MaxYaw = 110.0f, MinYaw = -10.0f;

	const float MaxRoll = 180.0f, MinRoll = -180.0f;
	const float MaxPitch = 180.0f, MinPitch = -180.0f;
	const float MaxYaw = 180.0f, MinYaw = -180.0f;

	// 축별 회전값 적용 전 검사
	if ((x > 0 && LocalRot.Roll < MaxRoll) || (x < 0 && LocalRot.Roll > MinRoll))
	{
		UpperArm_r.Roll += x;
	}
	if ((y > 0 && LocalRot.Pitch < MaxPitch) || (y < 0 && LocalRot.Pitch > MinPitch))
	{
		UpperArm_r.Pitch += y;
	}
	if ((z > 0 && LocalRot.Yaw < MaxYaw) || (z < 0 && LocalRot.Yaw > MinYaw))
	{
		UpperArm_r.Yaw += z;
	}

	// 디버깅 출력
	FTransform NewTransform = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_ParentBoneSpace);
	FRotator NewLocalRot = NewTransform.Rotator();
	UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot (RTS_ParentBoneSpace): %s"), *NewLocalRot.ToString());
	
	// UpperArm_r.Roll = UpperArm_r.Roll + x; //FMath::Clamp(UpperArm_r.Roll + x, -90.0f - 90.f, 90.0f);
	// UpperArm_r.Pitch = UpperArm_r.Pitch + y; //FMath::Clamp(UpperArm_r.Pitch + y, -45.0f, 135.0f);
	// UpperArm_r.Yaw = UpperArm_r.Yaw + z; //FMath::Clamp(UpperArm_r.Yaw + z, -90.0f, 90.0f);

	// FRotator BoneRotation = GetMesh()->GetSocketRotation(FName("upperarm_r"));
	// UE_LOG(LogTemp, Log, TEXT("upperarm_r: %s"), *BoneRotation.ToString());

	// FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("upperarm_r"), RTS_Component);
	// FRotator LocalRot = BoneTransform.Rotator();
	// UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot (ComponentSpace): %s"), *LocalRot.ToString());
	// UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot Pitch (ComponentSpace): %.2f"), LocalRot.Pitch);
	// UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot Yaw (ComponentSpace): %.2f"), LocalRot.Yaw);
	// UE_LOG(LogTemp, Log, TEXT("upperarm_r LocalRot Roll (ComponentSpace): %.2f"), LocalRot.Roll);
}

void ACapStoneCharacter::RotateLowerArm_r(float x, float y, float z)
{
	// 현재 Bone의 로컬 회전값 확인 (회전 적용 전 기준)
	FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("lowerarm_r"), RTS_ParentBoneSpace);
	FRotator LocalRot = BoneTransform.Rotator();

	// 허용 범위 설정
	// const float MaxRoll = 180.0f, MinRoll = -180.0f;
	// const float MaxPitch = 180.0f, MinPitch = -180.0f;
	// const float MaxYaw = 150.0f, MinYaw = 0.0f;

	const float MaxRoll = 180.0f, MinRoll = -180.0f;
	const float MaxPitch = 180.0f, MinPitch = -180.0f;
	const float MaxYaw = 180.0f, MinYaw = -180.0f;

	// 축별 회전값 적용 전 검사
	if ((x > 0 && LocalRot.Roll < MaxRoll) || (x < 0 && LocalRot.Roll > MinRoll))
	{
		LowerArm_r.Roll += x;
	}
	if ((y > 0 && LocalRot.Pitch < MaxPitch) || (y < 0 && LocalRot.Pitch > MinPitch))
	{
		LowerArm_r.Pitch += y;
	}
	if ((z > 0 && LocalRot.Yaw < MaxYaw) || (z < 0 && LocalRot.Yaw > MinYaw))
	{
		LowerArm_r.Yaw += z;
	}

	// 디버깅용 회전값 출력
	FTransform NewTransform = GetMesh()->GetSocketTransform(FName("lowerarm_r"), RTS_ParentBoneSpace);
	FRotator NewLocalRot = NewTransform.Rotator();
	UE_LOG(LogTemp, Log, TEXT("lowerarm_r LocalRot (RTS_ParentBoneSpace): %s"), *NewLocalRot.ToString());

	// LowerArm_r.Roll = LowerArm_r.Roll + x; //FMath::Clamp(LowerArm_r.Roll + x, 0.0f, 135.0f);
	// LowerArm_r.Pitch = LowerArm_r.Pitch + y; //FMath::Clamp(LowerArm_r.Pitch + y, -90.0f, 90.0f);
	// LowerArm_r.Yaw = LowerArm_r.Yaw + z; //FMath::Clamp(LowerArm_r.Yaw + z, -90.0f, 90.0f);

	// FRotator BoneRotation = GetMesh()->GetSocketRotation(FName("lowerarm_r"));
	// UE_LOG(LogTemp, Log, TEXT("lowerarm_r: %s"), *BoneRotation.ToString());

	// FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("lowerarm_r"), RTS_Component);
	// FRotator LocalRot = BoneTransform.Rotator();
	// UE_LOG(LogTemp, Log, TEXT("lowerarm_r LocalRot (ComponentSpace): %s"), *LocalRot.ToString());
}

void ACapStoneCharacter::RotateHand_r(float x, float y, float z)
{
	// 현재 Bone의 로컬 회전값 확인 (회전 적용 전 기준)
	FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("hand_r"), RTS_ParentBoneSpace);
	FRotator LocalRot = BoneTransform.Rotator();

	// 허용 범위 설정
	// const float MaxRoll = 30.0f, MinRoll = -150.0f;
	// const float MaxPitch = 10.0f, MinPitch = -10.0f;
	// const float MaxYaw = 90.0f, MinYaw = -90.0f;

	const float MaxRoll = 180.0f, MinRoll = -180.0f;
	const float MaxPitch = 180.0f, MinPitch = -180.0f;
	const float MaxYaw = 180.0f, MinYaw = -180.0f;

	// 축별 회전값 적용 전 검사
	if ((x > 0 && LocalRot.Roll < MaxRoll) || (x < 0 && LocalRot.Roll > MinRoll))
	{
		Hand_r.Roll += x;
	}
	if ((y < 0 && LocalRot.Pitch < MaxPitch) || (y > 0 && LocalRot.Pitch > MinPitch))
	{
		Hand_r.Pitch += y;
	}
	if ((z < 0 && LocalRot.Yaw < MaxYaw) || (z > 0 && LocalRot.Yaw > MinYaw))
	{
		Hand_r.Yaw += z;
	}

	// 디버깅용 회전값 출력
	FTransform NewTransform = GetMesh()->GetSocketTransform(FName("hand_r"), RTS_ParentBoneSpace);
	FRotator NewLocalRot = NewTransform.Rotator();
	UE_LOG(LogTemp, Log, TEXT("hand_r LocalRot (RTS_ParentBoneSpace): %s"), *NewLocalRot.ToString());

	// Hand_r.Roll = Hand_r.Roll + x; //FMath::Clamp(Hand_r.Roll + x, 0.0f, 0.0f);
	// Hand_r.Pitch = Hand_r.Pitch + y; //FMath::Clamp(Hand_r.Pitch + y, -30.0f, 30.0f);
	// Hand_r.Yaw = Hand_r.Yaw + z; //FMath::Clamp(Hand_r.Yaw + z, -90.0f, 90.0f);

	// FRotator BoneRotation = GetMesh()->GetSocketRotation(FName("hand_r"));
	// UE_LOG(LogTemp, Log, TEXT("hand_r: %s"), *BoneRotation.ToString());

	// FTransform BoneTransform = GetMesh()->GetSocketTransform(FName("hand_r"), RTS_Component);
	// FRotator LocalRot = BoneTransform.Rotator();
	// UE_LOG(LogTemp, Log, TEXT("hand_r LocalRot (ComponentSpace): %s"), *LocalRot.ToString());
}

FRotator ACapStoneCharacter::GetUpperArm_r()
{
	return UpperArm_r;
}

FRotator ACapStoneCharacter::GetLowerArm_r()
{
	return LowerArm_r;
}

FRotator ACapStoneCharacter::GetHand_r()
{
	return Hand_r;
}