#include "SandCastleActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

ASandCastleActor::ASandCastleActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(MeshComponent);
    MeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    MaxDurability = 900.f;
    Durability = MaxDurability;
    MidDamagedThresholdRatio = 0.66f;
    NearDestroyedThresholdRatio = 0.33f;
    CurrentState = ESandCastleState::Intact;
    bDestroyed = false;

    // HP text above head
    HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
    HPText->SetupAttachment(MeshComponent);
    HPText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    HPText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    HPText->SetWorldSize(108.f);
    HPText->SetTextRenderColor(FColor::White);
    HPText->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    UpdateHPText();
}

void ASandCastleActor::BeginPlay()
{
    Super::BeginPlay();

    Durability = MaxDurability;

    Durability = FMath::Clamp(Durability, 0.f, MaxDurability);
    EvaluateAndApplyState(false);

    OnTakeAnyDamage.AddDynamic(this, &ASandCastleActor::HandleTakeAnyDamage);
}

void ASandCastleActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // Ensure initial mesh matches current state in editor
    EvaluateAndApplyState(false);
}

void ASandCastleActor::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (bDestroyed || Damage <= 0.f)
    {
        return;
    }

    Durability = FMath::Clamp(Durability - Damage, 0.f, MaxDurability);

    UpdateHPText();

    if (Durability <= 0.f)
    {
        // Final destroy: spawn effect not attached, then destroy actor
        SpawnDestroyEffect();
        bDestroyed = true;
        Destroy();
        return;
    }

    // Evaluate new state and apply mesh; spawn stage effect if state changed
    EvaluateAndApplyState(true);
}

void ASandCastleActor::EvaluateAndApplyState(bool bSpawnStageEffect)
{
    ESandCastleState NewState = ESandCastleState::Intact;
    const float Ratio = (MaxDurability > 0.f) ? (Durability / MaxDurability) : 0.f;

    if (Ratio <= NearDestroyedThresholdRatio)
    {
        NewState = ESandCastleState::NearDestroyed;
    }
    else if (Ratio <= MidDamagedThresholdRatio)
    {
        NewState = ESandCastleState::MidDamaged;
    }
    else
    {
        NewState = ESandCastleState::Intact;
    }

    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        ApplyMeshForState(CurrentState);
        if (bSpawnStageEffect)
        {
            SpawnStageEffect();
        }
    }
    else
    {
        // Keep mesh consistent even if same state (e.g., edited in details)
        ApplyMeshForState(CurrentState);
    }
}

void ASandCastleActor::ApplyMeshForState(ESandCastleState NewState)
{
    if (!MeshComponent)
    {
        return;
    }

    UStaticMesh* DesiredMesh = nullptr;
    switch (NewState)
    {
    case ESandCastleState::Intact:        DesiredMesh = MeshIntact; break;
    case ESandCastleState::MidDamaged:    DesiredMesh = MeshMidDamaged; break;
    case ESandCastleState::NearDestroyed: DesiredMesh = MeshNearDestroyed; break;
    default: break;
    }

    if (DesiredMesh)
    {
        MeshComponent->SetStaticMesh(DesiredMesh);
    }
}

void ASandCastleActor::SpawnStageEffect()
{
    if (!StageChangeEffect)
    {
        return;
    }
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), StageChangeEffect, GetActorLocation(), GetActorRotation());
}

void ASandCastleActor::SpawnDestroyEffect()
{
    if (!DestroyEffect)
    {
        return;
    }
    // Not attached to this actor to ensure it persists after this actor is destroyed
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestroyEffect, GetActorLocation(), GetActorRotation());
}

void ASandCastleActor::UpdateHPText()
{
    if (!HPText)
    {
        return;
    }
    const FString Txt = FString::Printf(TEXT("%.0f / %.0f"), Durability, MaxDurability);
    HPText->SetText(FText::FromString(Txt));
}