#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecuteAttack.generated.h"

UCLASS()
class SOUL_API UBTTask_ExecuteAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_ExecuteAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    bool bFailIfOnCooldown = true;
};