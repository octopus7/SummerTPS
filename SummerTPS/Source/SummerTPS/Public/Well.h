#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Well.generated.h"

UCLASS()
class SUMMERTPS_API AWell : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWell();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Text")
	class UTextRenderComponent* DistanceText;

private:
	float DebugTextTimer;

};
