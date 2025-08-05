
#include "Tower.h"
#include "TowerAIController.h"
#include "Components/CapsuleComponent.h"

ATower::ATower()
{
    PrimaryActorTick.bCanEverTick = true;

    // We don't need the default capsule or skeletal mesh for a simple tower.
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetSkeletalMesh(nullptr);

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    RootComponent = TowerMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMeshAsset.Succeeded())
    {
        TowerMesh->SetStaticMesh(CylinderMeshAsset.Object);
        TowerMesh->SetRelativeScale3D(FVector(1.f, 1.f, 4.f)); // Make it taller like a tower
    }

    // Set the AI Controller
    AIControllerClass = ATowerAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ATower::BeginPlay()
{
    Super::BeginPlay();
}

void ATower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATower::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
