// Fill out your copyright notice in the Description page of Project Settings.

#include "CapStoneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MyLearningManager.h"
#include "LearningAgentsInteractor.h"
#include "MyLearningAgentsInteractor.h"
#include "LearningAgentsPolicy.h"


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

	// Set ActorCharacters
	ActorCharacters.Empty();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), ACapStoneCharacter::StaticClass(), Actors);

	for (AActor* Actor : Actors)
    {
        ACapStoneCharacter* Character = Cast<ACapStoneCharacter>(Actor);
        if (Character)
        {
			Character->AddTickPrerequisiteActor(this);
			ActorCharacters.Add(Character);
        }
    }

	// Make Interactor
	Interactor = ULearningAgentsInteractor::MakeInteractor(
		LearningAgentsManager, UMyLearningAgentsInteractor::StaticClass());
	
	// Make Policy
	Policy = ULearningAgentsPolicy::MakePolicy(
		LearningAgentsManager, Interactor, ULearningAgentsPolicy::StaticClass());
	if(RunInference)
	{
		
	}
	

}

// Called every frame
void AMyLearningManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

