#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SoulCharacter.h"
#include "SoulAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnNextAttackCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAttackHitCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunShotDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunCanReShotDelegate);
DECLARE_MULTICAST_DELEGATE(FOnGunShotEndDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeIFrameOnDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeIFrameOffDelegate);
DECLARE_MULTICAST_DELEGATE(FOnDodgeEndDelegate);
DECLARE_MULTICAST_DELEGATE(FOnLadderTopMountEnd);
DECLARE_MULTICAST_DELEGATE(FOnLadderTopExitEnd);

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

	void PlayOpenBoxMontage();
	void PlayOpenDoorMontage();

	void PlayLadderTopMountMontage();
	void PlayLadderTopExitMontage();

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

	UFUNCTION()
	void AnimNotify_LadderTopMountEnd();

	UFUNCTION()
	void AnimNotify_LadderTopExitEnd();

public:
	FOnNextAttackCheckDelegate OnNextAttackCheck;
	FOnAttackHitCheckDelegate OnAttackHitCheck;
	FOnGunShotDelegate OnGunShot;
	FOnGunCanReShotDelegate OnGunCanReShot;
	FOnGunShotEndDelegate  OnGunShotEnd;
	FOnDodgeIFrameOnDelegate  OnDodgeIFrameOn;
	FOnDodgeIFrameOffDelegate OnDodgeIFrameOff;
	FOnDodgeEndDelegate OnDodgeEnd;
	FOnLadderTopMountEnd OnLadderTopMountEnd;
	FOnLadderTopExitEnd OnLadderTopExitEnd;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> GunFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> OpenBoxMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> OpenDoorMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> LadderTopMountMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> LadderTopExitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	bool IsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	float Speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	float Direction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	EWeaponType CurrentWeaponType = EWeaponType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	bool bIsDead = false;

	TWeakObjectPtr<class ASoulCharacter> CachedCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AimPitch = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn", Meta = (AllowPrivateAccess = true))
	bool bIsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	bool bOnLadder = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", Meta = (AllowPrivateAccess = true))
	float LadderSpeed = 0;
};