// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class CAPSTONE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	USkeletalMeshComponent* SkeletalMesh;

public:	
	UPROPERTY(EditAnywhere)
    class UBoxComponent* BoxComponent;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
