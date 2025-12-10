#include "SoulCharacter.h"
#include "SoulAnimInstance.h"
#include "SoulPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

ASoulCharacter::ASoulCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultArmLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ASoulCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (FollowCamera)
	{
		DefaultFOV = FollowCamera->FieldOfView;
	}

	if (CameraBoom)
	{
		DefaultArmLength = CameraBoom->TargetArmLength;
		DefaultSocketOffset = CameraBoom->SocketOffset;
	}
}

void ASoulCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AnimInstance = Cast<USoulAnimInstance>(GetMesh()->GetAnimInstance());

	if (AnimInstance == nullptr)
	{
		return;
	}

	AnimInstance->OnMontageEnded.AddDynamic(this, &ASoulCharacter::OnAttackMontageEnded);

	AnimInstance->OnNextAttackCheck.AddLambda([this]() -> void {
		CanNextCombo = true;

		if (IsComboInputOn)
		{
			AttackStartComboState();
			AnimInstance->JumpToAttackMontageSection(CurrentCombo);
		}
	});

	AnimInstance->OnGunShot.AddUObject(this, &ASoulCharacter::DoGunShot);
	AnimInstance->OnGunCanReShot.AddUObject(this, &ASoulCharacter::OnGunCanReShot);
	AnimInstance->OnGunShotEnd.AddUObject(this, &ASoulCharacter::OnGunShotEnd);

	AnimInstance->OnAttackHitCheck.AddUObject(this, &ASoulCharacter::AttackCheck);
}

void ASoulCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASoulCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASoulCharacter::SprintStop);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASoulCharacter::Attack);

		EnhancedInputComponent->BindAction(SwapSwordAction, ETriggerEvent::Started, this, &ASoulCharacter::SwapSword);
		EnhancedInputComponent->BindAction(SwapGunAction, ETriggerEvent::Started, this, &ASoulCharacter::SwapGun);
		EnhancedInputComponent->BindAction(SwapEmptyAction, ETriggerEvent::Started, this, &ASoulCharacter::SwapEmpty);
		EnhancedInputComponent->BindAction(GunAimAction, ETriggerEvent::Started, this, &ASoulCharacter::GunAimStart);
		EnhancedInputComponent->BindAction(GunAimAction, ETriggerEvent::Completed, this, &ASoulCharacter::GunAimStop);
	}
}

void ASoulCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FollowCamera)
	{
		const float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
		const float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, FOVInterpSpeed);
		FollowCamera->SetFieldOfView(NewFOV);
	}

	if (CameraBoom)
	{
		const float TargetArm = bIsAiming ? AimArmLength : DefaultArmLength;
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaSeconds, CameraInterpSpeed);

		const FVector TargetOffset = bIsAiming ? AimSocketOffset : DefaultSocketOffset;
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaSeconds, CameraInterpSpeed);
	}
}

void ASoulCharacter::Move(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASoulCharacter::Look(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ASoulCharacter::SprintStart(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ASoulCharacter::SprintStop(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASoulCharacter::SwapSword(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	CurrentWeaponType = EWeaponType::Sword;
}

void ASoulCharacter::SwapGun(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	CurrentWeaponType = EWeaponType::Gun;
}

void ASoulCharacter::SwapEmpty(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	CurrentWeaponType = EWeaponType::Empty;
}

void ASoulCharacter::GunAimStart(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	if (CurrentWeaponType != EWeaponType::Gun)
	{
		return;
	}

	bIsAiming = true;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto SoulPC = Cast<ASoulPlayerController>(PC))
		{
			SoulPC->ShowCrosshair(true);
		}
	}
}

void ASoulCharacter::GunAimStop(const FInputActionValue& Value)
{
	if (CurrentWeaponType != EWeaponType::Gun)
	{
		return;
	}

	bIsAiming = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto SoulPC = Cast<ASoulPlayerController>(PC))
		{
			SoulPC->ShowCrosshair(false);
		}
	}
}

void ASoulCharacter::Attack(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	switch (CurrentWeaponType)
	{
	case EWeaponType::Sword:
		HandleSwordAttack();
		break;

	case EWeaponType::Gun:
		HandleGunAttack();
		break;

	default:
		break;
	}
}

void ASoulCharacter::HandleSwordAttack()
{
	if (bIsAttacking)
	{
		if (!FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo))
		{
			return;
		}
		if (CanNextCombo)
		{
			IsComboInputOn = true;
		}
	}
	else
	{
		if (CurrentCombo != 0)
		{
			return;
		}
		AttackStartComboState();
		AnimInstance->PlaySwordAttackMontage();
		AnimInstance->JumpToAttackMontageSection(CurrentCombo);
		bIsAttacking = true;
	}
}

void ASoulCharacter::HandleGunAttack()
{
	if (!bCanGunFire)
	{
		return;
	}

	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->PlayGunAttackMontage();

	bCanGunFire = false;
}

void ASoulCharacter::DoGunShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Gun Shot!"));

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto SoulPC = Cast<ASoulPlayerController>(PC))
		{
			SoulPC->OnCrosshairShot();
		}
	}
}

void ASoulCharacter::OnGunCanReShot()
{
	bCanGunFire = true;
}

void ASoulCharacter::OnGunShotEnd()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto SoulPC = Cast<ASoulPlayerController>(PC))
		{
			SoulPC->OnCrosshairReset();
		}
	}
}

void ASoulCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bIsAttacking)
	{
		return;
	}
	if (CurrentCombo <= 0)
	{
		return;
	}
	bIsAttacking = false;
	AttackEndComboState();

	OnAttackEnd.Broadcast();
}

void ASoulCharacter::AttackStartComboState()
{
	CanNextCombo = true;
	IsComboInputOn = false;
	if (!FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1))
	{
		return;
	}
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void ASoulCharacter::AttackEndComboState()
{
	IsComboInputOn = false;
	CanNextCombo = false;
	CurrentCombo = 0;
}

void ASoulCharacter::AttackCheck()
{
	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * SwordAttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(SwordAttackRadius),
		Params);

#if ENABLE_DRAW_DEBUG

	FVector TraceVec = GetActorForwardVector() * SwordAttackRange;
	FVector Center = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight = SwordAttackRange * 0.5f + SwordAttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		SwordAttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime);

#endif

	if (bResult)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name : %s"), *HitActor->GetName());
		}
	}
}