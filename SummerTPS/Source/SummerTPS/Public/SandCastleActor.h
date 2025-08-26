#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "SandCastleActor.generated.h"

class UNiagaraSystem;
class UUserWidget;
class UWidgetComponent;

UENUM(BlueprintType)
enum class ESandCastleState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    MidDamaged      UMETA(DisplayName = "MidDamaged"),
    NearDestroyed   UMETA(DisplayName = "NearDestroyed"),
};

UCLASS()
class SUMMERTPS_API ASandCastleActor : public AActor
{
    GENERATED_BODY()

public:
    ASandCastleActor();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* HPText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshIntact;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshMidDamaged;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Mesh")
    class UStaticMesh* MeshNearDestroyed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Destructible|Durability")
    float MaxDurability;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Durability")
    float Durability;

    // Remaining ratio thresholds (Durability/MaxDurability)
    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Durability", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MidDamagedThresholdRatio;      // e.g. 0.66 -> switch to mid-damaged when remaining <= 66%

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|Durability", meta=(ClampMin="0.0", ClampMax="1.0"))
    float NearDestroyedThresholdRatio;   // e.g. 0.33 -> switch to near-destroyed when remaining <= 33%

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX")
    UNiagaraSystem* StageChangeEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Destructible|FX")
    UNiagaraSystem* DestroyEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|State")
    ESandCastleState CurrentState;

    bool bDestroyed;

protected:
    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void EvaluateAndApplyState(bool bSpawnStageEffect);
    void ApplyMeshForState(ESandCastleState NewState);
    void SpawnStageEffect();
    void SpawnDestroyEffect();
    void UpdateHPText();
};
