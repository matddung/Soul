#include "EnemyMelee.h"
#include "EnemyAnimInstance.h"

AEnemyMelee::AEnemyMelee()
{
    MaxHP = 150.f;
    MovementSpeed = 400.f;
    AttackRange = 200.f;
    AttackCooldown = 2.f;
    AttackDamage = 20.f;
}