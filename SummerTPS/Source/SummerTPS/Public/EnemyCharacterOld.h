#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacterOld.generated.h"

class UHealthComponent;
class AWeapon;

UCLASS(Blueprintable)
class SUMMERTPS_API AEnemyCharacterOld : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacterOld();

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

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Attack();

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AWeapon> DefaultWeaponClass;

private:
    UPROPERTY()
    AWeapon* CurrentWeapon;

    bool bIsDead;

protected:
};
