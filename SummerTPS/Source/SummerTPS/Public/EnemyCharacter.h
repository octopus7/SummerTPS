#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h" 
#include "Perception/AISenseConfig_Sight.h"   
#include "EnemyAIController.h"               
#include "EnemyCharacter.generated.h"

class UHealthComponent;
class AWeapon;
class UBehaviorTree; 
class UBlackboardData; 

UCLASS(Blueprintable, meta = (AIControllerClass = "AEnemyAIController")) 
class SUMMERTPS_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

    UFUNCTION()
    void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "AI")
    void OnDeath();
    virtual void OnDeath_Implementation();

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void Attack();

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AWeapon> DefaultWeaponClass;

private:
    UPROPERTY()
    AWeapon* CurrentWeapon;

    bool bIsDead;

protected: 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* BlackboardData;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SightRadius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SightDetectionByAffiliation = 0.0f;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};