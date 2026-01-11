#include "EnemyBoss.h"
#include "EnemyAnimInstance.h"
#include "../Character/SoulAnimInstance.h"
#include "../Game/SoulPlayerController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AEnemyBoss::AEnemyBoss()
{
    MaxHP = 200.f;
    MovementSpeed = 600.f;
    AttackRange = 300.f;
    AttackCooldown = 1.0f;
    AttackDamage = 35.f;
}

bool AEnemyBoss::IsApplyingSkillDamage() const
{
    return bIsApplyingSkillDamage;
}

void AEnemyBoss::BeginPlay()
{
    Super::BeginPlay();

    if (EnemyAnimInstance)
    {
        EnemyAnimInstance->OnSkillEffect.AddUObject(this, &AEnemyBoss::HandleSkillEffect);
        EnemyAnimInstance->OnSkillHitCheck.AddUObject(this, &AEnemyBoss::HandleSkillHitCheck);
    }

    if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInstance->OnMontageEnded.AddDynamic(this, &AEnemyBoss::OnSkillMontageEnded);
    }
}

float AEnemyBoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsUsingHalfHpSkill)
    {
        return 0.f;
    }

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    TryTriggerHalfHpSkill();

    return ActualDamage;
}

void AEnemyBoss::ResetAttackState()
{
    Super::ResetAttackState();

    if (bHalfHpSkillPending)
    {
        TryTriggerHalfHpSkill();
    }
}

void AEnemyBoss::ResetHitReaction()
{
    Super::ResetHitReaction();

    if (bHalfHpSkillPending)
    {
        TryTriggerHalfHpSkill();
    }
}

void AEnemyBoss::HandleDeath()
{
    Super::HandleDeath();

    GetWorldTimerManager().ClearTimer(HalfHpSkillTimerHandle);
    bHalfHpSkillPending = false;
    bIsUsingHalfHpSkill = false;
    StopSkillEffectParticle();

    if (ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        SoulPC->HandleBossDefeated();
    }
}

void AEnemyBoss::TryTriggerHalfHpSkill()
{
    if (bHasTriggeredHalfHpSkill || bIsDead)
    {
        return;
    }

    const bool bReachedHalfHp = CurrentHP <= (MaxHP * 0.5f);
    if (!bReachedHalfHp)
    {
        return;
    }

    if (bIsAttacking || bIsHitReacting)
    {
        bHalfHpSkillPending = true;
        return;
    }

    ExecuteHalfHpSkill();
}

void AEnemyBoss::ExecuteHalfHpSkill()
{
    if (bHasTriggeredHalfHpSkill || bIsDead)
    {
        return;
    }

    bHasTriggeredHalfHpSkill = true;
    bHalfHpSkillPending = false;
    bIsAttacking = true;
    bIsUsingHalfHpSkill = true;

    if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
    {
        CachedMovementMode = CharacterMovementComp->MovementMode;
        CharacterMovementComp->StopMovementImmediately();
        CharacterMovementComp->DisableMovement();
        bMovementPausedForAttack = true;
    }

    if (HalfHpSkillMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            const float Duration = AnimInstance->Montage_Play(HalfHpSkillMontage);
            if (Duration > 0.f)
            {
                GetWorldTimerManager().SetTimer(HalfHpSkillTimerHandle, this, &AEnemyBoss::ResetSkillState, Duration, false);
                return;
            }
        }
    }

    ResetSkillState();
}

void AEnemyBoss::ResetSkillState()
{
    GetWorldTimerManager().ClearTimer(HalfHpSkillTimerHandle);
    bIsUsingHalfHpSkill = false;
    StopSkillEffectParticle();
    ResetAttackState();
}

void AEnemyBoss::HandleSkillEffect()
{
    if (!SkillEffectParticle)
    {
        return;
    }

    StopSkillEffectParticle();

    const FVector BossLocation = GetActorLocation();
    const FVector EffectLocation(BossLocation.X, BossLocation.Y, 0.f);
    ActiveSkillEffectParticle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SkillEffectParticle, EffectLocation, GetActorRotation());
}

void AEnemyBoss::HandleSkillHitCheck()
{
    if (!SkillHitNiagara)
    {
        return;
    }

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!PlayerCharacter)
    {
        return;
    }

#if ENABLE_DRAW_DEBUG
    DrawDebugSphere(GetWorld(), GetActorLocation(), SkillHitRadius, 24, FColor::Red, false, 1.0f);
#endif

    const float DistanceToPlayer = FVector::Dist(PlayerCharacter->GetActorLocation(), GetActorLocation());
    if (DistanceToPlayer > SkillHitRadius)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAttached(SkillHitNiagara, PlayerCharacter->GetMesh(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

    TGuardValue<bool> ApplyingSkillDamageGuard(bIsApplyingSkillDamage, true);
    UGameplayStatics::ApplyDamage(PlayerCharacter, SkillDamage, GetController(), this, nullptr);
}

void AEnemyBoss::StopSkillEffectParticle()
{
    if (ActiveSkillEffectParticle)
    {
        ActiveSkillEffectParticle->DeactivateSystem();
        ActiveSkillEffectParticle = nullptr;
    }
}

void AEnemyBoss::OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != HalfHpSkillMontage)
    {
        return;
    }

    ResetSkillState();
}