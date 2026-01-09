#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_ClearTargetIfDead.generated.h"

UCLASS()
class SOUL_API UBTTaskNode_ClearTargetIfDead : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTaskNode_ClearTargetIfDead();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
};