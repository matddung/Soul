#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "../Character/SoulCharacterStatComponent.h"
#include "SoulPlayerController.generated.h"

class UCrosshairWidget;
class UInteractPromptWidget;
class UInputAction;
class UInputMappingContext;
class ASoulCharacter;
class UPauseMenuWidget;
class USoundBase;
class UAudioComponent;
class USoundMix;
class USoundClass;
class UGameSettingSaveData;
class USoulCharacterStatWidget;
class USoulCharacterStatComponent;
class UInventoryWidget;
class USoulInventoryComponent;
class UHUDWidget;
class UGameProgressSaveData;

USTRUCT()
struct FPlayerActionKeyMapping
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UInputAction> InputAction = nullptr;

    UPROPERTY()
    FText DisplayName;
};

UCLASS()
class SOUL_API ASoulPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void ShowCrosshair(bool bShow);

    void OnCrosshairShot();
    void OnCrosshairReset();

    void ShowInteractPrompt(bool bShow, const FText& Text);

    void SaveGameSettings(float InMasterVolume);

    TArray<FPlayerActionKeyMapping> GetRebindableActions() const;
    FKey GetKeyForAction(const UInputAction* InputAction) const;
    void UpdateKeyMapping(UInputAction* InputAction, const FKey& NewKey, bool bSave = true);
    void ResetKeyMappingsToDefault();
    void SaveKeyMappings();
    void ApplySavedKeyMappings();
    TMap<FName, FKey> GetCurrentKeyMappings() const;

    void RequestClosePauseMenu();

    void OnGunAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

    void SaveCurrentGame();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void AddDefaultMappingContext();
    void RemoveDefaultMappingContext();
    void BindInputActions();
    ASoulCharacter* GetSoulCharacter() const;
    void BuildRuntimeMappingContext();
    UInputMappingContext* GetActiveMappingContext() const;

    void HandleMove(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleSprintStart(const FInputActionValue& Value);
    void HandleSprintStop(const FInputActionValue& Value);
    void HandleAttack(const FInputActionValue& Value);
    void HandleSwapSword(const FInputActionValue& Value);
    void HandleSwapGun(const FInputActionValue& Value);
    void HandleSwapEmpty(const FInputActionValue& Value);
    void HandleGunAimStart(const FInputActionValue& Value);
    void HandleGunAimStop(const FInputActionValue& Value);
    void HandleDodge(const FInputActionValue& Value);
    void HandleInteract(const FInputActionValue& Value);
    void HandleMoveCompleted(const FInputActionValue& Value);

    UFUNCTION()
    void TogglePauseMenu();

    void OpenPauseMenu();
    void ClosePauseMenu();

    void PlayBackgroundMusic();
    void StopBackgroundMusic();

    void LoadGameSettings();

    void HandleToggleStatus(const FInputActionValue& Value);

    void ToggleStatusWidget();
    void OpenStatusWidget();
    void CloseStatusWidget();
    void RefreshStatusWidget();
    void BindStatComponent();

    UFUNCTION()
    void OnCharacterStatChanged();

    UFUNCTION()
    void OnRequestAdjust(ECharacterStatType StatType, int32 Delta);

    void HandleToggleInventory(const FInputActionValue& Value);
    void ToggleInventory();
    void OpenInventory();
    void CloseInventory();

    void HandleQuickSlotScrollUp(const FInputActionValue& Value);
    void HandleQuickSlotScrollDown(const FInputActionValue& Value);

    void BindInventoryComponent();
    void OnInventoryChanged();
    void OnQuickSlotChanged();
    void RefreshHUD();

    void HandleUseQuickSlotItem(const FInputActionValue& Value);

    void LoadGameProgress();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

    UPROPERTY()
    TObjectPtr<UCrosshairWidget> CrosshairWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInteractPromptWidget> InteractPromptClass;

    UPROPERTY()
    TObjectPtr<UInteractPromptWidget> InteractPromptWidget;

    UPROPERTY(EditAnywhere, Category = "Input")
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
    TObjectPtr<UInputAction> PauseMenuAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> StatusAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InventoryAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> QuickSlotScrollUpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> QuickSlotScrollDownAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> QuickSlotUseAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPauseMenuWidget> PauseMenuClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> BackgroundMusic;

    UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0"))
    float BackgroundMusicVolume = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundMix> MasterSoundMix;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundClass> MasterSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InitialMasterVolume = 1.0f;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> BackgroundMusicComponent;

    UPROPERTY()
    TObjectPtr<UGameSettingSaveData> GameSettingSaveData;

    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> RuntimeMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<USoulCharacterStatWidget> CharacterStatWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<USoulCharacterStatWidget> CharacterStatWidget;

    UPROPERTY()
    TWeakObjectPtr<USoulCharacterStatComponent> CachedStatComponent;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UInventoryWidget> InventoryWidget;

    UPROPERTY()
    TWeakObjectPtr<USoulInventoryComponent> CachedInventoryComponent;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UHUDWidget> HUDWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UHUDWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UGameProgressSaveData> GameProgressSaveData;

private:
    bool bMappingContextAdded = false;
    bool bPauseMenuOpen = false;
    bool bStatusWidgetOpen = false;
    bool bInventoryOpen = false;

    UPROPERTY()
    TObjectPtr<UPauseMenuWidget> PauseMenuInstance = nullptr;
};