#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h" // Required for FAIStimulus
#include "EnemyAIController.generated.h"

UCLASS()
class SUMMERTPS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBlackboardData* BlackboardData;

protected:
    virtual void OnPossess(APawn* InPawn) override;

    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    class UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void OnTargetReceived(AActor* Actor, FAIStimulus Stimulus);

    bool bIsPlayerDetected;
};
