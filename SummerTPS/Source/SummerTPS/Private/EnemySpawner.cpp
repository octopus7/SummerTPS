#include "EnemySpawner.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "EnemyCharacter.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnVolume = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;
    SpawnVolume->InitSphereRadius(500.0f); // Default spawn radius

    NumberOfEnemiesToSpawn = 5;
    SpawnInterval = 2.0f;
    bSpawnOnBeginPlay = true;
    bRandomizeSpawnRotation = true;
    EnemiesSpawnedCount = 0;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnOnBeginPlay)
    {
        StartSpawning();
    }
}

void AEnemySpawner::StartSpawning()
{
    if (!EnemyClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyClass not set in %s"), *GetName());
        return;
    }

    EnemiesSpawnedCount = 0;
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnInterval, true, 0.0f);
}

void AEnemySpawner::SpawnEnemy()
{
    if (EnemiesSpawnedCount >= NumberOfEnemiesToSpawn)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    UWorld* const World = GetWorld();
    if (World && EnemyClass)
    {
        FVector SpawnOrigin = SpawnVolume->GetComponentLocation();
        float SpawnRadius = SpawnVolume->GetScaledSphereRadius();
        FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(SpawnOrigin, FVector(SpawnRadius));

        FHitResult HitResult;
        FVector StartLocation = FVector(RandomPoint.X, RandomPoint.Y, SpawnOrigin.Z + SpawnRadius); // Start trace from high up
        FVector EndLocation = FVector(RandomPoint.X, RandomPoint.Y, SpawnOrigin.Z - SpawnRadius * 2); // And trace down

        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);

        FVector SpawnLocation = RandomPoint; // Default to random point if no ground is found

        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
        {
            if (HitResult.bBlockingHit)
            {
                SpawnLocation = HitResult.ImpactPoint;
            }
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FRotator SpawnRotation = FRotator::ZeroRotator;
        if (bRandomizeSpawnRotation)
        {
            SpawnRotation.Yaw = FMath::FRand() * 360.0f;
        }

        AEnemyCharacter* SpawnedEnemy = World->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedEnemy)
        {
            EnemiesSpawnedCount++;
        }
    }
}
