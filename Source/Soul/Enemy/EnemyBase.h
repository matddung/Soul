#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "EnemyBase.generated.h"

class UWidgetComponent;
class UEnemyHPBarWidget;
class AFloatingDamageActor;
class UEnemyAnimInstance;
class UEnemyAIConfig;

UCLASS()
class SOUL_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category = "State")
    bool IsDead() const;

    UFUNCTION(BlueprintPure, Category = "State")
    bool IsAttacking() const;

    UFUNCTION(BlueprintPure, Category = "State")
    bool IsHitReacting() const;

    virtual void SetCurrentTarget(AActor* Target);
    AActor* GetCurrentTarget() const;

    virtual bool CanAttack(AActor* Target) const;
    virtual void DoAttack(AActor* Target);

protected:
    virtual void BeginPlay() override;

    virtual void HandleDeath();
    virtual void HandleHitReaction();
    virtual void ResetAttackState();
    virtual void ResetHitReaction();
    virtual void UpdateHPBar();
    void SpawnFloatingDamage(float Damage);

public:
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
    TArray<TObjectPtr<AActor>> PatrolRoutePoints;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
    int32 PatrolRouteIndex = 0;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MaxHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    float MovementSpeed;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
    bool bIsDead = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
    bool bIsHitReacting = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackDamage;

    UPROPERTY(VisibleInstanceOnly, Category = "Combat")
    float LastAttackTime = -AttackCooldown;;

    UPROPERTY(VisibleInstanceOnly, Category = "Target")
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UWidgetComponent> HPBarWidgetComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;

    UPROPERTY()
    TObjectPtr<UEnemyAnimInstance> EnemyAnimInstance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UEnemyAIConfig> AIConfig;

    FTimerHandle AttackTimerHandle;
    FTimerHandle HitReactTimerHandle;
};