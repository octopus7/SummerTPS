#include "EnemySpawnManager.h"
#include "EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEnemySpawnManager::AEnemySpawnManager()
{
    PrimaryActorTick.bCanEverTick = false;

    bStartSpawningOnBeginPlay = false;
    SpawnActivationDelay = 0.0f;
}

void AEnemySpawnManager::BeginPlay()
{
    Super::BeginPlay();

    FindSpawnersInWorld();

    if (bStartSpawningOnBeginPlay)
    {
        if (SpawnActivationDelay > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemySpawnManager::StartAllSpawners, SpawnActivationDelay, false);
        }
        else
        {
            StartAllSpawners();
        }
    }
}

void AEnemySpawnManager::FindSpawnersInWorld()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AEnemySpawner::StaticClass(), SpawnerGroupTag, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AEnemySpawner* Spawner = Cast<AEnemySpawner>(Actor);
        if (Spawner)
        {
            ManagedSpawners.Add(Spawner);
        }
    }

    if (ManagedSpawners.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemySpawnManager '%s' found no spawners with tag '%s'."), *GetName(), *SpawnerGroupTag.ToString());
    }
}

void AEnemySpawnManager::StartAllSpawners()
{
    for (AEnemySpawner* Spawner : ManagedSpawners)
    {
        if (Spawner)
        {
            // 각 스포너의 bSpawnOnBeginPlay를 false로 설정하여 중복 실행 방지
            Spawner->bSpawnOnBeginPlay = false; 
            Spawner->StartSpawning();
        }
    }
}
