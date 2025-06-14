// Fill out your copyright notice in the Description page of Project Settings.

#include "MyLearningAgentsEnv.h"

#include "CapStoneCharacter.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "LearningAgentsManagerListener.h"


void UMyLearningAgentsEnv::GatherAgentReward_Implementation(
    float& OutReward, const int32 AgentId
)
{
    UObject* RewardActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ACapStoneCharacter* RewardCharacter = Cast<ACapStoneCharacter>(RewardActor);
    if (RewardCharacter)
    {
        // 기본 reward
        // 적과의 거리 reward
        FVector MyLocation = RewardCharacter->GetActorLocation();
        const TArray<FVector>& EnemyLocations = RewardCharacter->GetEnemyLocation();
        if(EnemyLocations.Num() <= 0)
        {
            return;
        }
        float DistanceReward = ULearningAgentsRewards::MakeRewardFromLocationSimilarity(
            MyLocation, EnemyLocations[0], 100.0f, 1.0f);

        // 적 죽음 reward
        const TArray<ACapStoneCharacter*>& Enemies = RewardCharacter->GetEnemyCharacters();
        if(Enemies.Num() <= 0)
        {
            return;
        }
        float EnemyDeadReward = ULearningAgentsRewards::MakeRewardOnCondition(
            Enemies[0]->GetIsDead(), 100.0f);
        // 적 타격 reward
        float HitReward = ULearningAgentsRewards::MakeRewardOnCondition(
            RewardCharacter->IsHit(), 10.0f);

        // custom 조절 reward
        float EnemyHealthReward = ULearningAgentsRewards::MakeReward(
            1-Enemies[0]->GetHealth()/Enemies[0]->GetMaxHealth(),
            RewardCharacter->GetEHRScale() 
        );
        float MyHealthReward = ULearningAgentsRewards::MakeReward(
            RewardCharacter->GetHealth()/RewardCharacter->GetMaxHealth(),
            RewardCharacter->GetMHRScale()
        );
        float StaminaReward = ULearningAgentsRewards::MakeReward(
            RewardCharacter->GetStamina()/RewardCharacter->GetMaxStamina(),
            RewardCharacter->GetSRScale()
        );
    
        OutReward = DistanceReward + EnemyDeadReward + HitReward + EnemyHealthReward + MyHealthReward + StaminaReward;
    }
}

void UMyLearningAgentsEnv::GatherAgentCompletion_Implementation(
    ELearningAgentsCompletion& OutCompletion, const int32 AgentId
)
{
    UObject* CompletionActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ACapStoneCharacter* CompletionCharacter = Cast<ACapStoneCharacter>(CompletionActor);
    if (CompletionCharacter)
    {
        const TArray<ACapStoneCharacter*>& Enemies = CompletionCharacter->GetEnemyCharacters();
        if(Enemies.Num() <= 0)
        {
            return;
        }
        ELearningAgentsCompletion DeadCompletion = 
        ULearningAgentsCompletions::MakeCompletionOnCondition(Enemies[0]->GetIsDead());

        bool IsStaminaOver = 
        CompletionCharacter->GetStamina() > CompletionCharacter->GetMaxStamina();
        ELearningAgentsCompletion StaminaCompletion =
        ULearningAgentsCompletions::MakeCompletionOnCondition(IsStaminaOver);

        FVector MyLocation = CompletionCharacter->GetActorLocation();
        const TArray<FVector>& EnemyLocations = CompletionCharacter->GetEnemyLocation();
        if(EnemyLocations.Num() <= 0)
        {
            return;
        }
        ELearningAgentsCompletion DistanceCompletion = 
        ULearningAgentsCompletions::MakeCompletionOnLocationDifferenceAboveThreshold(
            MyLocation, EnemyLocations[0], CompletionCharacter->GetMaxEnemyDistance());

        OutCompletion = 
        ULearningAgentsCompletions::CompletionOr(
            ULearningAgentsCompletions::CompletionOr(
                DeadCompletion, StaminaCompletion), DistanceCompletion);
        
    }
}

void UMyLearningAgentsEnv::ResetAgentEpisode_Implementation(
    const int32 AgentId
)
{
    UObject* ResetActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ACapStoneCharacter* ResetCharacter = Cast<ACapStoneCharacter>(ResetActor);
    if (ResetCharacter)
    {
        ResetCharacter->RLResetCharacter();
    }
}

