#include "BTTask_SetNextPatrolPoint.h"
#include "../EnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/TargetPoint.h"

UBTTask_SetNextPatrolPoint::UBTTask_SetNextPatrolPoint()
{
    NodeName = TEXT("Set Next Patrol Point");
}

EBTNodeResult::Type UBTTask_SetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!BB || !AICon)
    {
        return EBTNodeResult::Failed;
    }

    AEnemyBase* Enemy = Cast<AEnemyBase>(AICon->GetPawn());
    if (!Enemy)
    {
        return EBTNodeResult::Failed;
    }

    const int32 Num = Enemy->PatrolPoints.Num();
    if (Num <= 0)
    {
        BB->ClearValue(PatrolPointKey.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    if (Enemy->CurrentPatrolIndex < 0 || Enemy->CurrentPatrolIndex >= Num)
    {
        Enemy->CurrentPatrolIndex = 0;
        Enemy->bPatrolForward = true;
    }
    else
    {
        int32 NextIndex = Enemy->CurrentPatrolIndex + (Enemy->bPatrolForward ? 1 : -1);

        if (NextIndex >= Num)
        {
            Enemy->bPatrolForward = false;
            NextIndex = FMath::Clamp(Enemy->CurrentPatrolIndex - 1, 0, Num - 1);
        }
        else if (NextIndex < 0)
        {
            Enemy->bPatrolForward = true;
            NextIndex = FMath::Clamp(Enemy->CurrentPatrolIndex + 1, 0, Num - 1);
        }

        if (Num == 1)
        {
            NextIndex = 0;
        }

        if (bAvoidSamePoint && Num >= 2 && NextIndex == Enemy->CurrentPatrolIndex)
        {
            NextIndex = Enemy->bPatrolForward
                ? FMath::Min(Enemy->CurrentPatrolIndex + 1, Num - 1)
                : FMath::Max(Enemy->CurrentPatrolIndex - 1, 0);
        }

        Enemy->CurrentPatrolIndex = NextIndex;
    }

    ATargetPoint* NextPoint = Enemy->PatrolPoints.IsValidIndex(Enemy->CurrentPatrolIndex)
        ? Enemy->PatrolPoints[Enemy->CurrentPatrolIndex].Get()
        : nullptr;

    if (!IsValid(NextPoint))
    {
        BB->ClearValue(PatrolPointKey.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    BB->SetValueAsObject(PatrolPointKey.SelectedKeyName, NextPoint);
    return EBTNodeResult::Succeeded;
}