#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBlackboardComponent;
class UBehaviorTree;
class UEnemyAIConfig;

UCLASS()
class SOUL_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Config")
    TObjectPtr<UEnemyAIConfig> AIConfig;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    void BindToPlayerDeath();
    void RefreshTargetInfo();
    bool TryClearTargetIfDead();
    AActor* GetNextPatrolPoint();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

private:
    UFUNCTION()
    void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void HandlePlayerDead();

    void ApplyConfigToPerception();
    void ApplyConfigToBlackboard();
    void UpdateTargeting();
    void SetTarget(AActor* NewTarget);
    void ClearTarget();
    bool IsTargetDead(const AActor* TargetActor) const;
    bool CanSeePlayer() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Blackboard", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBlackboardComponent> BlackboardComp;

    UPROPERTY()
    TObjectPtr<APawn> CachedPlayerPawn;

    FTimerHandle PerceptionUpdateTimer;

    static const FName Key_TargetActor;
    static const FName Key_LastKnownTargetLocation;
    static const FName Key_HasLineOfSight;
    static const FName Key_DistanceToTarget;
    static const FName Key_HomeLocation;
    static const FName Key_EnemyType;
    static const FName Key_PatrolMode;
    static const FName Key_PatrolRadius;
    static const FName Key_AggroRange;
    static const FName Key_AttackRange;
};