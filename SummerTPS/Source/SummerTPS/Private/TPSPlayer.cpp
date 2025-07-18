#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize projectile prediction speed
	ProjectilePredictionSpeed = 3000.f;

	// Initialize automatic fire rate
	TimeBetweenShots = 0.1f;

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
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	}
}

void ATPSPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
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
	// Implement aiming logic here
	// For example, change camera FOV, adjust character rotation, etc.
	 GetCharacterMovement()->bOrientRotationToMovement = false; // Stop orienting to movement
	 bUseControllerRotationYaw = true; // Allow controller to rotate character yaw
	UE_LOG(LogTemp, Warning, TEXT("Aiming Started!"));
}

void ATPSPlayer::AimStopped()
{
	// Implement aiming stop logic here
	 GetCharacterMovement()->bOrientRotationToMovement = true; // Resume orienting to movement
	 bUseControllerRotationYaw = false; // Stop controller from rotating character yaw
	UE_LOG(LogTemp, Warning, TEXT("Aiming Stopped!"));
}

void ATPSPlayer::StartFire()
{
	Fire(); // Fire immediately on press
	GetWorldTimerManager().SetTimer(TimerHandle_AutomaticFire, this, &ATPSPlayer::Fire, TimeBetweenShots, true);
}

void ATPSPlayer::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_AutomaticFire);
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
