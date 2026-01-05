#include "SoulPlayerController.h"
#include "../UI/CrosshairWidget.h"
#include "../UI/InteractPromptWidget.h"
#include "../Character/SoulCharacter.h"
#include "../UI/PauseMenuWidget.h"
#include "GameSettingSaveData.h"
#include "../UI/SoulCharacterStatWidget.h"
#include "../Character/SoulCharacterStatComponent.h"
#include "../UI/InventoryWidget.h"
#include "../Character/SoulInventoryComponent.h"
#include "../UI/HUDWidget.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "InputMappingContext.h"

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

    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
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
    BindStatComponent();
    BindInventoryComponent();
    RefreshStatusWidget();
    RefreshHUD();
}

void ASoulPlayerController::OnUnPossess()
{
    Super::OnUnPossess();

    RemoveDefaultMappingContext();

    CloseStatusWidget();

    if (CachedStatComponent.IsValid())
    {
        CachedStatComponent->OnStatChanged.RemoveDynamic(this, &ASoulPlayerController::OnCharacterStatChanged);
    }

    CachedStatComponent.Reset();

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
        CachedInventoryComponent->OnQuickSlotChanged.RemoveAll(this);
    }

    CachedInventoryComponent.Reset();

    if (HUDWidget)
    {
        HUDWidget->SetStatComponent(nullptr);
        HUDWidget->SetInventoryComponent(nullptr);
        HUDWidget->SetGunAmmo(0, 0, false);
    }

    if (InventoryWidget)
    {
        InventoryWidget->SetInventoryComponent(nullptr);
    }
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

void ASoulPlayerController::OnGunAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
    if (!HUDWidget)
    {
        return;
    }

    const ASoulCharacter* SoulCharacter = GetSoulCharacter();
    const bool bHasGunEquipped = SoulCharacter && SoulCharacter->GetCurrentWeaponType() == EWeaponType::Gun;
    const int32 AmmoToShow = bHasGunEquipped ? CurrentAmmo : 0;
    const int32 MaxAmmoToShow = bHasGunEquipped ? MaxAmmo : 0;

    HUDWidget->SetGunAmmo(AmmoToShow, MaxAmmoToShow, bHasGunEquipped);
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

TArray<FPlayerActionKeyMapping> ASoulPlayerController::GetRebindableActions() const
{
    TArray<FPlayerActionKeyMapping> Actions;
    Actions.Reserve(13);

    Actions.Add({ SprintAction, FText::FromString(TEXT("Sprint")) });
    Actions.Add({ AttackAction, FText::FromString(TEXT("Attack")) });
    Actions.Add({ SwapSwordAction, FText::FromString(TEXT("Swap Sword")) });
    Actions.Add({ SwapGunAction, FText::FromString(TEXT("Swap Gun")) });
    Actions.Add({ SwapEmptyAction, FText::FromString(TEXT("Unequip")) });
    Actions.Add({ SwordDodgeAction, FText::FromString(TEXT("Dodge")) });
    Actions.Add({ InteractAction, FText::FromString(TEXT("Interact")) });
    Actions.Add({ PauseMenuAction, FText::FromString(TEXT("Pause")) });
    Actions.Add({ StatusAction, FText::FromString(TEXT("Status")) });
    Actions.Add({ InventoryAction, FText::FromString(TEXT("Inventory")) });
    Actions.Add({ QuickSlotScrollUpAction, FText::FromString(TEXT("QuickSlotScrollUp")) });
    Actions.Add({ QuickSlotScrollDownAction, FText::FromString(TEXT("QuickSlotScrollDown")) });
    Actions.Add({ QuickSlotUseAction, FText::FromString(TEXT("Use Quick Slot")) });

    Actions.RemoveAll([](const FPlayerActionKeyMapping& Mapping)
        {
            return !IsValid(Mapping.InputAction);
        });

    return Actions;
}

FKey ASoulPlayerController::GetKeyForAction(const UInputAction* InputAction) const
{
    if (!InputAction)
    {
        return FKey();
    }

    if (const UInputMappingContext* MappingContext = GetActiveMappingContext())
    {
        for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
        {
            if (Mapping.Action == InputAction)
            {
                return Mapping.Key;
            }
        }
    }

    return FKey();
}

void ASoulPlayerController::UpdateKeyMapping(UInputAction* InputAction, const FKey& NewKey, bool bSave /*= true*/)
{
    if (!InputAction || !NewKey.IsValid())
    {
        return;
    }

    BuildRuntimeMappingContext();

    if (!RuntimeMappingContext)
    {
        return;
    }

    const TArray<FEnhancedActionKeyMapping> ExistingMappings = RuntimeMappingContext->GetMappings();

    for (const FEnhancedActionKeyMapping& Mapping : ExistingMappings)
    {
        if (Mapping.Key == NewKey && Mapping.Action != InputAction)
        {
            RuntimeMappingContext->UnmapKey(Mapping.Action, Mapping.Key);
        }
    }

    for (const FEnhancedActionKeyMapping& Mapping : ExistingMappings)
    {
        if (Mapping.Action == InputAction)
        {
            RuntimeMappingContext->UnmapKey(Mapping.Action, Mapping.Key);
        }
    }

    RuntimeMappingContext->MapKey(InputAction, NewKey);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        FModifyContextOptions Options;
        Options.bForceImmediately = true;
        Subsystem->RequestRebuildControlMappings(Options);
    }

    if (bSave)
    {
        SaveKeyMappings();
    }
}

void ASoulPlayerController::ResetKeyMappingsToDefault()
{
    if (!DefaultMappingContext)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (bMappingContextAdded)
        {
            if (UInputMappingContext* MappingContext = GetActiveMappingContext())
            {
                Subsystem->RemoveMappingContext(MappingContext);
            }
        }

        RuntimeMappingContext = DuplicateObject<UInputMappingContext>(DefaultMappingContext, this);
        Subsystem->AddMappingContext(RuntimeMappingContext, 0);
        bMappingContextAdded = true;

        FModifyContextOptions Options;
        Options.bForceImmediately = true;
        Subsystem->RequestRebuildControlMappings(Options);
    }

    SaveKeyMappings();
}

TMap<FName, FKey> ASoulPlayerController::GetCurrentKeyMappings() const
{
    TMap<FName, FKey> Result;

    const UInputMappingContext* MappingContext = GetActiveMappingContext();
    if (!MappingContext)
    {
        return Result;
    }

    const TArray<FPlayerActionKeyMapping> Actions = GetRebindableActions();
    for (const FPlayerActionKeyMapping& Action : Actions)
    {
        if (!Action.InputAction)
        {
            continue;
        }

        const FKey ActionKey = GetKeyForAction(Action.InputAction);
        if (ActionKey.IsValid())
        {
            Result.Add(Action.InputAction->GetFName(), ActionKey);
        }
    }

    return Result;
}

void ASoulPlayerController::ApplySavedKeyMappings()
{
    if (!GameSettingSaveData || GameSettingSaveData->SavedKeyMappings.Num() == 0)
    {
        return;
    }

    BuildRuntimeMappingContext();

    if (!RuntimeMappingContext)
    {
        return;
    }

    const TArray<FPlayerActionKeyMapping> Actions = GetRebindableActions();

    for (const TPair<FName, FKey>& SavedEntry : GameSettingSaveData->SavedKeyMappings)
    {
        if (!SavedEntry.Value.IsValid())
        {
            continue;
        }

        for (const FPlayerActionKeyMapping& Action : Actions)
        {
            if (Action.InputAction && Action.InputAction->GetFName() == SavedEntry.Key)
            {
                UpdateKeyMapping(Action.InputAction, SavedEntry.Value, false);
                break;
            }
        }
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        FModifyContextOptions Options;
        Options.bForceImmediately = true;
        Subsystem->RequestRebuildControlMappings(Options);
    }
}

void ASoulPlayerController::SaveKeyMappings()
{
    const float VolumeToSave = GameSettingSaveData ? GameSettingSaveData->MasterVolume : InitialMasterVolume;
    SaveGameSettings(VolumeToSave);
}

void ASoulPlayerController::BuildRuntimeMappingContext()
{
    if (!RuntimeMappingContext && DefaultMappingContext)
    {
        RuntimeMappingContext = DuplicateObject<UInputMappingContext>(DefaultMappingContext, this);
    }
}

UInputMappingContext* ASoulPlayerController::GetActiveMappingContext() const
{
    return RuntimeMappingContext ? RuntimeMappingContext : DefaultMappingContext;
}

void ASoulPlayerController::AddDefaultMappingContext()
{
    BuildRuntimeMappingContext();

    UInputMappingContext* MappingContext = GetActiveMappingContext();
    if (bMappingContextAdded || !MappingContext)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(MappingContext, 0);
        bMappingContextAdded = true;
    }
}

void ASoulPlayerController::RemoveDefaultMappingContext()
{
    if (!bMappingContextAdded)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (UInputMappingContext* MappingContext = GetActiveMappingContext())
        {
            Subsystem->RemoveMappingContext(MappingContext);
        }
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

        EnhancedInputComponent->BindAction(StatusAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleToggleStatus);

        EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleToggleInventory);

        EnhancedInputComponent->BindAction(QuickSlotScrollUpAction, ETriggerEvent::Triggered, this, &ASoulPlayerController::HandleQuickSlotScrollUp);
        EnhancedInputComponent->BindAction(QuickSlotScrollDownAction, ETriggerEvent::Triggered, this, &ASoulPlayerController::HandleQuickSlotScrollDown);

        EnhancedInputComponent->BindAction(QuickSlotUseAction, ETriggerEvent::Started, this, &ASoulPlayerController::HandleUseQuickSlotItem);
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

void ASoulPlayerController::RequestClosePauseMenu()
{
    ClosePauseMenu();
}

void ASoulPlayerController::TogglePauseMenu()
{
    if (bPauseMenuOpen)
    {
        ClosePauseMenu();
    }
    else
    {
        CloseInventory();
        CloseStatusWidget();
        OpenPauseMenu();
    }
}

void ASoulPlayerController::HandleToggleStatus(const FInputActionValue& Value)
{
    ToggleStatusWidget();
}

void ASoulPlayerController::OpenPauseMenu()
{
    CloseInventory();
    CloseStatusWidget();

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

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(PauseMenuInstance ? PauseMenuInstance->TakeWidget() : TSharedPtr<SWidget>());
    InputMode.SetHideCursorDuringCapture(false);
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

    if (!bInventoryOpen && !bStatusWidgetOpen)
    {
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }


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
        GameSettingSaveData->SavedKeyMappings = GetCurrentKeyMappings();
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
        ApplySavedKeyMappings();
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

void ASoulPlayerController::ToggleStatusWidget()
{
    if (bStatusWidgetOpen)
    {
        CloseStatusWidget();
    }
    else
    {
        ClosePauseMenu();
        CloseInventory();
        OpenStatusWidget();
    }
}

void ASoulPlayerController::OpenStatusWidget()
{
    if (!CharacterStatWidget && CharacterStatWidgetClass)
    {
        CharacterStatWidget = CreateWidget<USoulCharacterStatWidget>(this, CharacterStatWidgetClass);

        if (CharacterStatWidget)
        {
            CharacterStatWidget->OnRequestAdjustStat.AddDynamic(this, &ASoulPlayerController::OnRequestAdjust);
        }
    }

    if (!CharacterStatWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterStatWidgetClass is not set on PlayerController."));
        return;
    }

    if (CharacterStatWidget && !CharacterStatWidget->IsInViewport())
    {
        CharacterStatWidget->AddToViewport(50);
    }

    RefreshStatusWidget();

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(CharacterStatWidget ? CharacterStatWidget->TakeWidget() : TSharedPtr<SWidget>());
    SetInputMode(InputMode);

    bStatusWidgetOpen = true;
}

void ASoulPlayerController::CloseStatusWidget()
{
    if (CharacterStatWidget && CharacterStatWidget->IsInViewport())
    {
        CharacterStatWidget->RemoveFromParent();
    }

    bStatusWidgetOpen = false;

    if (!bPauseMenuOpen && !bInventoryOpen)
    {
        bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }
}

void ASoulPlayerController::RefreshStatusWidget()
{
    if (!CharacterStatWidget)
    {
        return;
    }

    if (USoulCharacterStatComponent* StatComp = CachedStatComponent.Get())
    {
        CharacterStatWidget->RefreshStats(StatComp);
    }
    else
    {
        CharacterStatWidget->RefreshStats(nullptr);
    }
}   

void ASoulPlayerController::BindStatComponent()
{
    if (CachedStatComponent.IsValid())
    {
        CachedStatComponent->OnStatChanged.RemoveDynamic(this, &ASoulPlayerController::OnCharacterStatChanged);
    }

    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        CachedStatComponent = SoulCharacter->StatComp;
    }
    else
    {
        CachedStatComponent.Reset();
    }

    if (CachedStatComponent.IsValid())
    {
        CachedStatComponent->OnStatChanged.AddDynamic(this, &ASoulPlayerController::OnCharacterStatChanged);
    }
}

void ASoulPlayerController::OnCharacterStatChanged()
{
    RefreshStatusWidget();
}

void ASoulPlayerController::OnRequestAdjust(ECharacterStatType StatType, int32 Delta)
{
    if (USoulCharacterStatComponent* StatComp = CachedStatComponent.Get())
    {
        bool bAdjusted = false;

        if (Delta > 0)
        {
            bAdjusted = StatComp->TryInvestStat(StatType);
        }
        else if (Delta < 0)
        {
            bAdjusted = StatComp->TryRefundStat(StatType);
        }

        if (!bAdjusted)
        {
            RefreshStatusWidget();
        }
    }
}

void ASoulPlayerController::HandleToggleInventory(const FInputActionValue& Value)
{
    ToggleInventory();
}

void ASoulPlayerController::ToggleInventory()
{
    if (bInventoryOpen)
    {
        CloseInventory();
    }
    else
    {
        ClosePauseMenu();
        CloseStatusWidget();
        OpenInventory();
    }
}

void ASoulPlayerController::OpenInventory()
{
    ClosePauseMenu();
    CloseStatusWidget();


    if (!InventoryWidget && InventoryWidgetClass)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
    }

    if (!InventoryWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryWidgetClass is not set on PlayerController."));
        return;
    }

    InventoryWidget->SetInventoryComponent(CachedInventoryComponent.Get());

    if (!InventoryWidget->IsInViewport())
    {
        InventoryWidget->AddToViewport(60);
    }

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(InventoryWidget ? InventoryWidget->TakeWidget() : TSharedPtr<SWidget>());
    SetInputMode(InputMode);

    bInventoryOpen = true;
}

void ASoulPlayerController::CloseInventory()
{
    if (InventoryWidget && InventoryWidget->IsInViewport())
    {
        InventoryWidget->RemoveFromParent();
    }

    bInventoryOpen = false;

    if (!bPauseMenuOpen && !bStatusWidgetOpen)
    {
        bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }
}

void ASoulPlayerController::BindInventoryComponent()
{
    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
        CachedInventoryComponent->OnQuickSlotChanged.RemoveAll(this);
    }

    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        CachedInventoryComponent = SoulCharacter->InventoryComp;
    }
    else
    {
        CachedInventoryComponent.Reset();
    }

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &ASoulPlayerController::OnInventoryChanged);
        CachedInventoryComponent->OnQuickSlotChanged.AddUObject(this, &ASoulPlayerController::OnQuickSlotChanged);
    }

    if (InventoryWidget)
    {
        InventoryWidget->SetInventoryComponent(CachedInventoryComponent.Get());
    }

    if (HUDWidget)
    {
        HUDWidget->SetInventoryComponent(CachedInventoryComponent.Get());
    }
}

void ASoulPlayerController::RefreshHUD()
{
    if (!HUDWidget)
    {
        return;
    }

    HUDWidget->SetStatComponent(CachedStatComponent.Get());
    HUDWidget->SetInventoryComponent(CachedInventoryComponent.Get());
    HUDWidget->RefreshStats();
    HUDWidget->RefreshQuickSlots();

    int32 CurrentAmmo = 0;
    int32 MaxAmmo = 0;
    bool bHasGunEquipped = false;

    if (ASoulCharacter* SoulCharacter = GetSoulCharacter())
    {
        CurrentAmmo = SoulCharacter->GetRemainingGunShots();
        MaxAmmo = SoulCharacter->GetMaxGunShots();
        bHasGunEquipped = SoulCharacter->GetCurrentWeaponType() == EWeaponType::Gun;
    }

    HUDWidget->SetGunAmmo(CurrentAmmo, MaxAmmo, bHasGunEquipped);
}

void ASoulPlayerController::OnInventoryChanged()
{
    RefreshHUD();
}

void ASoulPlayerController::OnQuickSlotChanged()
{
    RefreshHUD();
}

void ASoulPlayerController::HandleQuickSlotScrollUp(const FInputActionValue& Value)
{
    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->CycleQuickSlots(1);
    }
}

void ASoulPlayerController::HandleQuickSlotScrollDown(const FInputActionValue& Value)
{
    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->CycleQuickSlots(-1);
    }
}

void ASoulPlayerController::HandleUseQuickSlotItem(const FInputActionValue& Value)
{
    if (!CachedInventoryComponent.IsValid())
    {
        return;
    }

    const int32 SlotIndex = CachedInventoryComponent->GetActiveQuickSlotInventoryIndex();
    const TArray<FInventorySlot>& Slots = CachedInventoryComponent->GetSlots();

    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
    {
        return;
    }

    const FInventoryItem& Item = Slots[SlotIndex].Item;

    if (Item.Type == EInventoryItemType::EnhancementStone)
    {
        if (!bInventoryOpen)
        {
            OpenInventory();
        }

        if (InventoryWidget)
        {
            InventoryWidget->UseItemAtSlotIndex(SlotIndex);
        }
    }
    else
    {
        CachedInventoryComponent->UseActiveQuickSlotItem();
    }
}