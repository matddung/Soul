#include "EnemyRanged.h"
#include "EnemyAnimInstance.h"
#include "RangedArrowProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"

AEnemyRanged::AEnemyRanged()
{
    MaxHP = 100.f;
    MovementSpeed = 300.f;
    AttackRange = 850.f;
    AttackCooldown = 2.5f;
    AttackDamage = 15.f;
}

void AEnemyRanged::SpawnProjectileAtTarget(AActor* TargetActor)
{
    if (!ProjectileClass)
    {
        return;
    }

    if (!IsValid(TargetActor))
    {
        return;
    }

    const FVector SpawnLoc = GetMesh()->GetSocketLocation(MuzzleSocketName);

    FVector AimLoc = TargetActor->GetActorLocation();
    AimLoc.Z += 50.f;

    const FVector Dir = (AimLoc - SpawnLoc).GetSafeNormal();
    const FRotator SpawnRot = Dir.Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ARangedArrowProjectile* Proj =
        GetWorld()->SpawnActor<ARangedArrowProjectile>(ProjectileClass, SpawnLoc, SpawnRot, Params);

    if (!Proj)
    {
        return;
    }

    AController* InstigatorController = GetController();
    Proj->InitProjectile(InstigatorController, this, AttackDamage);

    if (UProjectileMovementComponent* MoveComp = Proj->GetProjectileMovement())
    {
        const float Speed = MoveComp->InitialSpeed > 0.f ? MoveComp->InitialSpeed : 3000.f;

        MoveComp->bInitialVelocityInLocalSpace = false;
        MoveComp->Velocity = Dir * Speed;
        MoveComp->Activate(true);
    }
}