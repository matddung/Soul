#include "SoulPlayerController.h"
#include "../UI/CrosshairWidget.h"
#include "../UI/InteractPromptWidget.h"
#include "../Character/SoulCharacter.h"
#include "../UI/PauseMenuWidget.h"
#include "GameSettingSaveData.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

void ASoulPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AddDefaultMappingContext();
    LoadGameSettings();

    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UCrosshairWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport();
            CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    bShowMouseCursor = false;

    if (MasterSoundMix && MasterSoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(this, MasterSoundMix, MasterSoundClass, InitialMasterVolume, 1.0f, 0.0f, true);
        UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
    }

    PlayBackgroundMusic();
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

void ASoulPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopBackgroundMusic();

    Super::EndPlay(EndPlayReason);
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

        EnhancedInputComponent->BindAction(PauseMenuAction, ETriggerEvent::Started, this, &ASoulPlayerController::TogglePauseMenu);
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

void ASoulPlayerController::TogglePauseMenu()
{
    if (bPauseMenuOpen)
    {
        ClosePauseMenu();
    }
    else
    {
        OpenPauseMenu();
    }
}

void ASoulPlayerController::OpenPauseMenu()
{
    if (!PauseMenuClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PauseMenuClass is not set on PlayerController."));
        return;
    }

    if (!PauseMenuInstance)
    {
        PauseMenuInstance = CreateWidget<UPauseMenuWidget>(this, PauseMenuClass);

        if (PauseMenuInstance)
        {
            PauseMenuInstance->SetMasterAudioConfig(MasterSoundMix, MasterSoundClass, MasterSoundMix ? InitialMasterVolume : PauseMenuInstance->DefaultMasterVolume);
        }
    }

    if (PauseMenuInstance && !PauseMenuInstance->IsInViewport())
    {
        PauseMenuInstance->AddToViewport(100);
    }

    SetPause(true);
    bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(PauseMenuInstance ? PauseMenuInstance->TakeWidget() : TSharedPtr<SWidget>());
    SetInputMode(InputMode);

    bPauseMenuOpen = true;
}

void ASoulPlayerController::ClosePauseMenu()
{
    if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
    {
        PauseMenuInstance->RemoveFromParent();
    }

    SetPause(false);
    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    bPauseMenuOpen = false;
}

void ASoulPlayerController::SaveGameSettings(float InMasterVolume)
{
    if (!GameSettingSaveData)
    {
        GameSettingSaveData = Cast<UGameSettingSaveData>(UGameplayStatics::CreateSaveGameObject(UGameSettingSaveData::StaticClass()));
    }

    if (GameSettingSaveData)
    {
        GameSettingSaveData->MasterVolume = FMath::Clamp(InMasterVolume, 0.0f, 1.0f);
        UGameplayStatics::SaveGameToSlot(GameSettingSaveData, UGameSettingSaveData::GetSlotName(), 0);
    }
}

void ASoulPlayerController::LoadGameSettings()
{
    const FString SlotName = UGameSettingSaveData::GetSlotName();

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        GameSettingSaveData = Cast<UGameSettingSaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    if (!GameSettingSaveData)
    {
        GameSettingSaveData = Cast<UGameSettingSaveData>(UGameplayStatics::CreateSaveGameObject(UGameSettingSaveData::StaticClass()));
    }

    if (GameSettingSaveData)
    {
        InitialMasterVolume = FMath::Clamp(GameSettingSaveData->MasterVolume, 0.0f, 1.0f);
    }
}

void ASoulPlayerController::PlayBackgroundMusic()
{
    if (!IsLocalController() || !BackgroundMusic)
    {
        return;
    }

    if (!BackgroundMusicComponent)
    {
        BackgroundMusicComponent = NewObject<UAudioComponent>(this);
        if (!BackgroundMusicComponent)
        {
            return;
        }

        BackgroundMusicComponent->bAutoActivate = false;
        BackgroundMusicComponent->bIsUISound = true;
        BackgroundMusicComponent->RegisterComponent();
    }

    BackgroundMusicComponent->SetSound(BackgroundMusic);
    BackgroundMusicComponent->Play();
}

void ASoulPlayerController::StopBackgroundMusic()
{
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->Stop();
    }
}