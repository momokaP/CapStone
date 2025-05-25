// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LearningAgentsManager.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyLearningManager.generated.h"

UCLASS()
class CAPSTONE_API AMyLearningManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULearningAgentsManager* LearningAgentsManager;
	
public:	
	// Sets default values for this actor's properties
	AMyLearningManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
