#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

UCLASS()
class SOUL_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

protected:
    virtual void OnPossess(APawn* InPawn) override;

public:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<class UBehaviorTree> BehaviorTreeAsset;

};