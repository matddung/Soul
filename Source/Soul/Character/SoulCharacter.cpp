#include "SoulCharacter.h"
#include "SoulAnimInstance.h"
#include "../Game/SoulPlayerController.h"
#include "SoulCharacterStatComponent.h"
#include "../UI/FloatingDamageActor.h"
#include "../Interact/SoulInteractableInterface.h"
#include "../Interact/SoulLadderActor.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

#include "Kismet/GameplayStatics.h"

ASoulCharacter::ASoulCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42, 96);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 500, 0);

	GetCharacterMovement()->MinAnalogWalkSpeed = 20;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500;

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

	AnimInstance->OnLadderTopMountEnd.AddLambda([this]()
		{
			if (!CurrentLadder.IsValid())
			{
				return;
			}

			FVector SnapLoc; FRotator SnapRot;
			CurrentLadder->GetSnapTransform(this, SnapLoc, SnapRot);
			SetActorLocation(SnapLoc);
			SetActorRotation(SnapRot);

			EnterLadderMode();
			bLadderMounting = false;
		});

	AnimInstance->OnLadderTopExitEnd.AddLambda([this]()
		{
			if (!CurrentLadder.IsValid())
			{
				return;
			}

			const FVector ExitLoc = CurrentLadder->GetTopExitLocation();
			EndLadder();
			bLadderMounting = false;
		});
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

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASoulCharacter::MoveCompleted);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ASoulCharacter::MoveCompleted);
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

	if (LocomotionState == ELocomotionState::Ladder)
	{
		UpdateLadder(DeltaSeconds);
	}

	if (bTopMountMoving)
	{
		UpdateTopMountMove(DeltaSeconds);
		return;
	}
}

float ASoulCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDodgeInvincible)
	{
		return 0;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = (ActualDamage > 0) ? ActualDamage : DamageAmount;

	float AppliedDamage = 0;

	if (StatComp)
	{
		if (FinalDamage > 0 && StatComp->ApplyDamage(FinalDamage))
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

	if (LocomotionState == ELocomotionState::Ladder)
	{
		LadderInput = MovementVector.Y;
		return;
	}

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
	if (LocomotionState == ELocomotionState::Ladder)
	{
		return;
	}

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

bool ASoulCharacter::IsGrounded() const
{
	if (const UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		return MoveComp->IsMovingOnGround();
	}

	return false;
}

void ASoulCharacter::SwapSword(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	if (LocomotionState == ELocomotionState::Ladder)
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

	if (LocomotionState == ELocomotionState::Ladder)
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

	if (LocomotionState == ELocomotionState::Ladder)
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

	if (LocomotionState == ELocomotionState::Ladder)
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

	if (LocomotionState == ELocomotionState::Ladder)
	{
		return;
	}

	if (!IsGrounded())
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
	if (LocomotionState == ELocomotionState::Ladder)
	{
		return;
	}

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

	if (!IsGrounded())
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
	DrawDebugLine(GetWorld(), Start, End, TraceColor, false, 1, 0, 1);
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
	FVector Center = GetActorLocation() + TraceVec * 0.5;
	float HalfHeight = SwordAttackRange * 0.5 + SwordAttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5;

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

	if (LocomotionState == ELocomotionState::Ladder)
	{
		return;
	}

	if (!IsGrounded())
	{
		return;
	}

	bIsDodging = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = false;
	MoveComp->StopMovementImmediately();

	FVector DodgeDir = MoveComp->GetLastInputVector();
	DodgeDir.Z = 0;
	DodgeDir = DodgeDir.GetSafeNormal();

	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = GetActorForwardVector();
		DodgeDir.Z = 0;
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
	if (bIsDead)
	{
		return;
	}

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
		}, 0.3, false);
}

void ASoulCharacter::SpawnDamageText(AActor* DamagedActor, float Damage)
{
	if (!DamageTextActorClass || !DamagedActor)
	{
		return;
	}

	FVector TargetLocation = DamagedActor->GetActorLocation() + FVector(0, 0, 100);
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
	if (!Value.Get<bool>())
	{
		return;
	}

	if (!CurrentInteractTarget.IsValid())
	{
		return;
	}

	AActor* Target = CurrentInteractTarget.Get();

	ISoulInteractableInterface::Execute_Interact(Target, this);

	if (auto PC = Cast<ASoulPlayerController>(GetController()))
    {
        PC->ShowInteractPrompt(false, FText::GetEmpty());
    }
}

void ASoulCharacter::SetInteractTarget(AActor* NewTarget)
{
	if (!NewTarget)
	{
		return;
	}

	if (!NewTarget->GetClass()->ImplementsInterface(USoulInteractableInterface::StaticClass()))
	{
		return;
	}

	CurrentInteractTarget = NewTarget;

	const FText Text = ISoulInteractableInterface::Execute_GetInteractText(NewTarget);

	if (auto PC = Cast<ASoulPlayerController>(GetController()))
	{
		PC->ShowInteractPrompt(true, Text);
	}
}

void ASoulCharacter::ClearInteractTarget(AActor* Target)
{
	if (CurrentInteractTarget.Get() != Target)
	{
		return;
	}

	CurrentInteractTarget = nullptr;

	if (auto PC = Cast<ASoulPlayerController>(GetController()))
	{
		PC->ShowInteractPrompt(false, FText::GetEmpty());
	}
}

void ASoulCharacter::FaceToActor(const AActor* Target)
{
	if (!Target)
	{
		return;
	}

	StartAutoFace(Target);
}

void ASoulCharacter::StartAutoFace(const AActor* Target)
{
	if (!Target)
	{
		return;
	}

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

	OnAutoFaceEnd.Broadcast();
}

void ASoulCharacter::UpdateAutoFace(float DeltaSeconds)
{
	if (!AutoFaceTarget.IsValid())
	{
		StopAutoFace();
		return;
	}

	FVector ToTarget = AutoFaceTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0;

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

void ASoulCharacter::PlayOpenBoxAnim()
{
	if (USoulAnimInstance* Anim = Cast<USoulAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->PlayOpenBoxMontage();
	}
}

void ASoulCharacter::SetWeaponType(EWeaponType NewType)
{
	if (CurrentWeaponType == NewType)
	{
		return;
	}

	CurrentWeaponType = NewType;
}

void ASoulCharacter::PlayOpenDoorAnim()
{
	if (USoulAnimInstance* Anim = Cast<USoulAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->PlayOpenDoorMontage();
	}
}

void ASoulCharacter::BeginLadder(ASoulLadderActor* Ladder)
{
	if (!Ladder)
	{
		return;
	}

	if (LocomotionState != ELocomotionState::Normal)
	{
		return;
	}

	StopAiming();
	bIsSprinting = false;
	UpdateMovementSpeed();

	CurrentLadder = Ladder;
	LadderInput = 0;
	bLadderMounting = true;

	FaceToActor(Ladder);

	OnAutoFaceEnd.RemoveAll(this);
	OnAutoFaceEnd.AddLambda([this]()
		{
			if (!CurrentLadder.IsValid())
			{
				bLadderMounting = false;
				return;
			}

			if (CurrentLadder->GetLastUseSide() == ELadderUseSide::Top)
			{
				bLadderMounting = true;
				bTopMountMoving = true;

				TopMountMoveElapsed = 0;
				TopMountStartLoc = GetActorLocation();
				TopMountStartRot = GetActorRotation();

				TopMountTargetLoc = CurrentLadder->GetTopMountStartLocation();
				TopMountTargetRot = CurrentLadder->GetTopMountStartRotation();

				if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
				{
					MoveComp->StopMovementImmediately();
					MoveComp->SetMovementMode(MOVE_Flying);
					MoveComp->Velocity = FVector::ZeroVector;
					MoveComp->bOrientRotationToMovement = false;
					MoveComp->bUseControllerDesiredRotation = false;
				}

				bUseControllerRotationYaw = false;

				return;
			}

			FVector SnapLoc;
			FRotator SnapRot;
			CurrentLadder->GetSnapTransform(this, SnapLoc, SnapRot);

			SetActorLocation(SnapLoc);
			SetActorRotation(SnapRot);

			EnterLadderMode();

			bLadderMounting = false;
		});
}

void ASoulCharacter::EndLadder()
{
	if (LocomotionState != ELocomotionState::Ladder)
	{
		return;
	}

	ExitLadderMode();

	CurrentLadder = nullptr;
	LadderInput = 0;
	bLadderMounting = false;
}

void ASoulCharacter::EnterLadderMode()
{
	LocomotionState = ELocomotionState::Ladder;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->Velocity = FVector::ZeroVector;

		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	bUseControllerRotationYaw = false;
}

void ASoulCharacter::ExitLadderMode()
{
	LocomotionState = ELocomotionState::Normal;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = true;
	}
}

void ASoulCharacter::UpdateLadder(float DeltaSeconds)
{
	if (!CurrentLadder.IsValid())
	{
		EndLadder();
		return;
	}

	if (bLadderMounting)
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->Velocity = FVector::ZeroVector;
		}

		return;
	}

	FVector SnapLoc;
	FRotator SnapRot;
	CurrentLadder->GetSnapTransform(this, SnapLoc, SnapRot);

	float MinZ, MaxZ;
	CurrentLadder->GetClimbZRange(MinZ, MaxZ);

	FVector NewLoc = GetActorLocation();

	NewLoc.X = SnapLoc.X;
	NewLoc.Y = SnapLoc.Y;

	NewLoc.Z += LadderInput * LadderMoveSpeed * DeltaSeconds;
	NewLoc.Z = FMath::Clamp(NewLoc.Z, MinZ, MaxZ);

	const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), SnapRot, DeltaSeconds, LadderAlignInterpSpeed);

	SetActorLocation(NewLoc);
	SetActorRotation(NewRot);

	constexpr float EndEpsilon = 1;

	const bool bAtTop = FMath::Abs(NewLoc.Z - MaxZ) <= EndEpsilon;
	const bool bAtBottom = FMath::Abs(NewLoc.Z - MinZ) <= EndEpsilon;

	if (bAtTop && LadderInput > 0)
	{
		LadderInput = 0;
		bLadderMounting = true;

		if (AnimInstance)
        {
            AnimInstance->PlayLadderTopExitMontage();
        }
		return;
	}

	if (bAtBottom && LadderInput < 0)
	{
		const FVector ExitLoc = CurrentLadder->GetBottomExitLocation();
		EndLadder();
		SetActorLocation(ExitLoc, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->Velocity = FVector::ZeroVector;
	}
}

void ASoulCharacter::MoveCompleted(const FInputActionValue& Value)
{
	if (LocomotionState == ELocomotionState::Ladder)
	{
		LadderInput = 0;
	}
}

void ASoulCharacter::UpdateTopMountMove(float DeltaSeconds)
{
	if (!CurrentLadder.IsValid())
	{
		bTopMountMoving = false;
		bLadderMounting = false;
		return;
	}

	TopMountMoveElapsed += DeltaSeconds;
	const float SafeDuration = FMath::Max(TopMountMoveTime, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(TopMountMoveElapsed / SafeDuration, 0, 1);

	const FVector NewLoc = FMath::Lerp(TopMountStartLoc, TopMountTargetLoc, Alpha);
	const FRotator NewRot = FMath::Lerp(TopMountStartRot, TopMountTargetRot, Alpha);

	SetActorLocation(NewLoc, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(NewRot);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->Velocity = FVector::ZeroVector;
	}

	if (Alpha >= 1)
	{
		bTopMountMoving = false;

		if (AnimInstance)
		{
			AnimInstance->PlayLadderTopMountMontage();
		}
	}
}