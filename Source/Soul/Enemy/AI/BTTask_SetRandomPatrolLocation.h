#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetRandomPatrolLocation.generated.h"

UCLASS()
class SOUL_API UBTTask_SetRandomPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_SetRandomPatrolLocation();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolLocationKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float DefaultPatrolRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "AI")
    int32 MaxTries = 10;
};