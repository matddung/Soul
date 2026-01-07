#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetRandomPatrolLocation.generated.h"

UCLASS()
class SOUL_API UBTTask_SetRandomPatrolLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()
public:
    UBTTask_SetRandomPatrolLocation();

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName HomeLocationKeyName = TEXT("HomeLocation");

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FName PatrolRadiusKeyName = TEXT("PatrolRadius");

protected:
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};