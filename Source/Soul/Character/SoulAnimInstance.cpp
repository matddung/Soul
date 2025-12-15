#include "SoulAnimInstance.h"
#include "SoulCharacter.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void USoulAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!::IsValid(Pawn)) return;

	CachedCharacter = Cast<ASoulCharacter>(Pawn);

	FVector Velocity = Pawn->GetVelocity();
	Speed = Velocity.Size();

	FRotator Rotation = Pawn->GetActorRotation();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);

	if (!CachedCharacter.IsValid())
	{
		IsInAir = false;
		CurrentWeaponType = EWeaponType::Empty;
		return;
	}

	IsInAir = CachedCharacter->GetMovementComponent()->IsFalling();
	CurrentWeaponType = CachedCharacter->GetCurrentWeaponType();
	bIsAiming = CachedCharacter->GetIsAiming();

	const FRotator AimRot = CachedCharacter->GetBaseAimRotation();
	const FRotator ActorRot = CachedCharacter->GetActorRotation();

	FRotator DeltaRot = (AimRot - ActorRot).GetNormalized();
	AimPitch = DeltaRot.Pitch;
}

bool USoulAnimInstance::IsAttacking() const
{
	if (!CachedCharacter.IsValid())
	{
		return false;
	}

	return CachedCharacter->GetIsAttacking();
}

bool USoulAnimInstance::IsSprinting() const
{
	if (!CachedCharacter.IsValid())
	{
		return false;
	}

	return CachedCharacter->GetIsSprinting();
}

ECharacterAnimState USoulAnimInstance::GetCharacterState() const
{
	if (IsAttacking())
	{
		return ECharacterAnimState::Attacking;
	}

	const bool bHasSpeed = Speed > KINDA_SMALL_NUMBER;
	if (bHasSpeed)
	{
		return IsSprinting() ? ECharacterAnimState::Sprinting : ECharacterAnimState::Walking;
	}

	return ECharacterAnimState::Idle;
}

void USoulAnimInstance::PlaySwordAttackMontage()
{
	Montage_Play(AttackMontage, 1);
}

void USoulAnimInstance::PlayGunAttackMontage()
{
	Montage_Play(GunFireMontage, 1);
}

void USoulAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
	Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void USoulAnimInstance::AnimNotify_AttackHitCheck()
{
	OnAttackHitCheck.Broadcast();
}

void USoulAnimInstance::AnimNotify_NextAttackCheck()
{
	OnNextAttackCheck.Broadcast();
}

FName USoulAnimInstance::GetAttackMontageSectionName(int32 Section)
{
	if (!ensureMsgf(FMath::IsWithinInclusive<int32>(Section, 1, 4), TEXT("GetAttackMontageSectionName error: Section %d is out of range (1~4)"), Section))
	{
		return NAME_None;
	}
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}

void USoulAnimInstance::AnimNotify_GunShot()
{
	OnGunShot.Broadcast();
}

void USoulAnimInstance::AnimNotify_GunCanReShot()
{
	OnGunCanReShot.Broadcast();
}

void USoulAnimInstance::AnimNotify_GunShotEnd()
{
	OnGunShotEnd.Broadcast();
}

void USoulAnimInstance::PlayDodgeMontage()
{
	Montage_Play(DodgeMontage, 1);
}

void USoulAnimInstance::AnimNotify_DodgeIFrameOn()
{
	OnDodgeIFrameOn.Broadcast();
}

void USoulAnimInstance::AnimNotify_DodgeIFrameOff()
{
	OnDodgeIFrameOff.Broadcast();
}

void USoulAnimInstance::AnimNotify_DodgeEnd()
{
	OnDodgeEnd.Broadcast();
}