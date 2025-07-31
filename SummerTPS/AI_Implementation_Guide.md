
# 적 AI 구현 가이드

이 문서는 언리얼 엔진에서 비헤이비어 트리와 C++ 코드를 사용하여 기본적인 적 AI 행동(대기, 순찰, 추적, 공격)을 구현하는 방법을 안내합니다.

## 1. C++ 코드 수정

### 1.1. AI 상태 Enum 생성

AI의 상태를 관리하기 위한 Enum을 정의합니다.

**경로:** `Source/SummerTPS/Public/EnemyAIState.h`
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "EnemyAIState.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    EAS_Idle      UMETA(DisplayName = "Idle"),
    EAS_Patrol    UMETA(DisplayName = "Patrol"),
    EAS_Chase     UMETA(DisplayName = "Chase"),
    EAS_Attack    UMETA(DisplayName = "Attack"),
    EAS_Dead      UMETA(DisplayName = "Dead")
};
```

### 1.2. EnemyCharacter 헤더 파일 수정

`EnemyCharacter.h`에 AI Perception 컴포넌트와 상태 변수를 추가합니다.

**경로:** `Source/SummerTPS/Public/EnemyCharacter.h`
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyAIState.h" // 새로 추가한 Enum 헤더
#include "EnemyCharacter.generated.h"

class UHealthComponent;
class AWeapon;
class UAIPerceptionComponent; // AI Perception 클래스 전방 선언

UCLASS()
class SUMMERTPS_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

    // AI Perception 컴포넌트 추가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UFUNCTION()
    void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "AI")
    void OnDeath();
    virtual void OnDeath_Implementation();

    // 플레이어 감지 시 호출될 함수
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void Attack();

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AWeapon> DefaultWeaponClass;

    // AI 상태 변수 추가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    EEnemyAIState AIState;

private:
    UPROPERTY()
    AWeapon* CurrentWeapon;

    bool bIsDead;
};
```

### 1.3. EnemyCharacter 소스 파일 수정

`EnemyCharacter.cpp`에 AI Perception 로직과 상태 변경 로직을 구현합니다.

**경로:** `Source/SummerTPS/Private/EnemyCharacter.cpp`
```cpp
#include "EnemyCharacter.h"
#include "HealthComponent.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h" // 관련 헤더 추가
#include "Perception/AISenseConfig_Sight.h"   // 관련 헤더 추가
#include "BehaviorTree/BlackboardComponent.h" // 관련 헤더 추가
#include "AIController.h"                     // 관련 헤더 추가

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    // AI Perception 컴포넌트 생성
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    bIsDead = false;
    // AI 상태 초기화
    AIState = EEnemyAIState::EAS_Idle;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnHealthChanged);
    // Perception 이벤트에 함수 바인딩
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::OnTargetPerceptionUpdated);

    if (DefaultWeaponClass)
    {
        CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
            CurrentWeapon->SetOwner(this);
        }
    }
}

void AEnemyCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        // 죽었을 때 상태 변경
        AIState = EEnemyAIState::EAS_Dead;
        OnDeath();
    }
}

void AEnemyCharacter::OnDeath_Implementation()
{
    // AI 로직 정지
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->GetBrainComponent()->StopLogic("Dead");
    }

    GetCharacterMovement()->StopMovementImmediately();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetSimulatePhysics(true);
    SetLifeSpan(5.0f);
}

// 플레이어 감지 처리 함수 구현
void AEnemyCharacter::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
        if (BlackboardComponent)
        {
            // 감지에 성공했을 경우
            if (Stimulus.WasSuccessfullySensed())
            {
                BlackboardComponent->SetValueAsObject("TargetPlayer", Actor);
                AIState = EEnemyAIState::EAS_Chase;
            }
            // 감지를 놓쳤을 경우
            else
            {
                BlackboardComponent->ClearValue("TargetPlayer");
                AIState = EEnemyAIState::EAS_Patrol;
            }
        }
    }
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnemyCharacter::Attack()
{
    if (CurrentWeapon && !bIsDead)
    {
        CurrentWeapon->Fire();
    }
}
```

### 1.4. Build.cs 파일 수정

프로젝트 빌드 파일에 AI 관련 모듈을 추가합니다.

**경로:** `Source/SummerTPS/SummerTPS.Build.cs`
```csharp
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SummerTPS : ModuleRules
{
    public SummerTPS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        // "AIModule"과 "GameplayTasks" 추가
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "AIModule", "GameplayTasks" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });
    }
}
```

### 1.5. 코드 컴파일

Unreal Editor를 실행하여 C++ 코드를 컴파일하고 에디터에 반영합니다.

---

## 2. 블루프린트 및 AI 에셋 설정

### 2.1. 블랙보드 (Blackboard) 설정

`Content/Blueprints/AI/BB_Enemy` 블랙보드 에셋을 엽니다.

1.  **새 키(New Key) 추가:**
    *   **TargetPlayer**: `Object` 타입, `Base Class`는 `Actor`로 설정합니다.
    *   **PatrolLocation**: `Vector` 타입으로 설정합니다.
    *   **AIState**: `Enum` 타입으로 설정하고, Enum Type에서 `EEnemyAIState`를 검색하여 선택합니다.

### 2.2. 비헤이비어 트리 (Behavior Tree) 구성

`Content/Blueprints/AI/BT_Enemy` 비헤이비어 트리를 엽니다.

1.  **루트(Root)** 노드에 `Selector` 노드를 연결합니다.
2.  `Selector` 노드에 아래 순서대로(왼쪽부터) 자식 노드들을 추가합니다.

    *   **공격 (Attack) 로직:**
        *   `Selector`에 `Sequence` 노드를 추가합니다.
        *   `Sequence` 노드에 `Blackboard` **데코레이터(Decorator)**를 추가하고, `TargetPlayer` is `Set`으로 설정합니다.
        *   `Sequence` 노드에 `Distance` **데코레이터**를 추가하여 플레이어와의 거리가 공격 가능 거리(예: 500) 내에 있는지 확인합니다.
        *   `Sequence` 노드에 `Attack` **태스크(Task)**를 추가합니다. (C++의 `Attack` 함수를 호출하는 새로운 `BTT_Attack` 태스크를 생성하거나, 기존 태스크를 활용합니다.)
        *   `Sequence` 노드에 `Wait` 태스크를 추가하여 공격 딜레이를 설정합니다.

    *   **추적 (Chase) 로직:**
        *   `Selector`에 `Sequence` 노드를 추가합니다.
        *   `Sequence` 노드에 `Blackboard` **데코레이터**를 추가하고, `TargetPlayer` is `Set`으로 설정합니다.
        *   `Sequence` 노드에 `Move To` 태스크를 추가하고, `Blackboard Key`를 `TargetPlayer`로 설정합니다.

    *   **순찰 (Patrol) 로직:**
        *   `Selector`에 `Sequence` 노드를 추가합니다.
        *   `Sequence` 노드에 `BTT_FindPatrolPoint` 태스크를 추가하여 `PatrolLocation` 키를 업데이트합니다.
        *   `Sequence` 노드에 `Move To` 태스크를 추가하고, `Blackboard Key`를 `PatrolLocation`으로 설정합니다.
        *   `Sequence` 노드에 `Wait` 태스크를 추가하여 순찰 지점에서 잠시 대기합니다. (예: 2-4초)

    *   **대기 (Idle) 로직:**
        *   `Selector`에 `Wait` 태스크를 추가합니다. (예: 3-5초) 이것이 가장 오른쪽, 즉 가장 낮은 우선순위의 행동이 됩니다.

### 2.3. BP_EnemyCharacter 블루프린트 설정

`Content/Blueprints/BP_EnemyCharacter` 블루프린트를 엽니다.

1.  **AI Perception 컴포넌트 설정:**
    *   컴포넌트 탭에서 `AIPerception`을 선택합니다.
    *   `Senses Config`에서 `+` 버튼을 눌러 `AI Sight config`를 추가합니다.
    *   `Sight Radius`와 `Lose Sight Radius`, `Peripheral Vision Half Angle Degrees`를 적절한 값으로 설정합니다. (예: Sight Radius 3000, Lose Sight Radius 3500, Angle 90)
    *   `Detection by Affiliation`에서 `Detect Friendlies`, `Detect Neutrals`, `Detect Enemies`를 모두 체크하여 모든 액터를 감지하도록 설정합니다.

2.  **AI Controller 설정:**
    *   클래스 디폴트(Class Defaults)에서 `AI Controller Class`를 `AIController` 또는 커스텀 AI Controller로 설정합니다.
    *   `Auto Possess AI`를 `Placed in World or Spawned`로 설정합니다.

---

이 가이드를 따라 C++ 코드와 블루프린트를 설정하면 기본적인 AI 행동이 구현됩니다. 이후 각 태스크와 데코레이터를 더 정교하게 만들어 AI를 고도화할 수 있습니다.
