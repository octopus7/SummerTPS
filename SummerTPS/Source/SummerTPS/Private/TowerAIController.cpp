
#include "TowerAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ATowerAIController::ATowerAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.f;
        SightConfig->LoseSightRadius = 2500.f;
        SightConfig->PeripheralVisionAngleDegrees = 360.0f; // Tower can see in all directions
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void ATowerAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ATowerAIController::OnTargetPerceived);
    }
}

void ATowerAIController::Tick(float DeltaTime)
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

void ATowerAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
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
}
