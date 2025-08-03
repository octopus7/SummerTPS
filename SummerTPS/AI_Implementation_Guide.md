# 적 AI 구현 가이드

이 문서는 Unreal Engine 5 프로젝트 `SummerTPS`에서 적 AI의 보편적인 행동 패턴(대기, 순찰, 플레이어 추적, 공격)을 구현하는 과정을 설명합니다. Behavior Tree와 Blackboard를 활용하여 AI 로직을 구성합니다.

## 1. 프로젝트 설정 변경 (C++)

### 1.1. `SummerTPS.Build.cs` 수정

AI 기능을 사용하기 위해 `AIModule`과 `GameplayTasks` 모듈을 `PublicDependencyModuleNames`에 추가합니다.

**파일:** `Source/SummerTPS/SummerTPS.Build.cs`

**변경 전:**
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
```

**변경 후:**
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "GameplayTasks" });
```

## 2. AI Controller 생성 (C++)

적 캐릭터를 제어할 커스텀 AI 컨트롤러 `AEnemyAIController`를 생성합니다. 이 컨트롤러는 Behavior Tree를 실행하고 Blackboard를 관리합니다.

### 2.1. `EnemyAIController.h`

**파일:** `Source/SummerTPS/Public/EnemyAIController.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class SUMMERTPS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	// Behavior Tree와 Blackboard Data를 외부에서 설정할 수 있도록 public으로 변경
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBlackboardData* BlackboardData;

	// BeginPlay 함수 선언 (컴파일 에러 해결을 위해 public으로 임시 변경)
	virtual void BeginPlay() override;
};
```

### 2.2. `EnemyAIController.cpp`

**파일:** `Source/SummerTPS/Private/EnemyAIController.cpp`

```cpp
#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h" // BlackboardComponent 사용을 위해 추가

AEnemyAIController::AEnemyAIController()
{
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree && BlackboardData)
	{
		// Blackboard 컴포넌트가 유효한지 확인하고 초기화
		if (UBlackboardComponent* BBComp = GetBlackboardComponent())
		{
			BBComp->InitializeBlackboard(*BlackboardData);
		}
		RunBehaviorTree(BehaviorTree);
	}
}
```

## 3. `EnemyCharacter` 수정 (C++)

`EnemyCharacter` 클래스에 AI 컨트롤러를 연결하고, AI 인지(Perception) 기능을 추가하며, Behavior Tree 및 Blackboard 에셋을 할당할 수 있는 속성을 추가합니다.

### 3.1. `EnemyCharacter.h`

**파일:** `Source/SummerTPS/Public/EnemyCharacter.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h" // AI 인지 컴포넌트
#include "Perception/AISenseConfig_Sight.h"   // 시야 감지 설정
#include "EnemyAIController.h"               // 커스텀 AI 컨트롤러
#include "EnemyCharacter.generated.h"

class UHealthComponent;
class AWeapon;
class UBehaviorTree; // Behavior Tree 클래스
class UBlackboardData; // Blackboard Data 클래스

UCLASS(Blueprintable, meta = (AIControllerClass = "AEnemyAIController")) // AI 컨트롤러 클래스 지정
class SUMMERTPS_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComponent;

    UFUNCTION()
    void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "AI")
    void OnDeath();
    virtual void OnDeath_Implementation();

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void Attack();

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<AWeapon> DefaultWeaponClass;

private:
    UPROPERTY()
    AWeapon* CurrentWeapon;

    bool bIsDead;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent; // AI 인지 컴포넌트

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTree; // 할당할 Behavior Tree 에셋

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* BlackboardData; // 할당할 Blackboard Data 에셋

    // 시야 감지 관련 설정 변수
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SightRadius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SightDetectionByAffiliation = 0.0f;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus); // 인지 업데이트 이벤트 핸들러
};
```

### 3.2. `EnemyCharacter.cpp`

**파일:** `Source/SummerTPS/Private/EnemyCharacter.cpp`

```cpp
#include "EnemyCharacter.h"
#include "HealthComponent.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h" // AI 컨트롤러 헤더
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h" // BlackboardComponent 사용을 위해 추가
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    // SetPerceptionComponent(*AIPerceptionComponent); // 이 줄은 제거됨 (자동으로 설정됨)

    // 시야 감지 설정
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true; // 적 감지 활성화
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyCharacter::OnPerceptionUpdated);

    bIsDead = false;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::OnHealthChanged);

    if (DefaultWeaponClass)
    {
        CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
        if (CurrentWeapon)
        {
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket")); // Assuming a "WeaponSocket" exists on the skeleton
            CurrentWeapon->SetOwner(this);
        }
    }

    // AI 컨트롤러에 Behavior Tree와 Blackboard Data 전달
    AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());
    if (AICon)
    {
        if (BehaviorTree && BlackboardData)
        {
            AICon->BehaviorTree = BehaviorTree;
            AICon->BlackboardData = BlackboardData;
            // AICon->RunBehaviorTree(BehaviorTree); // BeginPlay에서 직접 실행하지 않고, AIController의 BeginPlay에서 실행되도록 함
        }
    }
}

void AEnemyCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath();
    }
}

void AEnemyCharacter::OnDeath_Implementation()
{
    // Stop AI logic here
    GetCharacterMovement()->StopMovementImmediately();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll or play death animation
    GetMesh()->SetSimulatePhysics(true);

    SetLifeSpan(5.0f); // Actor will be destroyed after 5 seconds
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

// 인지 업데이트 이벤트 핸들러 구현
void AEnemyCharacter::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());
    if (AICon && AICon->GetBlackboardComponent())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // 플레이어를 감지하면 Blackboard의 TargetActor 키에 설정
            AICon->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Actor);
        }
        else
        {
            // 플레이어를 시야에서 놓치면 TargetActor 키를 지움
            AICon->GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
        }
    }
}
```

## 4. Unreal Editor 설정

C++ 코드 컴파일 후 Unreal Editor에서 다음 단계를 진행합니다.

### 4.1. Behavior Tree 및 Blackboard 에셋 생성

*   `Content Browser`에서 `AI` 폴더 (예: `Content/AI`)를 생성합니다.
*   `AI` 폴더에서 마우스 오른쪽 버튼 클릭:
    *   `Artificial Intelligence` -> `Behavior Tree` 선택, 이름: `BT_EnemyAI`
    *   `Artificial Intelligence` -> `Blackboard` 선택, 이름: `BB_EnemyAI`

### 4.2. Blackboard (`BB_EnemyAI`)에 키 추가

`BB_EnemyAI` 에셋을 열고 `Keys` 섹션에서 다음 키들을 추가합니다.

*   **`TargetActor`**:
    *   `Key Name`: `TargetActor`
    *   `Key Type`: `Object`
    *   `Base Class`: `Actor`
*   **`PatrolLocation`**:
    *   `Key Name`: `PatrolLocation`
    *   `Key Type`: `Vector`

### 4.3. `BP_EnemyCharacter` 블루프린트 수정

`BP_EnemyCharacter` 블루프린트를 열고 다음을 설정합니다.

*   **`Details` 패널 -> `Pawn` 섹션:**
    *   `AI Controller Class`: `EnemyAIController`로 설정
*   **`Details` 패널 -> `AI` 섹션:**
    *   `Behavior Tree`: `BT_EnemyAI` 에셋 할당
    *   `Blackboard Data`: `BB_EnemyAI` 에셋 할당

### 4.4. Behavior Tree (`BT_EnemyAI`) 구조 설정

`BT_EnemyAI` 에셋을 열고 다음 노드들을 추가하고 연결합니다.

*   **Root 노드**
    *   `Selector` 노드 (Root의 자식)
        *   **`Attack Sequence` (Sequence 노드)**
            *   **Decorator:** `Blackboard` (Key: `TargetActor`, Key Query: `Is Set`)
            *   **Task:** `Run BTTask_BlueprintBase` (나중에 `BTT_Attack`으로 변경)
        *   **`Chase Sequence` (Sequence 노드)**
            *   **Decorator:** `Blackboard` (Key: `TargetActor`, Key Query: `Is Set`)
            *   **Task:** `Move To` (Blackboard Key: `TargetActor`)
        *   **`Patrol Sequence` (Sequence 노드)**
            *   **Task:** `Run BTTask_BlueprintBase` (나중에 `BTT_FindPatrolLocation`으로 변경)
            *   **Task:** `Move To` (Blackboard Key: `PatrolLocation`)
            *   **Task:** `Wait` (Wait Time: 3.0, Random Deviation: 1.0)
        *   **`Idle Sequence` (Sequence 노드)**
            *   **Task:** `Wait` (Wait Time: 5.0, Random Deviation: 2.0)

*   **`Selector` 노드의 자식 순서 (왼쪽에서 오른쪽):** `Attack Sequence` -> `Chase Sequence` -> `Patrol Sequence` -> `Idle Sequence`

### 4.5. 커스텀 Behavior Tree 태스크 블루프린트 생성

`AI` 폴더에서 다음 두 개의 `BTTask_BlueprintBase`를 상속받는 블루프린트를 생성합니다.

#### 4.5.1. `BTT_Attack`

*   **이름:** `BTT_Attack`
*   **부모 클래스:** `BTTask_BlueprintBase`
*   **구현:**
    *   `Event Receive Execute AI` 노드에서 시작.
    *   `Controlled Pawn`을 `Cast To EnemyCharacter`로 캐스팅.
    *   캐스팅 성공 시 `Attack` 함수 호출.
    *   `Finish Execute` 노드를 `Success`로 연결.

#### 4.5.2. `BTT_FindPatrolLocation`

*   **이름:** `BTT_FindPatrolLocation`
*   **부모 클래스:** `BTTask_BlueprintBase`
*   **구현:**
    *   `Event Receive Execute AI` 노드에서 시작.
    *   `Get AI Controller` -> `Get Controlled Pawn` -> `Get Actor Location`으로 현재 AI의 위치를 가져옴.
    *   `Get Random Reachable Point in Radius` 노드를 사용하여 랜덤 순찰 지점 계산 (Origin: AI 위치, Radius: 1000.0).
    *   `Get AI Controller` -> `Get Blackboard Component` 노드를 가져옴.
    *   `Set Value as Vector` 노드를 사용하여 `PatrolLocation` 키에 계산된 랜덤 위치를 설정 (Key Name: `PatrolLocation`, Value: 랜덤 위치).
    *   `Finish Execute` 노드를 `Success`로 연결.

### 4.6. Behavior Tree (`BT_EnemyAI`) 업데이트

`BT_EnemyAI` 에셋을 다시 열고, `Run BTTask_BlueprintBase` 노드들을 생성한 커스텀 태스크로 변경합니다.

*   `Attack Sequence` 아래의 `Run BTTask_BlueprintBase` 노드를 선택하고 `Task` 드롭다운을 `BTT_Attack`으로 변경.
*   `Patrol Sequence` 아래의 첫 번째 `Run BTTask_BlueprintBase` 노드를 선택하고 `Task` 드롭다운을 `BTT_FindPatrolLocation`으로 변경.

모든 설정을 완료한 후, 프로젝트를 저장하고 게임을 실행하여 적 AI의 행동을 테스트할 수 있습니다.
