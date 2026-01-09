#include "BTDecorator_EnumCompareBase.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

void UBTDecorator_EnumCompareBase::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    UBlackboardData* BB = GetBlackboardAsset();
    if (!BB)
    {
        return;
    }

    EnumKey.ResolveSelectedKey(*BB);

    if (UEnum* Allowed = GetAllowedEnum())
    {
        EnumKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_EnumCompareBase, EnumKey), Allowed);
    }
}

bool UBTDecorator_EnumCompareBase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return false;
    }

    const uint8 Current = BB->GetValueAsEnum(EnumKey.SelectedKeyName);
    return Current == GetExpectedValue();
}