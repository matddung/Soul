#include "SoulPlayerController.h"
#include "../UI/CrosshairWidget.h"
#include "../UI/InteractPromptWidget.h"
#include "../Character/SoulCharacter.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ASoulPlayerController::BeginPlay()
{
	Super::BeginPlay();

    AddDefaultMappingContext();

    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UCrosshairWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport();
            CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ASoulPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    BindInputActions();
}

void ASoulPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AddDefaultMappingContext();
}

void ASoulPlayerController::OnUnPossess()
{
    Super::OnUnPossess();

    RemoveDefaultMappingContext();
}

void ASoulPlayerController::ShowCrosshair(bool bShow)
{
    if (!CrosshairWidget)
    {
        return;
    }

    CrosshairWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ASoulPlayerController::OnCrosshairShot()
{
    if (CrosshairWidget)
    {
        CrosshairWidget->OnShot();
    }
}

void ASoulPlayerController::OnCrosshairReset()
{
    if (CrosshairWidget)
    {
        CrosshairWidget->OnReset();
    }
}

void ASoulPlayerController::ShowInteractPrompt(bool bShow, const FText& Text)
{
    if (bShow)
    {
        if (!InteractPromptWidget)
        {
            if (!InteractPromptClass)
            {
                return;
            }

            InteractPromptWidget = CreateWidget<UInteractPromptWidget>(this, InteractPromptClass);

            if (InteractPromptWidget)
            {
                InteractPromptWidget->AddToViewport(10);
            }
        }

        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Visible);
            InteractPromptWidget->SetPromptText(Text);
        }
    }
    else
    {
        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ASoulPlayerController::AddDefaultMappingContext()
{
    if (bMappingContextAdded || !DefaultMappingContext)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        bMappingContextAdded = true;
    }
}

void ASoulPlayerController::RemoveDefaultMappingContext()
{
    if (!bMappingContextAdded || !DefaultMappingContext)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(DefaultMappingContext);
    }

    bMappingContextAdded = false;
}

void ASoulPlayerController::BindInputActions()
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASoulPlayerController::HandleMove);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASoulPlayerController::HandleLook);

        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleSprintStart);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASoulPlayerController::HandleSprintStop);

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleAttack);

        EnhancedInputComponent->BindAction(SwapSwordAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleSwapSword);
        EnhancedInputComponent->BindAction(SwapGunAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleSwapGun);
        EnhancedInputComponent->BindAction(SwapEmptyAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleSwapEmpty);
        EnhancedInputComponent->BindAction(GunAimAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleGunAimStart);
        EnhancedInputComponent->BindAction(GunAimAction, ETriggerEvent::Completed, this, &ASoulPlayerController::HandleGunAimStop);

        EnhancedInputComponent->BindAction(SwordDodgeAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleDodge);

        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleInteract);

        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASoulPlayerController::HandleMoveCompleted);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ASoulPlayerController::HandleMoveCompleted);
    }
}

ASoulCharacter* ASoulPlayerController::GetSoulCharacter() const
{
    return GetPawn<ASoulCharacter>();
}

void ASoulPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->Move(Value);
    }
}

void ASoulPlayerController::HandleLook(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->Look(Value);
    }
}

void ASoulPlayerController::HandleSprintStart(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->SprintStart(Value);
    }
}

void ASoulPlayerController::HandleSprintStop(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->SprintStop(Value);
    }
}

void ASoulPlayerController::HandleAttack(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->Attack(Value);
    }
}

void ASoulPlayerController::HandleSwapSword(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->SwapSword(Value);
    }
}

void ASoulPlayerController::HandleSwapGun(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->SwapGun(Value);
    }
}

void ASoulPlayerController::HandleSwapEmpty(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->SwapEmpty(Value);
    }
}

void ASoulPlayerController::HandleGunAimStart(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->GunAimStart(Value);
    }
}

void ASoulPlayerController::HandleGunAimStop(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->GunAimStop(Value);
    }
}

void ASoulPlayerController::HandleDodge(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->Dodge(Value);
    }
}

void ASoulPlayerController::HandleInteract(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->Interact(Value);
    }
}

void ASoulPlayerController::HandleMoveCompleted(const FInputActionValue& Value)
{
    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        SoulCharacter->MoveCompleted(Value);
    }
}