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
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetReceived);
    }
}

void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetPawn())
    {
        FVector TextLocation = GetPawn()->GetActorLocation() + FVector(0, 0, 100.f);
        if (bIsPlayerDetected)
        {
            DrawDebugString(GetWorld(), TextLocation, TEXT("Player Detected"), nullptr, FColor::Green, 0.f, true);
        }
        else
        {
            DrawDebugString(GetWorld(), TextLocation, TEXT("Searching..."), nullptr, FColor::Red, 0.f, true);
        }
    }
}

void AEnemyAIController::OnTargetReceived(AActor* Actor, FAIStimulus Stimulus)
{
    FVector TextLocation = GetPawn()->GetActorLocation() + FVector(0, 0, 100.f);

    // A much simpler and more reliable way to check if the perceived actor is the player.
    APawn* TargetPawn = Cast<APawn>(Actor);
    if (!TargetPawn)
    {
        // If the cast fails, it's not a pawn, so we are not interested.
        return;
    }

    if (TargetPawn->IsPlayerControlled())
    {
        bIsPlayerDetected = Stimulus.WasSuccessfullySensed();
    }
    else
    {
        // If the perceived actor is not the player, check if we just lost sight of the player.
        // This handles the case where the stimulus is for an actor other than the player.
        if (!Stimulus.WasSuccessfullySensed())
        {
            // To prevent flickering, you could add a timer here before setting bIsPlayerDetected to false.
            // For now, we'll just set it directly.
            bIsPlayerDetected = false;
        }
    }

    if (TargetPawn && TargetPawn->IsPlayerControlled())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Player Detected: Set Target in Blackboard
            GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Actor);
            DrawDebugString(GetWorld(), TextLocation, TEXT("Player Detected"), nullptr, FColor::Green, 0.f, true);  
        }
        else
        {
            // Player Lost: Clear Target in Blackboard
            GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
            DrawDebugString(GetWorld(), TextLocation, TEXT("Searching..."), nullptr, FColor::Green, 0.f, true);
        }
    }
    else DrawDebugString(GetWorld(), TextLocation, TEXT("Error"), nullptr, FColor::Green, 0.f, true);

}
