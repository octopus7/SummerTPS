#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class SUMMERTPS_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/************************************************************************
	* Camera Control
	************************************************************************/

	/** Default distance of the camera from the player */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultCameraArmLength;

	/** Distance of the camera from the player when aiming */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float AimingCameraArmLength;

	/** Default socket offset for the camera boom */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector DefaultCameraSocketOffset;

	/** Socket offset for the camera boom when aiming */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector AimingCameraSocketOffset;

	/** Distance of the camera from the player when sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float SprintCameraArmLength;

	/** Socket offset for the camera boom when sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector SprintCameraSocketOffset;

	/** Speed at which the camera interpolates to its new position */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraInterpSpeed;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/** Cover Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CoverAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for aiming input (start) */
	void AimStarted();

	/** Called for aiming input (stop) */
	void AimStopped();

	/** Called for firing input (start) */
	void StartFire();

	/** Called for firing input (stop) */
	void StopFire();

	/** Called for cover input */
	void Cover();

	/** Called for sprint input (start) */
	void SprintStarted();

	/** Called for sprint input (stop) */
	void SprintStopped();

	/** Fires a projectile */
	void Fire();

	/** Projectile spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ProjectileSpawnPoint;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class AActor> ProjectileClass; // Using AActor for now, can be changed to a specific projectile class later

	/** Speed used for the projectile trajectory prediction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float ProjectilePredictionSpeed;

	/** Default walk speed of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultWalkSpeed;

	/** Sprint walk speed of the character */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintWalkSpeed;

	/** Time between shots for automatic fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TimeBetweenShots;

	/************************************************************************
	* Weapon Handling
	************************************************************************/

	/** Blueprint of the weapon to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> WeaponBlueprint;

	/** Socket name on the mesh to attach the weapon to */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName;

	/** A reference to the spawned weapon */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	AActor* SpawnedWeapon;

protected:
	/************************************************************************
	* Cover System
	************************************************************************/

	/** Tries to find cover and enter it */
	void TryEnterCover();

	/** Exits from the current cover */
	void ExitCover();

	/** True if player is in cover. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover")
	bool bIsCovered;

	/** The normal of the cover surface */
	FVector CoverWallNormal;

	/************************************************************************
	* Enter Cover Animation
	************************************************************************/
	/** True if player is currently entering cover with animation. */
	bool bIsEnteringCover;

	/** Time when the enter cover animation started. */
	float EnterCoverStartTime;

	/** Duration of the enter cover animation. */
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	float EnterCoverDuration;

	/** Player's location when enter cover animation started. */
	FVector EnterCoverStartLocation;

	/** Target location for the player after entering cover animation. */
	FVector EnterCoverTargetLocation;

	/** Player's rotation when enter cover animation started. */
	FRotator EnterCoverStartRotation;

	/** Target rotation for the player after entering cover animation. */
	FRotator EnterCoverTargetRotation;

	/************************************************************************
	* Exit Cover Animation
	************************************************************************/
	/** True if player is currently exiting cover with animation. */
	bool bIsExitingCover;

	/** Time when the exit cover animation started. */
	float ExitCoverStartTime;

	/** Duration of the exit cover animation. */
	UPROPERTY(EditDefaultsOnly, Category = "Cover")
	float ExitCoverDuration;

	/** Player's location when exit cover animation started. */
	FVector ExitCoverStartLocation;

	/** Target location for the player after exiting cover animation. */
	FVector ExitCoverTargetLocation;

private:
	/** Timer handle for automatic firing */
	FTimerHandle TimerHandle_AutomaticFire;

	/** Flag to track if the dedicated aim button is pressed */
	bool bIsAiming;

	/** Flag to track if the player is sprinting */
	bool bIsSprinting;

	/** Updates the character's rotation settings based on aiming and firing states */
	void UpdateRotationSettings();
};
