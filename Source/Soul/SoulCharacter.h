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
	None	UMETA(DisplayName = "None"),
	Sword	UMETA(DisplayName = "Sword"),
	Gun		UMETA(DisplayName = "Gun")
};

UCLASS()
class SOUL_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASoulCharacter();

	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintStop(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void AttackStartComboState();
	void AttackEndComboState();
	void AttackCheck();

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
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed = 300;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed = 600;

	UPROPERTY(EditAnywhere, Category = "Movement")
	bool bIsSprinting = false;

	UPROPERTY()
	class USoulAnimInstance* AnimInstance;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	bool IsAttacking;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	bool CanNextCombo;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	int32 MaxCombo = 4;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	float AttackRange = 200;

	UPROPERTY(VisibleInstanceOnly, Category = "Attack")
	float AttackRadius = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType = EWeaponType::None;
};
