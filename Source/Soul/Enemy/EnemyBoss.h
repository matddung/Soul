#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyBoss.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UParticleSystem;

UCLASS()
class SOUL_API AEnemyBoss : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemyBoss();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    bool IsApplyingSkillDamage() const;

protected:
    virtual void BeginPlay() override;
    virtual void ResetAttackState() override;
    virtual void ResetHitReaction() override;
    virtual void HandleDeath() override;

    void TryTriggerHalfHpSkill();
    void ExecuteHalfHpSkill();
    void ResetSkillState();
    void HandleSkillEffect();
    void HandleSkillHitCheck();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Skill")
    TObjectPtr<UAnimMontage> HalfHpSkillMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Skill")
    TObjectPtr<UParticleSystem> SkillEffectParticle = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Skill")
    TObjectPtr<UNiagaraSystem> SkillHitNiagara = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Skill", meta = (ClampMin = "0.0"))
    float SkillHitRadius = 300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Skill", meta = (ClampMin = "0.0"))
    float SkillDamage = 100.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Skill")
    bool bHasTriggeredHalfHpSkill = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Skill")
    bool bHalfHpSkillPending = false;

    bool bIsApplyingSkillDamage = false;

    FTimerHandle HalfHpSkillTimerHandle;
};