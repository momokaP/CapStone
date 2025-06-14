// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/ConstraintInstance.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CapStoneCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACapStoneCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	/** Sprint Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* NumAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PlusMinusAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* RightPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* LeftPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* RightHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* LeftHandle;

	UPROPERTY(VisibleAnywhere)
	UPhysicalAnimationComponent* PhysicalAnim;

	FConstraintInstance* RightConstraint;

	UBoxComponent* WeaponCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsHit = false;

	float HitDuration = 0.2f;	

	FTimerHandle HitResetTimerHandle;

public:
	ACapStoneCharacter();

	// Called every frame
    virtual void Tick(float DeltaTime) override;

    void ShowDebugSphere();

    void ShowRightHandAngle();

    UFUNCTION(BlueprintCallable)	
	void RLMove(FVector2D MovementVector);
	UFUNCTION(BlueprintCallable)	
	void RLLook(FVector2D LookAxisVector);
	
	UFUNCTION(BlueprintCallable)
	void RLRightPointMove(FVector RightOffset);
	UFUNCTION(BlueprintCallable)
	void RLLeftPointMove(FVector LeftOffset);

	void RLResetCharacter();

	// Getter, Setter
	const TArray<FVector>& GetEnemyLocation() const;
	const TArray<FVector>& GetEnemyDirection() const;
	const TArray<ACapStoneCharacter*>& GetEnemyCharacters() const;

	USceneComponent* GetRightPoint() const;
	USceneComponent* GetLeftPoint() const;

	int32 GetMaxStamina() const;
	int32 GetStamina() const;
	void SetStamina(int32 NewStamina);

	bool GetIsDead() const;
	void SetIsDead(bool bDead);

	bool IsHit() const;
	void SetIsHit(bool bHit);

	float GetMaxHealth() const { return MaxHealth; }
	float GetHealth() const { return Health; }
	void SetHealth(float NewHealth) { Health = FMath::Clamp(NewHealth, 0.f, 100.f); }

	float GetMaxEnemyDistance() const { return MaxEnemyDistance; }

	float GetEHRScale() const { return EnemyHealthRewardScale; }
	float GetMHRScale() const { return MyHealthRewardScale; }
	float GetSRScale() const { return StaminaRewardScale; }

protected:
	void ResetHitState();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void HandleRotationInput(const FInputActionValue& Value);

	void HandlePlusMinus(const FInputActionValue& Value);

	void StartSprint();
	void StopSprint();

	UFUNCTION()
	void OnMeshHit(
		UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit
	);

protected:
    virtual void BeginPlay() override;

    void CalculateMaxRange();

    void NewFunction();

    void InitSimulatePhysics();

    virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	TArray<ACapStoneCharacter*> EnemyCharacters;
	TArray<FVector> EnemyLocation;
	TArray<FVector> EnemyDirection;

	void MakeEnemyInformation();
	void InitPointHandle();

	FName hand_rSocket = TEXT("hand_rSocket");
	FName hand_r = TEXT("hand_r");
	FName lowerarm_r = TEXT("lowerarm_r");

	FName hand_lSocket = TEXT("hand_lSocket");
	FName hand_l = TEXT("hand_l");
	FName lowerarm_l = TEXT("lowerarm_l");

	FRotator RightRotator = FRotator::ZeroRotator;
	FRotator LeftRotator = FRotator::ZeroRotator;

	float DefaultWalkSpeed = 250.f;
	float SprintSpeed = 500.f;  // 달리기 속도
	float MoveAmount = 1.f;
	float MaxRange = 1.f;
	float Damage = 10.f;
	float ResetDistance = 800.f;
	float MaxRadius = 400.f;
	float MaxEnemyDistance = 600.f;

	float EnemyHealthRewardScale = 1.0f; 
	float MyHealthRewardScale = 1.0f;
	float StaminaRewardScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = TeamNumber)
	int32 TeamID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = ManagerTag)
	FName ManagerTag;
	bool FoundManager = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = OriginTag)
	FName OriginTag;
	FVector OriginLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsTraining = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsDead = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 Stamina = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 MaxStamina = 5000;

	// 얘네들 c++에서 생성하려면 에디터 완전히 닫고 컴파일, 빌드 해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> HandRight;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> HandLeft;
};

