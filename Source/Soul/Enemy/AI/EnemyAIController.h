#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIConfig.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyAIController.generated.h"

class UBlackboardData;
class UBehaviorTree;
class UEnemyAIConfig;

UCLASS()
class SOUL_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    void ApplyAIConfig(const UEnemyAIConfig* Config);

public:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBlackboardData> BlackboardAsset;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Combat")
    float AggroRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Combat")
    float AttackRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
    float PatrolRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Type")
    EEnemyType EnemyType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
    EPatrolMode PatrolMode;
};