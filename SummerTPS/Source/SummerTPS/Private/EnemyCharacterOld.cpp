#include "EnemyCharacterOld.h"
#include "HealthComponent.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacterOld::AEnemyCharacterOld()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    bIsDead = false;
}

void AEnemyCharacterOld::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacterOld::OnHealthChanged);
    }

    if (DefaultWeaponClass)
    {
        CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket")); // Assuming a "WeaponSocket" exists on the skeleton
            CurrentWeapon->SetOwner(this);
        }
    }

    // AI/BehaviorTree/Blackboard 제거됨
}

// Perception/Blackboard 관련 코드 제거됨

void AEnemyCharacterOld::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath();
    }
}



void AEnemyCharacterOld::OnDeath_Implementation()
{
    // Stop AI logic here
    GetCharacterMovement()->StopMovementImmediately();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll or play death animation
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    SetLifeSpan(5.0f); // Actor will be destroyed after 5 seconds
}


void AEnemyCharacterOld::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnemyCharacterOld::Attack()
{
    if (CurrentWeapon && !bIsDead)
    {
        CurrentWeapon->Fire();
    }
}
