#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize projectile prediction speed
	ProjectilePredictionSpeed = 3000.f;

	// Initialize automatic fire rate
	TimeBetweenShots = 0.1f;

	// Initialize aiming flag
	bIsAiming = false;

	// Initialize sprinting flag
	bIsSprinting = false;

	// --- Camera Control Defaults ---
	DefaultCameraArmLength = 400.0f;
	AimingCameraArmLength = 80.0f;
	SprintCameraArmLength = 200.0f; // Example value, adjust as needed
	DefaultCameraSocketOffset = FVector(0.f, 50.f, 70.f);
	AimingCameraSocketOffset = FVector(0.f, 70.f, 60.f);
	SprintCameraSocketOffset = FVector(0.f, 60.f, 40.f); // Example value, adjust as needed
	CameraInterpSpeed = 20.0f;

	// Initialize cover flag
	bIsCovered = false;

	// Initialize exit cover animation variables
	bIsExitingCover = false;
	ExitCoverDuration = 0.1f;

	// Initialize enter cover animation variables
	bIsEnteringCover = false;
	EnterCoverDuration = 0.1f;

	// Initialize movement speeds
	DefaultWalkSpeed = 500.f;
	SprintWalkSpeed = 800.f;

	// Initialize weapon socket name
	WeaponSocketName = FName("Weapon");

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Configure character movement for jumping/falling
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	// Create a camera boom (pulls in towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a projectile spawn point
	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(GetMesh()); // Attach to mesh, can be adjusted to a specific socket later
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Set initial camera boom properties
	CameraBoom->TargetArmLength = DefaultCameraArmLength;
	CameraBoom->SocketOffset = DefaultCameraSocketOffset;
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Spawn and attach the weapon
	if (WeaponBlueprint && GetMesh() && GetMesh()->DoesSocketExist(WeaponSocketName))
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			SpawnedWeapon = World->SpawnActor<AActor>(WeaponBlueprint, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (SpawnedWeapon)
			{
				FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
				SpawnedWeapon->AttachToComponent(GetMesh(), AttachmentRules, WeaponSocketName);
			}
		}
	}
	else
	{
		if (!WeaponBlueprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponBlueprint is not set in TPSPlayer."));
		}
		if (!GetMesh() || !GetMesh()->DoesSocketExist(WeaponSocketName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Socket '%s' does not exist on the player mesh."), *WeaponSocketName.ToString());
		}
	}
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- Camera Interpolation ---
	float TargetArmLength;
	FVector TargetSocketOffset;

	if (bIsSprinting)
	{
		TargetArmLength = SprintCameraArmLength;
		TargetSocketOffset = SprintCameraSocketOffset;
	}
	else if (bIsAiming)
	{
		TargetArmLength = AimingCameraArmLength;
		TargetSocketOffset = AimingCameraSocketOffset;
	}
	else
	{
		TargetArmLength = DefaultCameraArmLength;
		TargetSocketOffset = DefaultCameraSocketOffset;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraInterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, CameraInterpSpeed);

	if (bIsCovered)
	{
		// If currently entering cover, interpolate position
		if (bIsEnteringCover)
		{
			float ElapsedTime = GetWorld()->GetTimeSeconds() - EnterCoverStartTime;
			float Alpha = FMath::Clamp(ElapsedTime / EnterCoverDuration, 0.f, 1.f);
			FVector CurrentLocation = FMath::Lerp(EnterCoverStartLocation, EnterCoverTargetLocation, Alpha);
			FRotator CurrentRotation = FMath::Lerp(EnterCoverStartRotation, EnterCoverTargetRotation, Alpha);
			SetActorLocationAndRotation(CurrentLocation, CurrentRotation);

			if (Alpha >= 1.f)
			{
				bIsEnteringCover = false;
			}
			DrawDebugString(GetWorld(), FVector(0, 0, 100), "Entering Cover", this, FColor::Cyan, 0.f);
		}
		else
		{
			// Keep the player facing away from the cover wall
			SetActorRotation((-CoverWallNormal).Rotation());
			DrawDebugString(GetWorld(), FVector(0, 0, 100), "Covered", this, FColor::Green, 0.f);
		}

		// Draw an arrow indicating the cover normal
		FVector ArrowStart = GetActorLocation() + FVector(0, 0, 50.f); // Chest height
		FVector ArrowEnd = ArrowStart + CoverWallNormal * 50.f;
		DrawDebugDirectionalArrow(GetWorld(), ArrowStart, ArrowEnd, 2.5f, FColor::Blue, false, 0.f, 0, 1.0f);
	}
	else if (bIsExitingCover)
	{
		float ElapsedTime = GetWorld()->GetTimeSeconds() - ExitCoverStartTime;
		float Alpha = FMath::Clamp(ElapsedTime / ExitCoverDuration, 0.f, 1.f);
		FVector CurrentLocation = FMath::Lerp(ExitCoverStartLocation, ExitCoverTargetLocation, Alpha);
		SetActorLocation(CurrentLocation);

		if (Alpha >= 1.f)
		{
			bIsExitingCover = false;
		}
		DrawDebugString(GetWorld(), FVector(0, 0, 100), "Exiting Cover", this, FColor::Yellow, 0.f);
	}
	else
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 100), "Not Covered", this, FColor::Red, 0.f);
	}

	if (ProjectileClass && ProjectileSpawnPoint)
	{
		FPredictProjectilePathParams PredictParams(20.f, ProjectileSpawnPoint->GetComponentLocation(), GetActorForwardVector() * ProjectilePredictionSpeed, 5.f, ECC_Visibility);
		FPredictProjectilePathResult PredictResult;

		if (UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult))
		{
			for (int32 i = 0; i < PredictResult.PathData.Num() - 1; ++i)
			{
				DrawDebugLine(GetWorld(), PredictResult.PathData[i].Location, PredictResult.PathData[i+1].Location, FColor::Yellow, false, 0.f, 0, 0.5f);
			}
		}
	}
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSPlayer::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSPlayer::Look);

		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPSPlayer::AimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPSPlayer::AimStopped);

		//Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATPSPlayer::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATPSPlayer::StopFire);

		//Cover
		EnhancedInputComponent->BindAction(CoverAction, ETriggerEvent::Started, this, &ATPSPlayer::Cover);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATPSPlayer::SprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSPlayer::SprintStopped);
	}
}

void ATPSPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (bIsCovered)
		{
			FVector MoveDirection = FVector::CrossProduct(CoverWallNormal, FVector::UpVector) * MovementVector.X;

			// Check if there is a wall in the direction of movement
			FHitResult HitResult;
			FVector Start = GetActorLocation() + MoveDirection * 50.f; // Check slightly in front of the player
			FVector End = Start - CoverWallNormal * 100.f; // Check towards the wall
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
			{
				// Only move if there is a wall to stay in cover
				AddMovementInput(MoveDirection, 1.f);
			}
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void ATPSPlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPSPlayer::AimStarted()
{
	bIsAiming = true;
	UpdateRotationSettings();
}

void ATPSPlayer::AimStopped()
{
	bIsAiming = false;
	UpdateRotationSettings();
}

void ATPSPlayer::StartFire()
{
	UpdateRotationSettings();
	Fire(); // Fire immediately on press
	GetWorldTimerManager().SetTimer(TimerHandle_AutomaticFire, this, &ATPSPlayer::Fire, TimeBetweenShots, true);
}

void ATPSPlayer::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_AutomaticFire);
	UpdateRotationSettings();
}

void ATPSPlayer::UpdateRotationSettings()
{
	bool bShouldOrientToMovement = true;
	bool bShouldUseControllerRotationYaw = false;

	// If we are aiming OR firing, we want to face the camera direction.
	if (bIsAiming || GetWorldTimerManager().IsTimerActive(TimerHandle_AutomaticFire))
	{
		bShouldOrientToMovement = false;
		bShouldUseControllerRotationYaw = true;
	}

	GetCharacterMovement()->bOrientRotationToMovement = bShouldOrientToMovement;
	bUseControllerRotationYaw = bShouldUseControllerRotationYaw;
}

void ATPSPlayer::Fire()
{
	// Implement projectile firing logic here
	if (ProjectileClass && ProjectileSpawnPoint)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			// Get the forward vector of the character for firing direction
			FVector CharacterForward = GetActorForwardVector();
			FRotator SpawnRotation = CharacterForward.Rotation();
			FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();

			// Spawn the projectile
			AActor* SpawnedProjectile = World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedProjectile)
			{
				UE_LOG(LogTemp, Warning, TEXT("Projectile Fired!"));

				// Update the prediction speed from the spawned projectile
				UProjectileMovementComponent* ProjectileMovement = SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
				if (ProjectileMovement)
				{
					ProjectilePredictionSpeed = ProjectileMovement->InitialSpeed;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileClass or ProjectileSpawnPoint not set!"));
	}
}

void ATPSPlayer::Cover()
{
	if (bIsCovered)
	{
		ExitCover();
	}
	else
	{
		TryEnterCover();
	}
}

void ATPSPlayer::SprintStarted()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintWalkSpeed;
}

void ATPSPlayer::SprintStopped()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void ATPSPlayer::TryEnterCover()
{
	UE_LOG(LogTemp, Warning, TEXT("TryEnterCover!"));

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 100.f;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// For now, we'll use the Visibility channel. We'll create a custom "Cover" channel later.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// If already exiting cover, stop it
		if (bIsExitingCover)
		{
			bIsExitingCover = false;
		}

		bIsCovered = true;
		CoverWallNormal = HitResult.ImpactNormal;

		// --- Snap to cover ---
		// Calculate the new location to snap to the wall
		const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector TargetLocation = HitResult.ImpactPoint + CoverWallNormal * (CapsuleRadius + 2.0f); // Add a small buffer to avoid clipping
		TargetLocation.Z = GetActorLocation().Z; // Keep the current Z location to prevent snapping up/down

		// Set up enter cover animation
		bIsEnteringCover = true;
		EnterCoverStartTime = GetWorld()->GetTimeSeconds();
		EnterCoverStartLocation = GetActorLocation();
		EnterCoverTargetLocation = TargetLocation;
		EnterCoverStartRotation = GetActorRotation();
		EnterCoverTargetRotation = (-CoverWallNormal).Rotation();

		// Stop movement and set initial rotation
		GetCharacterMovement()->StopMovementImmediately();
		// SetActorLocationAndRotation(NewLocation, (-CoverWallNormal).Rotation()); // This will be handled by interpolation

		// Disable rotation to movement and enable controller rotation yaw
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

		UE_LOG(LogTemp, Warning, TEXT("Entered Cover!"));
	}
}

void ATPSPlayer::ExitCover()
{
	bIsCovered = false;

	// Set up exit cover animation
	bIsExitingCover = true;
	ExitCoverStartTime = GetWorld()->GetTimeSeconds();
	ExitCoverStartLocation = GetActorLocation();
	ExitCoverTargetLocation = GetActorLocation() + CoverWallNormal * 10.f;

	// Restore normal movement and rotation settings
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	UE_LOG(LogTemp, Warning, TEXT("Exited Cover!"));
}