#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_EnumCompareBase.generated.h"

UCLASS()
class SOUL_API UBTDecorator_EnumCompareBase : public UBTDecorator
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector EnumKey;

protected:
    virtual UEnum* GetAllowedEnum() const PURE_VIRTUAL(BTDecorator_EnumTypeEquals::GetAllowedEnum, return nullptr;);

    virtual uint8 GetExpectedValue() const PURE_VIRTUAL(BTDecorator_EnumTypeEquals::GetExpectedValue, return 0;);

    virtual void InitializeFromAsset(UBehaviorTree & Asset) override;
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const override;
};