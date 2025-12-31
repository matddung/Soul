#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Character/InventoryItem.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class USoulInventoryComponent;

UCLASS()
class SOUL_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetInventoryComponent(USoulInventoryComponent* InventoryComponent);
    void RefreshInventory(const TArray<FInventorySlot>& Slots, int32 TotalSlotCount);

protected:
    virtual void NativeOnInitialized() override;

    void BuildGrid();
    void ResetGrid();
    void AddSlotWidget(int32 SlotIndex, int32 Row, int32 Column);
    FText BuildSlotLabel(int32 SlotIndex) const;
    int32 GetColumnCount(int32 SlotCount) const;

    void OnInventoryChanged();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    UUniformGridPanel* GridPanel;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 DefaultColumns = 4;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 DefaultSlotCount = 16;

    TWeakObjectPtr<USoulInventoryComponent> CachedInventoryComponent;
    TArray<FInventorySlot> CachedSlots;
    int32 CachedSlotCount = 0;
};