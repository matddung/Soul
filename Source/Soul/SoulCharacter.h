#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SoulCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Empty	UMETA(DisplayName = "Empty"),
	Sword	UMETA(DisplayName = "Sword"),
	Gun		UMETA(DisplayName = "Gun")
};

UCLASS()
class SOUL_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASoulCharacter();

	FORCEINLINE bool GetIsSprinting() { return bIsSprinting; }
	FORCEINLINE bool GetIsAttacking() { return bIsAttacking; }
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintStop(const FInputActionValue& Value);
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
	void TickDodgeMove();

protected:
	FOnAttackEndDelegate OnAttackEnd;

	UPROPERTY(EditAnywhere,Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwapSwordAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwapGunAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwapEmptyAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GunAimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwordDodgeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Movement|Empty")
	float EmptyWalkSpeed = 400;

	UPROPERTY(EditAnywhere, Category = "Movement|Empty")
	float EmptySprintSpeed = 800;

	UPROPERTY(EditAnywhere, Category = "Movement|Sword")
	float SwordWalkSpeed = 200;

	UPROPERTY(EditAnywhere, Category = "Movement|Sword")
	float SwordSprintSpeed = 400;

	UPROPERTY(EditAnywhere, Category = "Movement|Gun")
	float GunWalkSpeed = 200;

	UPROPERTY(EditAnywhere, Category = "Movement|Gun")
	float GunAimWalkSpeed = 50;

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bIsSprinting = false;

	UPROPERTY()
	class USoulAnimInstance* AnimInstance;

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
	FVector AimSocketOffset = FVector(0.f, 50.f, 40.f);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraInterpSpeed = 10.f;

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

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bIsDodging = false;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeStrength = 400;

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bDodgeInvincible = false;
};
