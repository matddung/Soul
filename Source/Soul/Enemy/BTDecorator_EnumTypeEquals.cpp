#include "BTDecorator_EnumTypeEquals.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UBTDecorator_EnumTypeEquals::UBTDecorator_EnumTypeEquals()
{
    NodeName = TEXT("Enum Equals (Base)");
}

void UBTDecorator_EnumTypeEquals::InitializeFromAsset(UBehaviorTree& Asset)
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
        EnumKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_EnumTypeEquals, EnumKey), Allowed);
    }
}

bool UBTDecorator_EnumTypeEquals::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return false;
    }

    const uint8 Current = BB->GetValueAsEnum(EnumKey.SelectedKeyName);
    return Current == GetExpectedValue();
}