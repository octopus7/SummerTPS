#include "Well.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWell::AWell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMeshAsset.Object);
	}

	DistanceText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DistanceText"));
	DistanceText->SetupAttachment(RootComponent);
	DistanceText->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	DistanceText->SetHorizontalAlignment(EHTA_Center);
	DistanceText->SetWorldSize(100.f);
	DistanceText->SetTextRenderColor(FColor::Cyan);

	DebugTextTimer = 0.f;
}

// Called when the game starts or when spawned
void AWell::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DebugTextTimer += DeltaTime;

    if (DebugTextTimer <= 0.5f)
    {
        DrawDebugString(GetWorld(), GetActorLocation(), TEXT("Ready"), nullptr, FColor::Yellow, 0.f, true);
    }

    if(DebugTextTimer >= 1.f)
    {
        DebugTextTimer = 0.f;
    }

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		float Distance = GetDistanceTo(PlayerPawn);
		float DistanceInMeters = Distance / 100.0f;
		FString DistanceString = FString::Printf(TEXT("%.1fm"), DistanceInMeters);
		DistanceText->SetText(FText::FromString(DistanceString));

		// Make the text face the player, but keep it upright
		FVector PlayerLocation = PlayerPawn->GetActorLocation();
		FVector TextLocation = DistanceText->GetComponentLocation();
		FRotator LookAtRotation = (PlayerLocation - TextLocation).Rotation();
		
		// By creating a new rotator with only the Yaw, we ensure it only rotates horizontally
        // and stays upright (no pitch or roll).
		FRotator UprightRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);
		DistanceText->SetWorldRotation(UprightRotation);
	}
}
