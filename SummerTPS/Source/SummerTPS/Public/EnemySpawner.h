#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyCharacter;
class USphereComponent;

UCLASS()
class SUMMERTPS_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public: 
    AEnemySpawner();

protected:
    virtual void BeginPlay() override;

public: 
    // 스폰할 적 캐릭터의 종류 (블루프린트에서 선택)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
    TSubclassOf<AEnemyCharacter> EnemyClass;

    // 적이 스폰될 범위를 나타내는 볼륨
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
    USphereComponent* SpawnVolume;

    // 한 번에 스폰할 적의 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 NumberOfEnemiesToSpawn;

    // 각 적이 스폰될 때의 시간 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnInterval;

    // 게임 시작 시 자동으로 스폰을 시작할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bSpawnOnBeginPlay;

    // 스폰 시 적의 방향을 랜덤으로 설정할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    bool bRandomizeSpawnRotation;

    // 스폰 프로세스를 시작하는 함수 (블루프린트나 다른 코드에서 호출 가능)
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StartSpawning();

private:
    // 실제로 적 한 명을 스폰하는 내부 함수
    void SpawnEnemy();

    // 시간차를 두고 스폰을 관리하기 위한 타이머 핸들
    FTimerHandle SpawnTimerHandle;

    // 현재까지 스폰된 적의 수를 추적하는 카운터
    int32 EnemiesSpawnedCount;
};