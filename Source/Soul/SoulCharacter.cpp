#include "SoulCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASoulCharacter::ASoulCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ASoulCharacter::BeginPlay()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void ASoulCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulCharacter::Look);
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

	UE_LOG(LogTemp, Warning, TEXT("Move Called: %s"), *Value.ToString());
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