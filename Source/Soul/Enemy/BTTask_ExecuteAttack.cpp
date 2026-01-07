#include "BTTask_ExecuteAttack.h"
#include "EnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ExecuteAttack::UBTTask_ExecuteAttack()
{
    NodeName = TEXT("Execute Attack");
}

void UBTTask_ExecuteAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    UBlackboardData* BlackboardData = GetBlackboardAsset();
    if (!BlackboardData)
    {
        return;
    }

    TargetActorKey.ResolveSelectedKey(*BlackboardData);
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ExecuteAttack, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_ExecuteAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!BlackboardComp || !AIController)
    {
        return EBTNodeResult::Failed;
    }

    AEnemyBase* EnemyPawn = Cast<AEnemyBase>(AIController->GetPawn());
    if (!EnemyPawn)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    if (!EnemyPawn->CanAttack(TargetActor))
    {
        return EBTNodeResult::Failed;
    }

    EnemyPawn->DoAttack(TargetActor);
    return EBTNodeResult::Succeeded;
}