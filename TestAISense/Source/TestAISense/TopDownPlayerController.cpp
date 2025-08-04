// Fill out your copyright notice in the Description page of Project Settings.

#include "TopDownPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "TopDownPlayerCharacter.h"

void ATopDownPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        ATopDownPlayerCharacter* PlayerCharacter = Cast<ATopDownPlayerCharacter>(GetPawn());
        if (PlayerCharacter && PlayerCharacter->DefaultMappingContext)
        {
            Subsystem->AddMappingContext(PlayerCharacter->DefaultMappingContext, 0);
        }
    }
}
