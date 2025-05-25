// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLearningAgentsInteractor.h"

void UMyLearningAgentsInteractor::SpecifyAgentObservation_Implementation(
    FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
    ULearningAgentsObservationSchema* InObservationSchema
)
{

}

void UMyLearningAgentsInteractor::GatherAgentObservation_Implementation(
    FLearningAgentsObservationObjectElement& OutObservationObjectElement, 
    ULearningAgentsObservationObject* InObservationObject, 
    const int32 AgentId
)
{

}

void UMyLearningAgentsInteractor::SpecifyAgentAction_Implementation(
    FLearningAgentsActionSchemaElement& OutActionSchemaElement, 
    ULearningAgentsActionSchema* InActionSchema
)
{

}

void UMyLearningAgentsInteractor::PerformAgentAction_Implementation(
    const ULearningAgentsActionObject* InActionObject, 
    const FLearningAgentsActionObjectElement& InActionObjectElement, 
    const int32 AgentId
)
{

}

