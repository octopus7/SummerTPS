// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIPerceptionCharacter.generated.h"

UCLASS()
class TESTAISENSE_API AAIPerceptionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAIPerceptionCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
