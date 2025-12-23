#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SoulCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class USoulCharacterStatComponent;
class ASoulLadderActor;

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAutoFaceEndDelegate);

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Empty	UMETA(DisplayName = "Empty"),
	Sword	UMETA(DisplayName = "Sword"),
	Gun		UMETA(DisplayName = "Gun")
};

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Ladder	UMETA(DisplayName = "Ladder")
};

UCLASS()
class SOUL_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASoulCharacter();

	FORCEINLINE bool GetIsSprinting() const { return bIsSprinting; }
	FORCEINLINE bool GetIsAttacking() const { return bIsAttacking; }
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }
	FORCEINLINE bool GetIsDead() const { return bIsDead; }
	FORCEINLINE bool GetIsHit() const { return bIsHit; }
	FORCEINLINE bool IsOnLadder() const { return LocomotionState == ELocomotionState::Ladder; }
	FORCEINLINE float GetLadderInput() const { return LadderInput; }

	void SetInteractTarget(AActor* NewTarget);
	void ClearInteractTarget(AActor* Target);

	void FaceToActor(const AActor* Target);

	void PlayOpenBoxAnim();
	void SetWeaponType(EWeaponType NewType);

	void PlayOpenDoorAnim();

	void BeginLadder(ASoulLadderActor* Ladder);
	void EndLadder();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Reset() override;

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

	void OnHitDamage();

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

public:
	FOnAutoFaceEndDelegate OnAutoFaceEnd;

protected:
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(EditAnywhere,Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwapSwordAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwapGunAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwapEmptyAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> GunAimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwordDodgeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsSprinting = false;

	UPROPERTY()
	TObjectPtr<class USoulAnimInstance> AnimInstance;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bIsAttacking;

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

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bIsAiming = false;

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
	float SwordAttackRange = 200;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	float SwordAttackRadius = 50;

	UPROPERTY(EditAnywhere, Category = "Weapon|Sword")
	float SwordDamage = 20;

	UPROPERTY(EditAnywhere, Category = "Weapon|Gun")
	float GunDamage = 15;

	UPROPERTY(EditAnywhere, Category = "Weapon|Gun")
	float GunRange = 2000;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsDodging = false;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeStrength = 400;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bDodgeInvincible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USoulCharacterStatComponent> StatComp;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsHit = false;

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
};