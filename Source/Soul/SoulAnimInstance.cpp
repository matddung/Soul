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

	if (Pawn)
	{
		FVector Velocity = Pawn->GetVelocity();

		Speed = Velocity.Size();

		FRotator Rotation = Pawn->GetActorRotation();
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
	}

	ASoulCharacter* Character = Cast<ASoulCharacter>(Pawn);
	if (Character) {
		IsInAir = Character->GetMovementComponent()->IsFalling();
	}

	CurrentWeaponType = Character->GetCurrentWeaponType();
}

void USoulAnimInstance::PlayAttackMontage()
{
	Montage_Play(AttackMontage, 1);
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