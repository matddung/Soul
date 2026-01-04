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

protected:
    virtual void NativeOnInitialized() override;

    UFUNCTION()
    void HandleStatChanged();

    void UpdateBar(USizeBox* Box, UProgressBar* Bar, float Current, float Max) const;
    FText BuildQuickSlotLabel(int32 QuickSlotOffset) const;

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    USizeBox* Box_Health;

    UPROPERTY(meta = (BindWidgetOptional))
    USizeBox* Box_Stamina;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* Prg_Health;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* Prg_Stamina;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_QuickSlotA;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_QuickSlotB;

private:
    UPROPERTY()
    TWeakObjectPtr<USoulCharacterStatComponent> CachedStatComponent;

    UPROPERTY()
    TWeakObjectPtr<USoulInventoryComponent> CachedInventoryComponent;
};