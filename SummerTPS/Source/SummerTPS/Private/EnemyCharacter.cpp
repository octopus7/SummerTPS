#include "EnemyCharacter.h"
#include "HealthComponent.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    bIsDead = false;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnHealthChanged);

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
        OnDeath();
    }
}

void AEnemyCharacter::OnDeath_Implementation()
{
    // Stop AI logic here
    GetCharacterMovement()->StopMovementImmediately();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll or play death animation
    GetMesh()->SetSimulatePhysics(true);

    SetLifeSpan(5.0f); // Actor will be destroyed after 5 seconds
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
