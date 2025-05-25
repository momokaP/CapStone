// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLearningManager.h"

// Sets default values
AMyLearningManager::AMyLearningManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LearningAgentsManager = CreateDefaultSubobject<ULearningAgentsManager>(TEXT("LearningAgentsManager"));

}

// Called when the game starts or when spawned
void AMyLearningManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyLearningManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

