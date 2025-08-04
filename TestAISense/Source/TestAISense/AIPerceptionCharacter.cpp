// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPerceptionCharacter.h"
#include "AIPerceptionAIController.h"

AAIPerceptionCharacter::AAIPerceptionCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIPerceptionAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAIPerceptionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAIPerceptionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAIPerceptionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
