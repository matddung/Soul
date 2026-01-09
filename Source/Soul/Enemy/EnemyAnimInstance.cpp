#include "EnemyAnimInstance.h"
#include "EnemyBase.h"
#include "EnemyRanged.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    CachedEnemy = Cast<AEnemyBase>(OwnerPawn);
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        CachedEnemy.Reset();
        Speed = 0.f;
        bIsInAir = false;
        bIsDead = false;
        bIsAttacking = false;
        bIsHitReacting = false;
        return;
    }

    if (!CachedEnemy.IsValid())
    {
        CachedEnemy = Cast<AEnemyBase>(OwnerPawn);
    }

    const FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size();

    if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn))
    {
        if (const UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
        {
            bIsInAir = Movement->IsFalling();
        }
    }

    if (CachedEnemy.IsValid())
    {
        bIsDead = CachedEnemy->IsDead();
        bIsAttacking = CachedEnemy->IsAttacking();
        bIsHitReacting = CachedEnemy->IsHitReacting();
    }
    else
    {
        bIsDead = false;
        bIsAttacking = false;
        bIsHitReacting = false;
    }
}

float UEnemyAnimInstance::PlayAttackMontage()
{
    if (!AttackMontage)
    {
        return 0.f;
    }

    return Montage_Play(AttackMontage, 0.5f);
}

float UEnemyAnimInstance::PlayHitReactMontage()
{
    if (!HitReactMontage)
    {
        return 0.f;
    }

    return Montage_Play(HitReactMontage);
}

void UEnemyAnimInstance::AnimNotify_AttackHitCheck()
{
    OnAttackHitCheck.Broadcast();
}

void UEnemyAnimInstance::AnimNotify_SpawnProjectile()
{
    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    AEnemyRanged* Enemy = Cast<AEnemyRanged>(OwnerPawn);
    if (!Enemy)
    {
        return;
    }

    AAIController* AICon = Cast<AAIController>(Enemy->GetController());
    if (!AICon)
    {
        return;
    }

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    static const FName TargetKey(TEXT("Target"));
    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKey));
    if (!IsValid(TargetActor))
    {
        return;
    }

    Enemy->SpawnProjectileAtTarget(TargetActor);
}