#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetHomeLocation.generated.h"

UCLASS()
class SOUL_API UBTTask_SetHomeLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_SetHomeLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};