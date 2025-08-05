
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Tower.generated.h"

UCLASS()
class SUMMERTPS_API ATower : public ACharacter
{
    GENERATED_BODY()

public:
    ATower();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* TowerMesh;
};
