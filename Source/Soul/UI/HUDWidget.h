#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class USizeBox;
class UTextBlock;
class USoulCharacterStatComponent;
class USoulInventoryComponent;

UCLASS()
class SOUL_API UHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeWidget(USoulCharacterStatComponent* StatComponent, USoulInventoryComponent* InventoryComponent);

    void SetStatComponent(USoulCharacterStatComponent* StatComponent);
    void SetInventoryComponent(USoulInventoryComponent* InventoryComponent);

    void RefreshStats();
    void RefreshQuickSlots();
    void SetGunAmmo(int32 CurrentAmmo, int32 MaxAmmo, bool bShowAmmo);

protected:
    virtual void NativeOnInitialized() override;

    UFUNCTION()
    void HandleStatChanged();

    void UpdateBar(USizeBox* Box, UProgressBar* Bar, float Current, float Max) const;
    FText BuildQuickSlotLabel(int32 QuickSlotOffset) const;

protected:
    UPROPERTY(meta = (BindWidget))
    USizeBox* Box_Health;

    UPROPERTY(meta = (BindWidget))
    USizeBox* Box_Stamina;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Prg_Health;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Prg_Stamina;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_QuickSlotA;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_QuickSlotB;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_GunAmmo;

private:
    UPROPERTY()
    TWeakObjectPtr<USoulCharacterStatComponent> CachedStatComponent;

    UPROPERTY()
    TWeakObjectPtr<USoulInventoryComponent> CachedInventoryComponent;
};