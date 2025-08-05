#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h" // For Cast<ACharacter>

AEnemyAIController::AEnemyAIController()
{
    // Create Perception Component and Sight Config
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.f;
        SightConfig->LoseSightRadius = 2000.f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f; // Standard forward vision
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // Usually false for enemies
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false; // Usually false for enemies

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Start Behavior Tree and bind perception update
    if (BehaviorTree && BlackboardData)
    {
        UBlackboardComponent* BBComp = GetBlackboardComponent();
        if (BBComp)
        {
            BBComp->InitializeBlackboard(*BlackboardData);
        }
        RunBehaviorTree(BehaviorTree);
    }

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceived);
    }
}

void AEnemyAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* TargetPawn = Cast<APawn>(Actor);
    if (TargetPawn && TargetPawn->IsPlayerControlled())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Player Detected: Set Target in Blackboard
            GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Actor);
        }
        else
        {
            // Player Lost: Clear Target in Blackboard
            GetBlackboardComponent()->ClearValue(TEXT("Target"));
        }
    }
}
