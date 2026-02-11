#include "BTService_InAttackRange.h"
#include "../EnemyBase.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_InAttackRange::UBTService_InAttackRange()
{
    NodeName = TEXT("Update InAttackRange");
    Interval = 0.1f;
    RandomDeviation = 0.0f;
}

void UBTService_InAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!BB || !AICon) return;

    AEnemyBase* Enemy = Cast<AEnemyBase>(AICon->GetPawn());
    if (!Enemy) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!IsValid(Target))
    {
        BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, false);
        return;
    }

    float AttackRange = DefaultAttackRange;

    if (AttackRangeKey.SelectedKeyName != NAME_None)
    {
        const float FromBB = BB->GetValueAsFloat(AttackRangeKey.SelectedKeyName);
        if (FromBB > 0.f) AttackRange = FromBB;
    }

    const bool bInRange = Enemy->IsTargetInAttackRange(Target, AttackRange);

    BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInRange);
}