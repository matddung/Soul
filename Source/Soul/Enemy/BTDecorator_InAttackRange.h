#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_InAttackRange.generated.h"

UCLASS()
class SOUL_API UBTDecorator_InAttackRange : public UBTDecorator_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTDecorator_InAttackRange();

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector AttackRangeKey;

protected:
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};