#include "BTTask_SetNextPatrolPoint.h"
#include "EnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetNextPatrolPoint::UBTTask_SetNextPatrolPoint()
{
    NodeName = TEXT("Set Next Patrol Point");
}

void UBTTask_SetNextPatrolPoint::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    UBlackboardData* BlackboardData = GetBlackboardAsset();
    if (!BlackboardData)
    {
        return;
    }

    PatrolLocationKey.ResolveSelectedKey(*BlackboardData);
    PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SetNextPatrolPoint, PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_SetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    AEnemyAIController* EnemyController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!EnemyController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    AActor* PatrolPoint = EnemyController->GetNextPatrolPoint();
    if (!PatrolPoint)
    {
        return EBTNodeResult::Failed;
    }

    BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, PatrolPoint->GetActorLocation());
    return EBTNodeResult::Succeeded;
}