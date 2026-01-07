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

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolLocationKey;

protected:
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};