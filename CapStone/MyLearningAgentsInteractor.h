// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CapStoneCharacter.h"

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "MyLearningAgentsInteractor.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_API UMyLearningAgentsInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()

public:
    virtual void SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema) override;
	
	virtual void GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId) override;
	
	virtual void SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema) override;
	
	virtual void PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId) override;
private:
	FLearningAgentsActionSchemaElement MakeStructAction3Location3Rotation(
		ULearningAgentsActionSchema* InActionSchema
	);

	void ApplyDiscreteActionMove(
		const ULearningAgentsActionObject* InActionObject,
		const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
		const FName& KeyName,
		ACapStoneCharacter* ActCharacter,
		const FVector& MoveAxis,
		void (ACapStoneCharacter::*MoveFunc)(FVector)
	);

	void ApplyDiscreteActionRotate(
		const ULearningAgentsActionObject* InActionObject,
		const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
		const FName& KeyName,
		ACapStoneCharacter* ActCharacter,
		const FRotator& RotateAxis,
		USceneComponent* (ACapStoneCharacter::*GetPointFunc)() const
	);

	int LocationAmount = 5;
	int RotationAmount = 5;

	int MaxEnemyArrayNum = 4;
	int DiscreteActionSize = 3;
};
