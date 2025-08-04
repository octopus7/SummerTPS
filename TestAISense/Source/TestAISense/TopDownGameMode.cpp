// Fill out your copyright notice in the Description page of Project Settings.

#include "TopDownGameMode.h"
#include "TopDownPlayerCharacter.h"
#include "TopDownPlayerController.h"

ATopDownGameMode::ATopDownGameMode()
{
    DefaultPawnClass = ATopDownPlayerCharacter::StaticClass();
    PlayerControllerClass = ATopDownPlayerController::StaticClass();
}
