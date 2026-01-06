#include "EnemyBoss.h"
#include "EnemyAnimInstance.h"

AEnemyBoss::AEnemyBoss()
{
    MaxHP = 200.f;
    MovementSpeed = 600.f;
    AttackRange = 350.f;
    AttackCooldown = 1.0f;
    AttackDamage = 35.f;
}