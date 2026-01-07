#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ClearTargetIfDead.generated.h"

UCLASS()
class SOUL_API UBTTask_ClearTargetIfDead : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ClearTargetIfDead();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};