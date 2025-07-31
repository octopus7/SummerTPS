#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyAIState.h"
#include "EnemyCharacter.generated.h"

class UHealthComponent;
class AWeapon;
class UAIPerceptionComponent;

UCLASS()
class SUMMERTPS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComponent;

	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, Category = "AI")
	void OnDeath();
	virtual void OnDeath_Implementation();

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void Attack();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyAIState AIState;

private:
	UPROPERTY()
	AWeapon* CurrentWeapon;

	bool bIsDead;
};
