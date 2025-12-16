#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SoulCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Templates/SharedPointer.h"
#include "SoulAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnNextAttackCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAttackHitCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunShotDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunCanReShotDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunShotEndDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeIFrameOnDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeIFrameOffDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeEndDelegate);

UENUM(BlueprintType)
enum class ECharacterAnimState : uint8
{
	Idle,
	Walking,
	Sprinting,
	Attacking,
	Dead
};

class ASoulCharacter;

UCLASS()
class SOUL_API USoulAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Pawn")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Pawn")
	bool IsSprinting() const;

	UFUNCTION(BlueprintPure, Category = "Pawn")
	ECharacterAnimState GetCharacterState() const;

	void PlaySwordAttackMontage();
	void PlayGunAttackMontage();
	void JumpToAttackMontageSection(int32 NewSection);

	void PlayDodgeMontage();

	void PlayHitReactMontage();

protected:
	UFUNCTION()
	void AnimNotify_AttackHitCheck();

	UFUNCTION()
	void AnimNotify_NextAttackCheck();

	FORCEINLINE FName GetAttackMontageSectionName(int32 Section);

	UFUNCTION()
	void AnimNotify_GunShot();

	UFUNCTION()
	void AnimNotify_GunCanReShot();

	UFUNCTION()
	void AnimNotify_GunShotEnd();

	UFUNCTION()
	void AnimNotify_DodgeIFrameOn();

	UFUNCTION()
	void AnimNotify_DodgeIFrameOff();

	UFUNCTION()
	void AnimNotify_DodgeEnd();

public:
	FOnNextAttackCheckDelegate OnNextAttackCheck;
	FOnAttackHitCheckDelegate OnAttackHitCheck;
	FOnGunShotDelegate OnGunShot;
	FOnGunCanReShotDelegate OnGunCanReShot;
	FOnGunShotEndDelegate  OnGunShotEnd;
	FOnDodgeIFrameOnDelegate  OnDodgeIFrameOn;
	FOnDodgeIFrameOffDelegate OnDodgeIFrameOff;
	FOnDodgeEndDelegate OnDodgeEnd;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	UAnimMontage* GunFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float Speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float Direction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	EWeaponType CurrentWeaponType = EWeaponType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool bIsDead = false;

	TWeakObjectPtr<class ASoulCharacter> CachedCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AimPitch = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool bIsHit = false;
};