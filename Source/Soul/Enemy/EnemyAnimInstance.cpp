#include "EnemyAnimInstance.h"
#include "EnemyBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

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

    return Montage_Play(AttackMontage);
}

float UEnemyAnimInstance::PlayHitReactMontage()
{
    if (!HitReactMontage)
    {
        return 0.f;
    }

    return Montage_Play(HitReactMontage);
}