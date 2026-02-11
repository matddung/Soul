#include "BTTask_ExecuteAttack.h"
#include "../EnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ExecuteAttack::UBTTask_ExecuteAttack()
{
    NodeName = TEXT("Execute Attack");
}

EBTNodeResult::Type UBTTask_ExecuteAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!AICon || !BB) return EBTNodeResult::Failed;

    AEnemyBase* Enemy = Cast<AEnemyBase>(AICon->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!IsValid(Target)) return EBTNodeResult::Failed;

    const bool bDidAttack = Enemy->TryExecuteAttack(Target);
    if (!bDidAttack)
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Succeeded;
}