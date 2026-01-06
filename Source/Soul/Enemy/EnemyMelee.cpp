#include "EnemyMelee.h"
#include "EnemyAnimInstance.h"

AEnemyMelee::AEnemyMelee()
{
    MaxHP = 150.f;
    MovementSpeed = 400.f;
    AttackRange = 250.f;
    AttackCooldown = 1.25f;
    AttackDamage = 20.f;
}