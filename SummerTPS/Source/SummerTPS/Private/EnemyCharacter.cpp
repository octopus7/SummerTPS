#include "EnemyCharacter.h"
#include "HealthComponent.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "BrainComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    bIsDead = false;
    AIState = EEnemyAIState::EAS_Idle;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnHealthChanged);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::OnTargetPerceptionUpdated);

    if (DefaultWeaponClass)
    {
        CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket")); // Assuming a "WeaponSocket" exists on the skeleton
            CurrentWeapon->SetOwner(this);
        }
    }
}

void AEnemyCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        AIState = EEnemyAIState::EAS_Dead;
        OnDeath();
    }
}

void AEnemyCharacter::OnDeath_Implementation()
{
    // Stop AI logic here
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->GetBrainComponent()->StopLogic("Dead");
    }

    GetCharacterMovement()->StopMovementImmediately();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll or play death animation
    GetMesh()->SetSimulatePhysics(true);

    SetLifeSpan(5.0f); // Actor will be destroyed after 5 seconds
}

void AEnemyCharacter::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
        if (BlackboardComponent)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                BlackboardComponent->SetValueAsObject("TargetPlayer", Actor);
                AIState = EEnemyAIState::EAS_Chase;
            }
            else
            {
                BlackboardComponent->ClearValue("TargetPlayer");
                AIState = EEnemyAIState::EAS_Patrol;
            }
        }
    }
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnemyCharacter::Attack()
{
    if (CurrentWeapon && !bIsDead)
    {
        CurrentWeapon->Fire();
    }
}
