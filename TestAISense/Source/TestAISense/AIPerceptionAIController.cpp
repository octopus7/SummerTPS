// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPerceptionAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "TopDownPlayerCharacter.h"
#include "Engine/Engine.h"

AAIPerceptionAIController::AAIPerceptionAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 1000.f;
    SightConfig->LoseSightRadius = 1500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObject(TEXT("/Game/AI/BT_AIPerception"));
    if (BTObject.Succeeded())
    {
        BehaviorTree = BTObject.Object;
    }
}

void AAIPerceptionAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTree)
    {
        BlackboardComponent->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }

    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIPerceptionAIController::OnTargetPerceptionUpdated);
}

void AAIPerceptionAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Cast<ATopDownPlayerCharacter>(Actor))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Actor);
            BlackboardComponent->SetValueAsBool(TEXT("HasLineOfSight"), true);
			DrawDebugString(GetWorld(), GetPawn()->GetActorLocation() + FVector(0, 0, 100), TEXT("Player Detected"), nullptr, FColor::Green, 2.f, true);
        }
        else
        {
            BlackboardComponent->SetValueAsBool(TEXT("HasLineOfSight"), false);
			DrawDebugString(GetWorld(), GetPawn()->GetActorLocation() + FVector(0, 0, 100), TEXT("Player Lost"), nullptr, FColor::Red, 2.f, true);
        }
    }
}
