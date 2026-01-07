#include "BTDecorator_InAttackRange.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_InAttackRange::UBTDecorator_InAttackRange()
{
    NodeName = TEXT("In Attack Range");
}

void UBTDecorator_InAttackRange::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    UBlackboardData* BlackboardData = GetBlackboardAsset();
    if (!BlackboardData)
    {
        return;
    }

    DistanceKey.ResolveSelectedKey(*BlackboardData);
    AttackRangeKey.ResolveSelectedKey(*BlackboardData);

    DistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_InAttackRange, DistanceKey));
    AttackRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_InAttackRange, AttackRangeKey));
}

bool UBTDecorator_InAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

    const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp || !OwnerComp.GetAIOwner())
    {
        return false;
    }

    const float Distance = BlackboardComp->GetValueAsFloat(DistanceKey.SelectedKeyName);
    const float AttackRange = BlackboardComp->GetValueAsFloat(AttackRangeKey.SelectedKeyName);

    return Distance <= AttackRange;
}