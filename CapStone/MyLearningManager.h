// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyLearningManager.generated.h"

class ACapStoneCharacter;
class ULearningAgentsInteractor;
// class ULearningAgentsPolicy;
// class ULearningAgentsCritic;
class ULearningAgentsTrainingEnvironment;
class ULearningAgentsNeuralNetwork;

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

private:
	TArray<ACapStoneCharacter*> ActorCharacters;
	
	bool RunInference = false;
	bool Reinitialize = true;

	// Interactor
	ULearningAgentsInteractor* Interactor;

	// Policy
	ULearningAgentsPolicy* Policy;
	
	FFilePath EncoderSnapshot;
	FFilePath PolicySnapshot;
	FFilePath DecoderSnapshot;
	
	FString EncoderNNPath = "";
	ULearningAgentsNeuralNetwork* EncoderNN = 
	LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *EncoderNNPath);
	FString PolicyNNPath = "";
	ULearningAgentsNeuralNetwork* PolicyNN = 
	LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *PolicyNNPath);
	FString DecoderNNPath = "";
	ULearningAgentsNeuralNetwork* DecoderNN = 
	LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *DecoderNNPath);

	FLearningAgentsPolicySettings PolicySettings;

	// Critic
	ULearningAgentsCritic* Critic;
	FString CriticNNPath = "";
	ULearningAgentsNeuralNetwork* CriticNN = 
	LoadObject<ULearningAgentsNeuralNetwork>(nullptr, *CriticNNPath);
	FLearningAgentsCriticSettings CriticSettings;
	
	// TrainingEnvironment
	ULearningAgentsTrainingEnvironment* TrainingEnv;

	// Communicator
	FLearningAgentsCommunicator Communicator;
	FLearningAgentsTrainerProcessSettings TrainerProcessSettings;
	FLearningAgentsSharedMemoryCommunicatorSettings SharedMemorySettings;
	
	// PPO Trainer
	ULearningAgentsPPOTrainer* PPOTrainer;
	FLearningAgentsPPOTrainerSettings PPOTrainerSettings;
	FLearningAgentsPPOTrainingSettings PPOTrainingSettings;
	FLearningAgentsTrainingGameSettings TrainingGameSettings;

};
