#include "SoulCharacter.h"
#include "SoulAnimInstance.h"
#include "../Game/SoulPlayerController.h"
#include "SoulCharacterStatComponent.h"
#include "../UI/FloatingDamageActor.h"
#include "../Interact/SoulInteractableInterface.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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

	UpdateMovementSpeed();

	StatComp = CreateDefaultSubobject<USoulCharacterStatComponent>(TEXT("StatComponent"));
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

	AnimInstance->OnDodgeIFrameOn.AddUObject(this, &ASoulCharacter::StartDodgeInvincible);
	AnimInstance->OnDodgeIFrameOff.AddUObject(this, &ASoulCharacter::EndDodgeInvincible);
	AnimInstance->OnDodgeEnd.AddUObject(this, &ASoulCharacter::OnDodgeFinished);
	
	if (StatComp)
	{
		StatComp->OnDead.AddDynamic(this, &ASoulCharacter::HandleDead);
	}
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
		
		EnhancedInputComponent->BindAction(SwordDodgeAction, ETriggerEvent::Started, this, &ASoulCharacter::Dodge);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASoulCharacter::Interact);
	}
}

void ASoulCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsAiming && GetCharacterMovement() && !GetCharacterMovement()->IsMovingOnGround())
	{
		StopAiming();
	}

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

	if (bAutoFacing)
	{
		UpdateAutoFace(DeltaSeconds);
	}
}

float ASoulCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDodgeInvincible)
	{
		return 0;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = (ActualDamage > 0.f) ? ActualDamage : DamageAmount;

	float AppliedDamage = 0.f;

	if (StatComp)
	{
		if (FinalDamage > 0.f && StatComp->ApplyDamage(FinalDamage))
		{
			OnHitDamage();
			SpawnDamageText(this, FinalDamage);
			AppliedDamage = FinalDamage;
		}

		UE_LOG(LogTemp, Log, TEXT("TakeDamage | Damage: %.2f | HP: %.2f / %.2f"), FinalDamage, StatComp->HP, StatComp->MaxHP);
	}

	return AppliedDamage;
}

void ASoulCharacter::Reset()
{
	Super::Reset();

	if (StatComp)
	{
		StatComp->ResetCurrentToMax();
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

void ASoulCharacter::UpdateMovementSpeed()
{
	if (GetCharacterMovement() == nullptr)
	{
		return;
	}

	float TargetSpeed = EmptyWalkSpeed;

	if (bIsAiming && CurrentWeaponType == EWeaponType::Gun)
	{
		TargetSpeed = GunAimWalkSpeed;
	}
	else
	{
		switch (CurrentWeaponType)
		{
		case EWeaponType::Sword:
			TargetSpeed = bIsSprinting ? SwordSprintSpeed : SwordWalkSpeed;
			break;

		case EWeaponType::Gun:
			TargetSpeed = GunWalkSpeed;
			break;

		case EWeaponType::Empty:
		default:
			TargetSpeed = bIsSprinting ? EmptySprintSpeed : EmptyWalkSpeed;
			break;
		}
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ASoulCharacter::SprintStart(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	bIsSprinting = true;
	bIsAiming = false;
	UpdateMovementSpeed();
}

void ASoulCharacter::SprintStop(const FInputActionValue& Value)
{
	if (GetController() == nullptr)
	{
		return;
	}

	bIsSprinting = false;
	UpdateMovementSpeed();
}

void ASoulCharacter::SwapSword(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	StopAiming();

	CurrentWeaponType = EWeaponType::Sword;
	bIsAiming = false;
	UpdateMovementSpeed();
}

void ASoulCharacter::SwapGun(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	CurrentWeaponType = EWeaponType::Gun;
	bIsSprinting = false;
	UpdateMovementSpeed();
}

void ASoulCharacter::SwapEmpty(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	StopAiming();

	CurrentWeaponType = EWeaponType::Empty;
	UpdateMovementSpeed();
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
	UpdateMovementSpeed();

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
	StopAiming();
}

void ASoulCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	bIsAiming = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	UpdateMovementSpeed();

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

	if (!bIsAiming)
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
	if (!FollowCamera)
	{
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector Direction = FollowCamera->GetForwardVector();
	const FVector End = Start + Direction * GunRange;

	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ObjectParams, Params);

#if ENABLE_DRAW_DEBUG
	const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), Start, End, TraceColor, false, 1.0f, 0, 1.0f);
#endif

	if (bHit)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (HitActor->IsA<ACharacter>())
			{
				UGameplayStatics::ApplyPointDamage(HitActor, GunDamage, Direction, HitResult, GetController(), this, nullptr);

				UE_LOG(LogTemp, Warning, TEXT("Gun hit actor: %s"), *HitActor->GetName());
			}
		}
	}

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

	bool bResult = GetWorld()->SweepSingleByChannel(HitResult, GetActorLocation(), GetActorLocation() + GetActorForwardVector() * SwordAttackRange, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel2, FCollisionShape::MakeSphere(SwordAttackRadius), Params);

#if ENABLE_DRAW_DEBUG

	FVector TraceVec = GetActorForwardVector() * SwordAttackRange;
	FVector Center = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight = SwordAttackRange * 0.5f + SwordAttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(), Center, HalfHeight, SwordAttackRadius, CapsuleRot, DrawColor, false, DebugLifeTime);

#endif

	if (bResult)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, SwordDamage, GetActorForwardVector(), HitResult, GetController(), this, nullptr);
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name : %s"), *HitActor->GetName());
		}
	}
}

void ASoulCharacter::Dodge(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	if (CurrentWeaponType == EWeaponType::Gun)
	{
		return;
	}

	if (bIsDodging)
	{
		return;
	}

	if (bIsAttacking)
	{
		return;
	}

	bIsDodging = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = false;
	MoveComp->StopMovementImmediately();

	FVector DodgeDir = MoveComp->GetLastInputVector();
	DodgeDir.Z = 0.f;
	DodgeDir = DodgeDir.GetSafeNormal();

	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = GetActorForwardVector();
		DodgeDir.Z = 0.f;
		DodgeDir.Normalize();
	}

	SetActorRotation(DodgeDir.Rotation());

	LaunchCharacter(DodgeDir * DodgeStrength, true, true);

	if (AnimInstance)
	{
		AnimInstance->PlayDodgeMontage();
	}
}

void ASoulCharacter::OnDodgeFinished()
{
	bIsDodging = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ASoulCharacter::StartDodgeInvincible()
{
	bDodgeInvincible = true;
}

void ASoulCharacter::EndDodgeInvincible()
{
	bDodgeInvincible = false;
}

void ASoulCharacter::HandleDead()
{
	if (bIsDead) return;
	bIsDead = true;

	GetCharacterMovement()->DisableMovement();

	bIsAttacking = false;
	bIsSprinting = false;
	bIsAiming = false;
	UpdateMovementSpeed();

	SetActorEnableCollision(false);
}

void ASoulCharacter::OnHitDamage()
{
	if (bIsDead)
	{
		return;
	}

	if (AnimInstance)
	{
		AnimInstance->PlayHitReactMontage();
	}

	bIsHit = true;

	GetWorldTimerManager().SetTimer(HitRecoveryTimer, [this]() {
		bIsHit = false;
		}, 0.3f, false);
}

void ASoulCharacter::SpawnDamageText(AActor* DamagedActor, float Damage)
{
	if (!DamageTextActorClass || !DamagedActor) return;

	FVector TargetLocation = DamagedActor->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFloatingDamageActor* DamageText = GetWorld()->SpawnActor<AFloatingDamageActor>(DamageTextActorClass, TargetLocation, FRotator::ZeroRotator, Params);
	if (DamageText)
	{
		DamageText->SetDamage(Damage);
	}
}

void ASoulCharacter::Interact(const FInputActionValue& Value)
{
	if (!Value.Get<bool>()) return;

	if (!CurrentInteractTarget.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Interact: No current target"));
		return;
	}

	AActor* Target = CurrentInteractTarget.Get();

	UE_LOG(LogTemp, Log, TEXT("Interact: %s"), *Target->GetName());
	ISoulInteractableInterface::Execute_Interact(Target, this);
}

void ASoulCharacter::SetInteractTarget(AActor* NewTarget)
{
	if (!NewTarget) return;

	if (!NewTarget->GetClass()->ImplementsInterface(USoulInteractableInterface::StaticClass()))
		return;

	CurrentInteractTarget = NewTarget;

	UE_LOG(LogTemp, Log, TEXT("Interact target set: %s"), *NewTarget->GetName());
}

void ASoulCharacter::ClearInteractTarget(AActor* Target)
{
	if (CurrentInteractTarget.Get() == Target)
	{
		UE_LOG(LogTemp, Log, TEXT("Interact target cleared: %s"), *Target->GetName());
		CurrentInteractTarget = nullptr;
	}
}

void ASoulCharacter::FaceToActor(const AActor* Target)
{
	if (!Target) return;

	StartAutoFace(Target);
}

void ASoulCharacter::StartAutoFace(const AActor* Target)
{
	if (!Target) return;

	AutoFaceTarget = Target;
	bAutoFacing = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		bPrevOrientToMove = MoveComp->bOrientRotationToMovement;
		bPrevUseControllerDesired = MoveComp->bUseControllerDesiredRotation;

		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;

		MoveComp->StopMovementImmediately();
	}
}

void ASoulCharacter::StopAutoFace()
{
	bAutoFacing = false;
	AutoFaceTarget = nullptr;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = bPrevOrientToMove;
		MoveComp->bUseControllerDesiredRotation = bPrevUseControllerDesired;
	}
}

void ASoulCharacter::UpdateAutoFace(float DeltaSeconds)
{
	if (!AutoFaceTarget.IsValid())
	{
		StopAutoFace();
		return;
	}

	FVector ToTarget = AutoFaceTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		StopAutoFace();
		return;
	}

	const FRotator CurrentRot = GetActorRotation();
	const FRotator TargetRot = ToTarget.Rotation();

	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, AutoFaceInterpSpeed);

	const float YawDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, TargetRot.Yaw));
	if (YawDiff <= AutoFaceSnapDeg)
	{
		NewRot.Yaw = TargetRot.Yaw;
		SetActorRotation(NewRot);
		StopAutoFace();
		return;
	}

	SetActorRotation(NewRot);
}