#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyBoss.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UParticleSystem;
class UParticleSystemComponent;

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
    void StopSkillEffectParticle();

    UFUNCTION()
    void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UAnimMontage> HalfHpSkillMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UParticleSystem> SkillEffectParticle = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
    FVector SkillEffectScale = FVector::OneVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
    TObjectPtr<UNiagaraSystem> SkillHitNiagara = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
    float SkillHitRadius = 300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
    float SkillDamage = 100.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skill")
    bool bHasTriggeredHalfHpSkill = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skill")
    bool bHalfHpSkillPending = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Skill")
    bool bIsUsingHalfHpSkill = false;

    bool bIsApplyingSkillDamage = false;

    FTimerHandle HalfHpSkillTimerHandle;
    FTimerHandle BossDefeatTimerHandle;

    UPROPERTY(Transient)
    TObjectPtr<UParticleSystemComponent> ActiveSkillEffectParticle = nullptr;
};