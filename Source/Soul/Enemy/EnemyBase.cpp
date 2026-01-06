#include "EnemyBase.h"
#include "../UI/EnemyHPBarWidget.h"
#include "../UI/FloatingDamageActor.h"
#include "EnemyAnimInstance.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
    HPBarWidgetComponent->SetupAttachment(GetCapsuleComponent());
    HPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    HPBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    HPBarWidgetComponent->SetWidgetClass(UEnemyHPBarWidget::StaticClass());

    EnemyAnimInstanceClass = UEnemyAnimInstance::StaticClass();
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHP = MaxHP;

    UpdateHPBar();

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        SkeletalMesh->SetAnimInstanceClass(EnemyAnimInstanceClass);
    }

    if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
    {
        CharacterMovementComponent->MaxWalkSpeed = MovementSpeed;
    }
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead)
    {
        return 0.f;
    }

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    const float FinalDamage = FMath::Max(ActualDamage, 0.f);

    if (FinalDamage <= 0.f)
    {
        return 0.f;
    }

    CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0.f, MaxHP);

    SpawnFloatingDamage(FinalDamage);
    UpdateHPBar();

    if (CurrentHP <= 0.f)
    {
        bIsDead = true;
        HandleDeath();
    }
    else
    {
        bIsHitReacting = true;
        HandleHitReaction();
    }

    return FinalDamage;
}

bool AEnemyBase::IsDead() const
{
    return bIsDead;
}

bool AEnemyBase::IsAttacking() const
{
    return bIsAttacking;
}

bool AEnemyBase::IsHitReacting() const
{
    return bIsHitReacting;
}

void AEnemyBase::SetCurrentTarget(AActor* Target)
{
    CurrentTarget = Target;
}

AActor* AEnemyBase::GetCurrentTarget() const
{
    return CurrentTarget.Get();
}

bool AEnemyBase::CanAttack(AActor* Target) const
{
    if (bIsDead || bIsAttacking || bIsHitReacting || Target == nullptr)
    {
        return false;
    }

    const float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    const bool bIsCooldownReady = (CurrentTime - LastAttackTime) >= AttackCooldown;

    return DistanceToTarget <= AttackRange && bIsCooldownReady;
}

void AEnemyBase::DoAttack(AActor* Target)
{
    if (!CanAttack(Target))
    {
        return;
    }

    bIsAttacking = true;
    LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    if (Target)
    {
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    }

    if (UEnemyAnimInstance* EnemyAnimInstance = GetMesh() ? Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
    {
        const float Duration = EnemyAnimInstance->PlayAttackMontage();
        if (Duration > 0.f)
        {
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AEnemyBase::ResetAttackState, Duration, false);
            return;
        }
    }

    ResetAttackState();
}

void AEnemyBase::HandleDeath()
{
    bIsHitReacting = false;
    bIsAttacking = false;

    GetWorldTimerManager().ClearTimer(AttackTimerHandle);
    GetWorldTimerManager().ClearTimer(HitReactTimerHandle);

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
    if (CharacterMovementComp)
    {
        CharacterMovementComp->StopMovementImmediately();
        CharacterMovementComp->DisableMovement();
    }

    if (HPBarWidgetComponent)
    {
        HPBarWidgetComponent->SetVisibility(false);
    }

    DetachFromControllerPendingDestroy();
}

void AEnemyBase::HandleHitReaction()
{
    ResetHitReaction();
}

void AEnemyBase::ResetAttackState()
{
    bIsAttacking = false;
}

void AEnemyBase::ResetHitReaction()
{
    bIsHitReacting = false;
}

void AEnemyBase::UpdateHPBar()
{
    if (!HPBarWidgetComponent)
    {
        return;
    }

    if (UUserWidget* Widget = HPBarWidgetComponent->GetWidget())
    {
        if (UEnemyHPBarWidget* HPBar = Cast<UEnemyHPBarWidget>(Widget))
        {
            HPBar->SetHealth(CurrentHP, MaxHP);
        }
    }
}

void AEnemyBase::SpawnFloatingDamage(float Damage)
{
    if (!FloatingDamageActorClass)
    {
        return;
    }

    FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AFloatingDamageActor* DamageActor = GetWorld()->SpawnActor<AFloatingDamageActor>(FloatingDamageActorClass, SpawnLocation, FRotator::ZeroRotator, Params))
    {
        DamageActor->SetDamage(Damage);
    }
}