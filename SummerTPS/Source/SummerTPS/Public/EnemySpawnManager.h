#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnManager.generated.h"

class AEnemySpawner;

UCLASS()
class SUMMERTPS_API AEnemySpawnManager : public AActor
{
    GENERATED_BODY()

public: 
    AEnemySpawnManager();

protected:
    virtual void BeginPlay() override;

public: 
    // 이 매니저가 제어할 스포너들의 그룹 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    FName SpawnerGroupTag;

    // 게임 시작 시 자동으로 스폰을 시작할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bStartSpawningOnBeginPlay;

    // 스폰 시작까지의 딜레이 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnActivationDelay;

    // 지정된 태그를 가진 모든 스포너의 스폰을 시작시키는 함수
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StartAllSpawners();

private:
    // 레벨에서 찾은, 관리 대상이 되는 스포너들의 목록
    UPROPERTY()
    TArray<AEnemySpawner*> ManagedSpawners;

    // 태그를 이용해 관리할 스포너들을 찾는 내부 함수
    void FindSpawnersInWorld();
};