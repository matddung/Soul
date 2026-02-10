#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "../Common/WeaponTypes.h"
#include "SoulInventoryComponent.h"
#include "SoulCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USoulCharacterStatComponent;
class ASoulLadderActor;
class USoulWeaponComponent;
class USoulWeaponData;
class USoulInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAutoFaceEndDelegate);

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Ladder	UMETA(DisplayName = "Ladder")
};

UENUM(BlueprintType)
enum class ECharacterActionState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Attack		UMETA(DisplayName = "Attack"),
	Dodge		UMETA(DisplayName = "Dodge"),
	Hit			UMETA(DisplayName = "Hit"),
	Sprinting	UMETA(DisplayName = "Sprinting"),
	Aiming		UMETA(DisplayName = "Aiming"),
	Dead		UMETA(DisplayName = "Dead"),
	OnLadder	UMETA(DisplayName = "OnLadder")
};

UCLASS()
class SOUL_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	friend class ASoulPlayerController;

	ASoulCharacter();

	FORCEINLINE bool GetIsSprinting() const { return ActionState == ECharacterActionState::Sprinting; }
	FORCEINLINE bool GetIsAttacking() const { return ActionState == ECharacterActionState::Attack; }
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }
	FORCEINLINE bool GetIsAiming() const { return ActionState == ECharacterActionState::Aiming; }
	FORCEINLINE bool GetIsDead() const { return ActionState == ECharacterActionState::Dead; }
	FORCEINLINE bool GetIsHit() const { return ActionState == ECharacterActionState::Hit; }
	FORCEINLINE bool GetIsDodging() const { return ActionState == ECharacterActionState::Dodge; }
	FORCEINLINE ECharacterActionState GetActionState() const { return ActionState; }
	FORCEINLINE bool IsOnLadder() const { return ActionState == ECharacterActionState::OnLadder; }
	FORCEINLINE float GetLadderInput() const { return LadderInput; }
	FORCEINLINE int32 GetMaxGunShots() const { return MaxGunShots; }
	FORCEINLINE int32 GetRemainingGunShots() const { return RemainingGunShots; }

	void SetInteractTarget(AActor* NewTarget);
	void ClearInteractTarget(AActor* Target);

	void FaceToActor(const AActor* Target);

	void PlayOpenBoxAnim();
	void SetWeaponType(EWeaponType NewType);

	void PlayOpenDoorAnim();

	void BeginLadder(ASoulLadderActor* Ladder);
	void EndLadder();

	void GiveGunFromBox(bool bAutoEquip = false);
	bool EnhanceWeapon(EWeaponType Type);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Reset() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void Landed(const FHitResult& Hit) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintStop(const FInputActionValue& Value);
	bool IsGrounded() const;
	void Attack(const FInputActionValue& Value);
	void SwapSword(const FInputActionValue& Value);
	void SwapGun(const FInputActionValue& Value);
	void SwapEmpty(const FInputActionValue& Value);
	void GunAimStart(const FInputActionValue& Value);
	void GunAimStop(const FInputActionValue& Value);
	void StopAiming();
	void HandleSwordAttack();
	void HandleGunAttack();
	void DoGunShot();
	void OnGunCanReShot();
	void OnGunShotEnd();
	void UpdateMovementSpeed();
	bool IsAnimationBlockingActions() const;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void AttackStartComboState();
	void AttackEndComboState();
	void AttackCheck();

	void Dodge(const FInputActionValue& Value);
	void StartDodgeInvincible();
	void EndDodgeInvincible();
	void OnDodgeFinished();

	UFUNCTION()
	void HandleDead();

	void OnHitDamage(bool bUseKnockback);

	void SpawnDamageText(AActor* DamagedActor, float Damage);

	void Interact(const FInputActionValue& Value);

	void StartAutoFace(const AActor* Target);
	void UpdateAutoFace(float DeltaSeconds);
	void StopAutoFace();

	void EnterLadderMode();
	void ExitLadderMode();
	void UpdateLadder(float DeltaSeconds);

	void MoveCompleted(const FInputActionValue& Value);

	void UpdateTopMountMove(float DeltaSeconds);

	void RestoreWeaponOwnershipFromSave();
	void SaveWeaponOwnership(bool bHasGunOwned);

	void HandleFallStart();
	void HandleLandingDamage();

	void ResetGunShots();
	void NotifyGunAmmoChanged();

	void UpdateStamina(float DeltaSeconds);
	bool TryConsumeStamina(float Amount);

public:
	FOnAutoFaceEndDelegate OnAutoFaceEnd;

protected:
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float EmptyWalkSpeed = 400;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float EmptySprintSpeed = 800;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SwordWalkSpeed = 200;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SwordSprintSpeed = 400;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GunWalkSpeed = 200;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GunAimWalkSpeed = 50;

	UPROPERTY()
	TObjectPtr<class USoulAnimInstance> AnimInstance;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	ECharacterActionState ActionState = ECharacterActionState::Idle;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool CanNextCombo;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	int32 MaxCombo = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType = EWeaponType::Empty;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bCanGunFire = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Camera")
	float DefaultFOV = 90;

	UPROPERTY(VisibleInstanceOnly, Category = "Camera")
	float AimFOV = 70;

	UPROPERTY(VisibleInstanceOnly, Category = "Camera")
	float FOVInterpSpeed = 20;

	UPROPERTY(VisibleInstanceOnly, Category = "Camera")
	float DefaultArmLength = 400;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float AimArmLength = 150;

	UPROPERTY(VisibleInstanceOnly, Category = "Camera")
	FVector DefaultSocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector AimSocketOffset = FVector(0, 50, 40);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraInterpSpeed = 10;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	float SwordAttackRange = 130;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	float SwordAttackRadius = 50;

	UPROPERTY(EditAnywhere, Category = "Weapon|Gun")
	float GunRange = 2000;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeStrength = 400;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bDodgeInvincible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USoulCharacterStatComponent> StatComp;

	FTimerHandle HitRecoveryTimer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class AFloatingDamageActor> DamageTextActorClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Interact")
	TWeakObjectPtr<AActor> CurrentInteractTarget;

	UPROPERTY(VisibleInstanceOnly, Category = "Interact")
	bool bAutoFacing = false;

	UPROPERTY()
	TWeakObjectPtr<const AActor> AutoFaceTarget;

	UPROPERTY(EditAnywhere, Category = "Interact")
	float AutoFaceInterpSpeed = 5;

	UPROPERTY(EditAnywhere, Category = "Interact")
	float AutoFaceSnapDeg = 2;

	bool bPrevOrientToMove = false;
	bool bPrevUseControllerDesired = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Movement")
	ELocomotionState LocomotionState = ELocomotionState::Normal;

	UPROPERTY()
	TWeakObjectPtr<ASoulLadderActor> CurrentLadder;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	float LadderInput = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ladder")
	float LadderMoveSpeed = 90;

	UPROPERTY(EditDefaultsOnly, Category = "Ladder")
	float LadderAlignInterpSpeed = 12;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	bool bLadderMounting = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	bool bTopMountMoving = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	FVector TopMountTargetLoc;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	FRotator TopMountTargetRot;

	UPROPERTY(EditDefaultsOnly, Category = "Ladder")
	float TopMountMoveTime = 1;

	float TopMountMoveElapsed = 0;
	FVector TopMountStartLoc;
	FRotator TopMountStartRot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoulWeaponComponent> WeaponComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USoulWeaponData> DefaultSwordData;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<class USoulWeaponData> DefaultGunData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoulInventoryComponent> InventoryComp;

	UPROPERTY(VisibleInstanceOnly, Category = "Movement")
	float FallStartZ = 0;

	UPROPERTY(EditAnywhere, Category = "Weapon|Gun")
	int32 MaxGunShots = 30;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Gun")
	int32 RemainingGunShots = 0;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float SprintStaminaCostPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float SwordAttackStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float DodgeStaminaCost = 25.0f;
};