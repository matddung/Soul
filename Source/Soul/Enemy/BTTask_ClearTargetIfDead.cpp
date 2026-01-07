#include "BTTask_ClearTargetIfDead.h"
#include "EnemyAIController.h"

UBTTask_ClearTargetIfDead::UBTTask_ClearTargetIfDead()
{
    NodeName = TEXT("Clear Target If Dead");
}

EBTNodeResult::Type UBTTask_ClearTargetIfDead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    AEnemyAIController* EnemyController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
    if (!EnemyController)
    {
        return EBTNodeResult::Failed;
    }

    const bool bCleared = EnemyController->TryClearTargetIfDead();
    return bCleared ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}