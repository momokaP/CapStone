// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLearningAgentsInteractor.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsManagerListener.h"
#include "LearningAgentsActions.h"
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
        InObservationSchema, 100.f);
    
    FLearningAgentsObservationSchemaElement EnemyDirection = 
    ULearningAgentsObservations::SpecifyDirectionObservation(
        InObservationSchema);
    
    EnemyMap.Add(TEXT("Location"), EnemyLocation);
    EnemyMap.Add(TEXT("Direction"), EnemyDirection);
    
    FLearningAgentsObservationSchemaElement EnemyStruct = 
    ULearningAgentsObservations::SpecifyStructObservation(
        InObservationSchema, EnemyMap);

    FLearningAgentsObservationSchemaElement EnemyArray = 
    ULearningAgentsObservations::SpecifyArrayObservation(
        InObservationSchema, EnemyStruct, MaxEnemyArrayNum
    );

    // Specify Arm Point Map
    FLearningAgentsObservationSchemaElement RightLocation = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement RightRotation = 
    ULearningAgentsObservations::SpecifyRotationObservation(
        InObservationSchema);

    FLearningAgentsObservationSchemaElement LeftLocation = 
    ULearningAgentsObservations::SpecifyLocationObservation(
        InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement LeftRotation = 
    ULearningAgentsObservations::SpecifyRotationObservation(
        InObservationSchema);

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
        InObservationSchema, 100.f);

    FLearningAgentsObservationSchemaElement Direction = 
    ULearningAgentsObservations::SpecifyDirectionObservation(
        InObservationSchema);

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
                InObservationObject, EnemyMap);
            
            EnemyElement.Add(EnemyStruct);
        }

        FLearningAgentsObservationObjectElement EnemyArray =
        ULearningAgentsObservations::MakeArrayObservation(
            InObservationObject, EnemyElement, MaxEnemyArrayNum
        );

        // Gather Arm Point Map
        FRotator Rotation = ObsCharacter->GetActorRotation();

        FVector RLocation = ObsCharacter->GetRightPoint()->GetComponentLocation();
        FRotator RRotation = ObsCharacter->GetRightPoint()->GetComponentRotation();
        FVector LLocation = ObsCharacter->GetLeftPoint()->GetComponentLocation();
        FRotator LRotation = ObsCharacter->GetLeftPoint()->GetComponentRotation();

        FLearningAgentsObservationObjectElement RightLocation =
        ULearningAgentsObservations::MakeLocationObservation(
            InObservationObject, RLocation, Transform
        );
        FLearningAgentsObservationObjectElement RightRotation =
        ULearningAgentsObservations::MakeRotationObservation(
           InObservationObject, RRotation, Rotation
        );
        FLearningAgentsObservationObjectElement LeftLocation =
        ULearningAgentsObservations::MakeLocationObservation(
            InObservationObject, LLocation, Transform
        );
        FLearningAgentsObservationObjectElement LeftRotation =
        ULearningAgentsObservations::MakeRotationObservation(
           InObservationObject, LRotation, Rotation
        );

        ArmPointMap.Add(TEXT("RLocation"), RightLocation);
        ArmPointMap.Add(TEXT("RRotation"), RightRotation);
        ArmPointMap.Add(TEXT("LLocation"), LeftLocation);
        ArmPointMap.Add(TEXT("LRotation"), LeftRotation);

        FLearningAgentsObservationObjectElement ArmPointStruct = 
        ULearningAgentsObservations::MakeStructObservation(
        InObservationObject, ArmPointMap);

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
        InObservationObject, Map);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cast to ACapStoneCharacter failed!"));
    }
}

FLearningAgentsActionSchemaElement UMyLearningAgentsInteractor::MakeStructAction3Location3Rotation(
    ULearningAgentsActionSchema* InActionSchema
)
{
    TMap<FName, FLearningAgentsActionSchemaElement> ActionMap;

    FLearningAgentsActionSchemaElement LocationX = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement LocationY = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement LocationZ = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});

    FLearningAgentsActionSchemaElement RotationX = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement RotationY = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});
    FLearningAgentsActionSchemaElement RotationZ = 
    ULearningAgentsActions::SpecifyExclusiveDiscreteAction(
        InActionSchema, DiscreteActionSize, {});
    
    ActionMap.Add(TEXT("LocationX"), LocationX);
    ActionMap.Add(TEXT("LocationY"), LocationY);
    ActionMap.Add(TEXT("LocationZ"), LocationZ);
    ActionMap.Add(TEXT("RotationX"), RotationX);
    ActionMap.Add(TEXT("RotationY"), RotationY);
    ActionMap.Add(TEXT("RotationZ"), RotationZ);

    return ULearningAgentsActions::SpecifyStructAction(InActionSchema, ActionMap);
}

void UMyLearningAgentsInteractor::SpecifyAgentAction_Implementation(
    FLearningAgentsActionSchemaElement& OutActionSchemaElement, 
    ULearningAgentsActionSchema* InActionSchema
)
{
    TMap<FName, FLearningAgentsActionSchemaElement> Map;

    // Specify Movement
    TMap<FName, FLearningAgentsActionSchemaElement> MovementMap;
    
    FLearningAgentsActionSchemaElement XMovement = 
    ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f);
    FLearningAgentsActionSchemaElement YMovement = 
    ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f);

    MovementMap.Add(TEXT("X"), XMovement);
    MovementMap.Add(TEXT("Y"), YMovement);

    FLearningAgentsActionSchemaElement MovementStruct = 
    ULearningAgentsActions::SpecifyStructAction(
        InActionSchema, MovementMap);

    // Specify Rotation
    FLearningAgentsActionSchemaElement Rotation = 
    ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f);

    // Specify Right, Left Action
    FLearningAgentsActionSchemaElement RightStruct = 
    MakeStructAction3Location3Rotation(InActionSchema);
    FLearningAgentsActionSchemaElement LeftStruct = 
    MakeStructAction3Location3Rotation(InActionSchema);

    // Specify Map
    Map.Add(TEXT("Movement"), MovementStruct);
    Map.Add(TEXT("Rotation"), Rotation);
    Map.Add(TEXT("Right"), RightStruct);
    Map.Add(TEXT("Left"), LeftStruct);

    OutActionSchemaElement = 
    ULearningAgentsActions::SpecifyStructAction(
        InActionSchema, Map);

}

void UMyLearningAgentsInteractor::PerformAgentAction_Implementation(
    const ULearningAgentsActionObject* InActionObject, 
    const FLearningAgentsActionObjectElement& InActionObjectElement, 
    const int32 AgentId
)
{
    UObject* ActActor = ULearningAgentsManagerListener::GetAgent(AgentId);
    ACapStoneCharacter* ActCharacter = Cast<ACapStoneCharacter>(ActActor);
    if (ActCharacter)
    {
        TMap<FName, FLearningAgentsActionObjectElement> OutActions;

        ULearningAgentsActions::GetStructAction(OutActions, InActionObject, InActionObjectElement);
        
        // Perform Movement
        TMap<FName, FLearningAgentsActionObjectElement> MovementActions;
        ULearningAgentsActions::GetStructAction(
            MovementActions, InActionObject, *OutActions.Find(TEXT("Movement")));
        
        float XMovement;
        ULearningAgentsActions::GetFloatAction(
            XMovement, InActionObject, *MovementActions.Find(TEXT("X"))
        );
        ActCharacter->RLMove(FVector2D(XMovement, 0.0f));

        float YMovement;
        ULearningAgentsActions::GetFloatAction(
            YMovement, InActionObject, *MovementActions.Find(TEXT("Y"))
        );
        ActCharacter->RLMove(FVector2D(0.0f, YMovement));

        // Perform Rotation
        float Rotation;
        ULearningAgentsActions::GetFloatAction(
            Rotation, InActionObject, *OutActions.Find(TEXT("Rotation"))
        );
        ActCharacter->RLLook(FVector2D(Rotation, 0.0f));

        // Perform Right
        TMap<FName, FLearningAgentsActionObjectElement> Right;
        ULearningAgentsActions::GetStructAction(
            Right, InActionObject, *OutActions.Find(TEXT("Right")));
    
        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationX"), 
        ActCharacter, FVector(1, 0, 0), &ACapStoneCharacter::RLRightPointMove);
        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationY"), 
        ActCharacter, FVector(0, 1, 0), &ACapStoneCharacter::RLRightPointMove);
        ApplyDiscreteActionMove(InActionObject, Right, TEXT("LocationZ"), 
        ActCharacter, FVector(0, 0, 1), &ACapStoneCharacter::RLRightPointMove);

        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationX"),
        ActCharacter, FRotator(1, 0, 0), &ACapStoneCharacter::GetRightPoint);
        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationY"),
        ActCharacter, FRotator(0, 1, 0), &ACapStoneCharacter::GetRightPoint);
        ApplyDiscreteActionRotate(InActionObject, Right, TEXT("RotationZ"),
        ActCharacter, FRotator(0, 0, 1), &ACapStoneCharacter::GetRightPoint);
    
        // Perform Left
        TMap<FName, FLearningAgentsActionObjectElement> Left;
        ULearningAgentsActions::GetStructAction(
            Left, InActionObject, *OutActions.Find(TEXT("Left")));
    
        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationX"), 
        ActCharacter, FVector(1, 0, 0), &ACapStoneCharacter::RLLeftPointMove);
        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationY"), 
        ActCharacter, FVector(0, 1, 0), &ACapStoneCharacter::RLLeftPointMove);
        ApplyDiscreteActionMove(InActionObject, Left, TEXT("LocationZ"), 
        ActCharacter, FVector(0, 0, 1), &ACapStoneCharacter::RLLeftPointMove);

        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationX"),
        ActCharacter, FRotator(1, 0, 0), &ACapStoneCharacter::GetLeftPoint);
        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationY"),
        ActCharacter, FRotator(0, 1, 0), &ACapStoneCharacter::GetLeftPoint);
        ApplyDiscreteActionRotate(InActionObject, Left, TEXT("RotationZ"),
        ActCharacter, FRotator(0, 0, 1), &ACapStoneCharacter::GetLeftPoint);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cast to ACapStoneCharacter failed!"));
    }
}

// 원본 코드
// int32 LocationX;
// ULearningAgentsActions::GetExclusiveDiscreteAction(
//     LocationX, InActionObject, *Right.Find(TEXT("LocationX")));
// LocationX = (LocationX - 1) * LocationAmount;
// ActCharacter->RLRightPointMove(FVector((float)LocationX, 0.0f, 0.0f));

void UMyLearningAgentsInteractor::ApplyDiscreteActionMove(
    const ULearningAgentsActionObject* InActionObject,
    const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
    const FName& KeyName,
    ACapStoneCharacter* ActCharacter,
    const FVector& MoveAxis,
    void (ACapStoneCharacter::*MoveFunc)(FVector)
)
{
    int32 LocationIndex;
    if (ULearningAgentsActions::GetExclusiveDiscreteAction(LocationIndex, InActionObject, *ActionMap.Find(KeyName)))
    {
        int32 AdjustedLocation = (LocationIndex - 1) * LocationAmount;
        FVector MoveVector = MoveAxis * (float)AdjustedLocation;
        (ActCharacter->*MoveFunc)(MoveVector);

        if(AdjustedLocation != 0)
        {
            ActCharacter->SetStamina(ActCharacter->GetStamina() + 1);
        }
    }
}

void UMyLearningAgentsInteractor::ApplyDiscreteActionRotate(
    const ULearningAgentsActionObject* InActionObject,
    const TMap<FName, FLearningAgentsActionObjectElement>& ActionMap,
    const FName& KeyName,
    ACapStoneCharacter* ActCharacter,
    const FRotator& RotateAxis,
    USceneComponent* (ACapStoneCharacter::*GetPointFunc)() const
)
{
    int32 RotationIndex;
    if (ULearningAgentsActions::GetExclusiveDiscreteAction(RotationIndex, InActionObject, *ActionMap.Find(KeyName)))
    {
        int32 AdjustedRotation = (RotationIndex - 1) * RotationAmount;
        FRotator RotateVector = RotateAxis * (float)AdjustedRotation;

        USceneComponent* TargetComponent = (ActCharacter->*GetPointFunc)();
        if (TargetComponent)
        {
            TargetComponent->AddLocalRotation(RotateVector);

            if (AdjustedRotation != 0)
            {
                ActCharacter->SetStamina(ActCharacter->GetStamina() + 1);
            }
        }
    }
}