// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLearningAgentsInteractor.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsManagerListener.h"
#include "CapStoneCharacter.h"

void UMyLearningAgentsInteractor::SpecifyAgentObservation_Implementation(
    FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
    ULearningAgentsObservationSchema* InObservationSchema
)
{
    TMap<FName, FLearningAgentsObservationSchemaElement> Map;
    TMap<FName, FLearningAgentsObservationSchemaElement> EnemyMap;
    TMap<FName, FLearningAgentsObservationSchemaElement> ArmPointMap;

    // Specify Enemy Map
    FLearningAgentsObservationSchemaElement EnemyLocation = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f, TEXT("LocationObservation"));
    
    FLearningAgentsObservationSchemaElement EnemyDirection = 
    ULearningAgentsObservations::SpecifyDirectionObservation(
        InObservationSchema, TEXT("DirectionObservation"));
    
    EnemyMap.Add(TEXT("Location"), EnemyLocation);
    EnemyMap.Add(TEXT("Direction"), EnemyDirection);
    
    FLearningAgentsObservationSchemaElement EnemyStruct = 
    ULearningAgentsObservations::SpecifyStructObservation(
        InObservationSchema, EnemyMap);

    FLearningAgentsObservationSchemaElement EnemyArray = 
    ULearningAgentsObservations::SpecifyArrayObservation(
        InObservationSchema, EnemyStruct, MaxEnemyArrayNum,
        32, 4, 32, TEXT("ArrayObservation")
    );

    // Specify Arm Point Map
    FLearningAgentsObservationSchemaElement RightLocation = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f, TEXT("LocationObservation"));

    FLearningAgentsObservationSchemaElement RightRotation = 
    ULearningAgentsObservations::SpecifyRotationObservation(
        InObservationSchema, TEXT("RotationObservation"));

    FLearningAgentsObservationSchemaElement LeftLocation = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f, TEXT("LocationObservation"));

    FLearningAgentsObservationSchemaElement LeftRotation = 
    ULearningAgentsObservations::SpecifyRotationObservation(
        InObservationSchema, TEXT("RotationObservation"));

    ArmPointMap.Add(TEXT("RLocation"), RightLocation);
    ArmPointMap.Add(TEXT("RRotation"), RightRotation);
    ArmPointMap.Add(TEXT("LLocation"), LeftLocation);
    ArmPointMap.Add(TEXT("LRotation"), LeftRotation);

    FLearningAgentsObservationSchemaElement ArmPointStruct = 
    ULearningAgentsObservations::SpecifyStructObservation(
        InObservationSchema, ArmPointMap);

    // Specify Map
    FLearningAgentsObservationSchemaElement Location = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f, TEXT("LocationObservation"));

    FLearningAgentsObservationSchemaElement Direction = 
    ULearningAgentsObservations::SpecifyDirectionObservation(
        InObservationSchema, TEXT("DirectionObservation"));

    Map.Add(TEXT("MyLocation"), Location);
    Map.Add(TEXT("MyDirection"), Direction);
    Map.Add(TEXT("Enemy"), EnemyArray);
    Map.Add(TEXT("ArmPoint"), ArmPointStruct);

    OutObservationSchemaElement = 
    ULearningAgentsObservations::SpecifyStructObservation(
        InObservationSchema, Map);
}

void UMyLearningAgentsInteractor::GatherAgentObservation_Implementation(
    FLearningAgentsObservationObjectElement& OutObservationObjectElement, 
    ULearningAgentsObservationObject* InObservationObject, 
    const int32 AgentId
)
{
    TMap<FName, FLearningAgentsObservationObjectElement> Map;
    TMap<FName, FLearningAgentsObservationObjectElement> EnemyMap;
    TMap<FName, FLearningAgentsObservationObjectElement> ArmPointMap;

    UObject* ObsActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ACapStoneCharacter* ObsCharacter = Cast<ACapStoneCharacter>(ObsActor);
    if (ObsCharacter)
    {
        // Gather Enemy Map
        FTransform Transform = ObsCharacter->GetActorTransform();

        TArray<FLearningAgentsObservationObjectElement> EnemyElement;
        EnemyElement.Empty();

        const TArray<FVector>& Locations = ObsCharacter->GetEnemyLocation();
        const TArray<FVector>& Directions = ObsCharacter->GetEnemyDirection();
        int32 Count = FMath::Min(Locations.Num(), Directions.Num());

        for (int32 i = 0; i < Count; ++i)
        {
            const FVector& Location = Locations[i];
            const FVector& Direction = Directions[i];

            FLearningAgentsObservationObjectElement EnemyLocation = 
                ULearningAgentsObservations::MakeLocationObservation(
                    InObservationObject, Location, Transform);

            FLearningAgentsObservationObjectElement EnemyDirection = 
                ULearningAgentsObservations::MakeDirectionObservation(
                    InObservationObject, Direction, Transform);

            EnemyMap.Add(TEXT("Location"), EnemyLocation);
            EnemyMap.Add(TEXT("Direction"), EnemyDirection);

            FLearningAgentsObservationObjectElement EnemyStruct = 
            ULearningAgentsObservations::MakeStructObservation(
                InObservationObject, EnemyMap, TEXT("StructObservation"));
            
            EnemyElement.Add(EnemyStruct);
        }

        FLearningAgentsObservationObjectElement EnemyArray =
        ULearningAgentsObservations::MakeArrayObservation(
            InObservationObject, EnemyElement, MaxEnemyArrayNum, TEXT("ArrayObservation")
        );

        // Gather Arm Point Map
        FRotator Rotation = ObsCharacter->GetActorRotation();

        FVector RLocation = ObsCharacter->GetRightPoint()->GetComponentLocation();
        FRotator RRotation = ObsCharacter->GetRightPoint()->GetComponentRotation();
        FVector LLocation = ObsCharacter->GetLeftPoint()->GetComponentLocation();
        FRotator LRotation = ObsCharacter->GetLeftPoint()->GetComponentRotation();

        FLearningAgentsObservationObjectElement RightLocation =
        ULearningAgentsObservations::MakeLocationObservation(
            InObservationObject, RLocation, Transform, TEXT("LocationObservation")
        );
        FLearningAgentsObservationObjectElement RightRotation =
        ULearningAgentsObservations::MakeRotationObservation(
           InObservationObject, RRotation, Rotation, TEXT("RotationObservation")
        );
        FLearningAgentsObservationObjectElement LeftLocation =
        ULearningAgentsObservations::MakeLocationObservation(
            InObservationObject, LLocation, Transform, TEXT("LocationObservation")
        );
        FLearningAgentsObservationObjectElement LeftRotation =
        ULearningAgentsObservations::MakeRotationObservation(
           InObservationObject, LRotation, Rotation, TEXT("RotationObservation")
        );

        ArmPointMap.Add(TEXT("RLocation"), RightLocation);
        ArmPointMap.Add(TEXT("RRotation"), RightRotation);
        ArmPointMap.Add(TEXT("LLocation"), LeftLocation);
        ArmPointMap.Add(TEXT("LRotation"), LeftRotation);

        FLearningAgentsObservationObjectElement ArmPointStruct = 
        ULearningAgentsObservations::MakeStructObservation(
        InObservationObject, ArmPointMap, TEXT("StructObservation"));

        // Gather Map
        FLearningAgentsObservationObjectElement Location = 
        ULearningAgentsObservations::MakeLocationObservation(
            InObservationObject, ObsCharacter->GetActorLocation(), Transform);
        FLearningAgentsObservationObjectElement Direction = 
        ULearningAgentsObservations::MakeDirectionObservation(
            InObservationObject, ObsCharacter->GetActorForwardVector(), Transform);

        Map.Add(TEXT("MyLocation"), Location);
        Map.Add(TEXT("MyDirection"), Direction);
        Map.Add(TEXT("Enemy"), EnemyArray);
        Map.Add(TEXT("ArmPoint"), ArmPointStruct);

        OutObservationObjectElement = 
        ULearningAgentsObservations::MakeStructObservation(
        InObservationObject, Map, TEXT("StructObservation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cast to ACapStoneCharacter failed!"));
    }
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

