#include "EnemyRanged.h"
#include "EnemyAnimInstance.h"

AEnemyRanged::AEnemyRanged()
{
    MaxHP = 100.f;
    MovementSpeed = 300.f;
    AttackRange = 850.f;
    AttackCooldown = 2.5f;
    AttackDamage = 15.f;
}