#include "EnemyBase.h"
#include "../UI/EnemyHPBarWidget.h"
#include "../UI/FloatingDamageActor.h"
#include "EnemyAnimInstance.h"
#include "../Character/SoulCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
    HPBarWidgetComponent->SetupAttachment(GetCapsuleComponent());
    HPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    HPBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    HPBarWidgetComponent->SetWidgetClass(UEnemyHPBarWidget::StaticClass());
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();

    TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
    GetComponents(PrimitiveComponents);
    for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
    {
        if (!PrimitiveComponent)
        {
            continue;
        }

        if (PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            continue;
        }

        PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    CurrentHP = MaxHP;

    UpdateHPBar();

    if (HPBarWidgetComponent)
    {
        HPBarWidgetComponent->SetVisibility(false);
    }

    if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
    {
        CharacterMovementComponent->MaxWalkSpeed = MovementSpeed;
    }

    if (UEnemyAnimInstance* EnemyAnim = GetMesh() ? Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
    {
        EnemyAnimInstance = EnemyAnim;
        EnemyAnimInstance->OnAttackHitCheck.AddUObject(this, &AEnemyBase::AttackHitCheck);
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

    AActor* Causer = DamageCauser;

    if (!IsValid(Causer) && EventInstigator)
    {
        Causer = EventInstigator->GetPawn();
    }

    APawn* CauserPawn = Cast<APawn>(Causer);
    if (!CauserPawn && EventInstigator)
    {
        CauserPawn = EventInstigator->GetPawn();
    }

    if (IsValid(CauserPawn) && CauserPawn->IsPlayerControlled())
    {
        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
            {
                static const FName TargetKey(TEXT("Target"));
                BB->SetValueAsObject(TargetKey, CauserPawn);
            }
        }
    }

    CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0.f, MaxHP);

    SpawnFloatingDamage(FinalDamage);

    if (HPBarWidgetComponent)
    {
        HPBarWidgetComponent->SetVisibility(true);
    }

    UpdateHPBar();

    if (CurrentHP <= 0.f)
    {
        HandleDeath();
    }
    else
    {
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

    if (!BeginAttackState())
    {
        return;
    }

    CurrentTarget = Target;
    LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    if (UEnemyAnimInstance* EnemyAnim = EnemyAnimInstance ? EnemyAnimInstance.Get() : (GetMesh() ? Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr))
    {
        const float Duration = EnemyAnim->PlayAttackMontage();
        if (Duration > 0.f)
        {
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AEnemyBase::ResetAttackState, Duration, false);
            return;
        }
    }

    ResetAttackState();
}

bool AEnemyBase::TryExecuteAttack(AActor* Target)
{
    if (!CanAttack(Target))
    {
        return false;
    }

    DoAttack(Target);
    return true;
}

bool AEnemyBase::BeginAttackState()
{
    if (bIsDead || bIsAttacking || bIsHitReacting)
    {
        return false;
    }

    bIsAttacking = true;

    if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
    {
        CachedMovementMode = CharacterMovementComp->MovementMode;
        CharacterMovementComp->StopMovementImmediately();
        CharacterMovementComp->DisableMovement();
        bMovementPausedForAttack = true;
    }

    return true;
}

void AEnemyBase::HandleDeath()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    bIsHitReacting = false;
    bIsAttacking = false;
    bMovementPausedForAttack = false;

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

    if (UEnemyAnimInstance* EnemyAnim = EnemyAnimInstance ? EnemyAnimInstance.Get() : (GetMesh() ? Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr))
    {
        EnemyAnim->PlayDeathMontage();
    }

    DetachFromControllerPendingDestroy();

    SetLifeSpan(3.f);
}

void AEnemyBase::HandleHitReaction()
{
    if (bIsDead)
    {
        return;
    }

    bIsHitReacting = true;

    if (UEnemyAnimInstance* EnemyAnim = GetMesh() ? Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
    {
        const float Duration = EnemyAnim->PlayHitReactMontage();
        const float RecoveryTime = (Duration > 0.f) ? Duration : 0.3f;
        GetWorldTimerManager().SetTimer(HitReactTimerHandle, this, &AEnemyBase::ResetHitReaction, RecoveryTime, false);
        return;
    }

    GetWorldTimerManager().SetTimer(HitReactTimerHandle, this, &AEnemyBase::ResetHitReaction, 0.3f, false);
}

void AEnemyBase::ResetAttackState()
{
    bIsAttacking = false;

    if (bMovementPausedForAttack && !bIsDead)
    {
        if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
        {
            CharacterMovementComp->SetMovementMode(CachedMovementMode);
        }
    }

    bMovementPausedForAttack = false;
}

void AEnemyBase::ResetHitReaction()
{
    bIsHitReacting = false;

    GetWorldTimerManager().ClearTimer(HitReactTimerHandle);
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

void AEnemyBase::AttackHitCheck()
{
    if (bIsDead || !bIsAttacking)
    {
        return;
    }

    AActor* Target = CurrentTarget.Get();
    if (!Target)
    {
        return;
    }

    if (!Cast<ASoulCharacter>(Target))
    {
        return;
    }

    const float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistanceToTarget > AttackRange)
    {
        return;
    }

    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
}