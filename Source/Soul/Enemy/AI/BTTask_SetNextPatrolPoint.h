#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetNextPatrolPoint.generated.h"

UCLASS()
class SOUL_API UBTTask_SetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_SetNextPatrolPoint();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolPointKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    bool bAvoidSamePoint = true;
};