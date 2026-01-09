#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_InAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class SOUL_API UBTService_InAttackRange : public UBTService
{
	GENERATED_BODY()
	
public:
    UBTService_InAttackRange();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector InAttackRangeKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector AttackRangeKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float DefaultAttackRange = 200.f;
};