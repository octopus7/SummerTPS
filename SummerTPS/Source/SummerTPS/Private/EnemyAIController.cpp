#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (BehaviorTree && BlackboardData)
	{
		// Blackboard 컴포넌트가 유효한지 확인하고 초기화합니다.
		if (UBlackboardComponent* BBComp = GetBlackboardComponent())
		{
			BBComp->InitializeBlackboard(*BlackboardData);
		}
		RunBehaviorTree(BehaviorTree);
	}
}
