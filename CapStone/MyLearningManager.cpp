// Fill out your copyright notice in the Description page of Project Settings.

#include "MyLearningManager.h"

#include "Kismet/GameplayStatics.h"

#include "LearningAgentsInteractor.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsNeuralNetwork.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsTrainingEnvironment.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"

#include "CapStoneCharacter.h"
#include "MyLearningAgentsInteractor.h"
#include "MyLearningAgentsEnv.h"

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
		LearningAgentsManager, 
		Interactor, 
		ULearningAgentsPolicy::StaticClass(),
		TEXT("Policy"),
		EncoderNN,
		PolicyNN,
		DecoderNN,
		Reinitialize,
		Reinitialize,
		Reinitialize,
		PolicySettings
	);
	if(RunInference)
	{
		Policy->GetEncoderNetworkAsset()->LoadNetworkFromSnapshot(EncoderSnapshot);
		Policy->GetPolicyNetworkAsset()->LoadNetworkFromSnapshot(EncoderSnapshot);
		Policy->GetDecoderNetworkAsset()->LoadNetworkFromSnapshot(EncoderSnapshot);
	}

	// Make Critic
	Critic = ULearningAgentsCritic::MakeCritic(
		LearningAgentsManager,
		Interactor,
		Policy,
		ULearningAgentsCritic::StaticClass(),
		TEXT("Critic"),
		CriticNN,
		Reinitialize,
		CriticSettings
	);

	// Make TrainingEnvironment
	TrainingEnv = ULearningAgentsTrainingEnvironment::MakeTrainingEnvironment(
		LearningAgentsManager, UMyLearningAgentsEnv::StaticClass());
	
	// Make Communicator
	FLearningAgentsTrainerProcess TrainerProcess = 
	ULearningAgentsCommunicatorLibrary::SpawnSharedMemoryTrainingProcess(
		TrainerProcessSettings, SharedMemorySettings
	);
	Communicator = 
	ULearningAgentsCommunicatorLibrary::MakeSharedMemoryCommunicator(
		TrainerProcess, SharedMemorySettings
	);

	// Make PPO Trainer
	PPOTrainer = ULearningAgentsPPOTrainer::MakePPOTrainer(
		LearningAgentsManager,
		Interactor,
		TrainingEnv,
		Policy,
		Critic,
		Communicator,
		ULearningAgentsPPOTrainer::StaticClass(),
		TEXT("PPOTrainer"),
		PPOTrainerSettings
	);
}

// Called every frame
void AMyLearningManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(RunInference)
	{
		Policy->RunInference();
	}
	else
	{
		PPOTrainer->RunTraining(
			PPOTrainingSettings, TrainingGameSettings, true, true);
	}

}

