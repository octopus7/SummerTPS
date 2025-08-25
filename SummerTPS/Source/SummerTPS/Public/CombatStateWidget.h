#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatStateWidget.generated.h"

UCLASS()
class SUMMERTPS_API UCombatStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateStateText(FText InText);

private:
	UPROPERTY()
	class UTextBlock* StateTextBlock;
};

