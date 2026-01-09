#include "BTTask_SetRandomPatrolLocation.h"
#include "../EnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_SetRandomPatrolLocation::UBTTask_SetRandomPatrolLocation()
{
    NodeName = TEXT("Set Random Patrol Location");
}

EBTNodeResult::Type UBTTask_SetRandomPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Enemy->GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    const FVector Origin = Enemy->PatrolOrigin;
    const float Radius = DefaultPatrolRadius;

    FNavLocation ResultLocation;
    bool bFound = false;

    for (int32 i = 0; i < MaxTries; ++i)
    {
        if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, ResultLocation))
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        return EBTNodeResult::Failed;
    }

    BB->SetValueAsVector(PatrolLocationKey.SelectedKeyName, ResultLocation.Location);
    return EBTNodeResult::Succeeded;
}