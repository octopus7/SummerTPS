// Simple teleport portal actor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UBoxComponent;
class UTextRenderComponent;

UCLASS()
class SUMMERTPS_API APortal : public AActor
{
	GENERATED_BODY()

public:
    APortal();

protected:
	virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;

    virtual void OnConstruction(const FTransform& Transform) override;

protected:
	/** Trigger volume used to detect entering actors */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
    UBoxComponent* TriggerBox;

    /** Text label shown above the portal displaying "PortalId -> TargetPortalId" */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
    UTextRenderComponent* TextLabel;

	/** Unique identifier for this portal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FString PortalId;

	/** Id of the portal to teleport to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FString TargetPortalId;

	/** Offset applied at destination relative to target portal's forward vector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float DestinationForwardOffset;

	/** Optional vertical offset when arriving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float DestinationZOffset;

	/** Draw debug lines for the trigger box */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDraw;

	/** Thickness of debug lines */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLineThickness;

    /** Height above actor origin for the label */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelHeight;

    /** Text world size for the label */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelWorldSize;

    /** Rotate label to face the camera every tick */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bFaceCamera;

    /** If true, only rotate around Z (yaw) to keep text upright */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bFaceCameraYawOnly;

    /** Smoothly interpolate label rotation toward camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bSmoothFaceCamera;

    /** Interp speed for smooth facing (deg/sec-ish). Higher = snappier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float FaceCameraInterpSpeed;

    /** Enable distance-based label visibility control */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bUseDistanceVisibility;

    /** Label visible when camera distance is within [Min, Max] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelVisibleMinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LabelVisibleMaxDistance;

private:
	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

    APortal* FindDestinationPortal() const;

    void UpdateLabelText();
};
