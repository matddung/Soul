#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "SoulInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOUL_API USoulInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USoulInventoryComponent();

    const TArray<FInventorySlot>& GetSlots() const { return Slots; }
    int32 GetSlotCount() const { return Slots.Num(); }

    void SetSlotCount(int32 NewSlotCount);

    bool GainItem(EInventoryItemType Type, int32 Quantity);
    bool UseItemAtIndex(int32 SlotIndex, int32 Quantity = 1);

    FOnInventoryChanged OnInventoryChanged;

protected:
    virtual void BeginPlay() override;

    FInventoryItemDefinition GetDefinition(EInventoryItemType Type) const;
    FInventoryItem MakeItem(EInventoryItemType Type, int32 Quantity) const;
    int32 FindEmptySlot() const;

    bool TryMergeStack(FInventorySlot& Slot, const FInventoryItemDefinition& Definition, int32& RemainingQuantity);
    bool TryCreateNewStack(const FInventoryItemDefinition& Definition, int32& RemainingQuantity);

protected:
    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 InitialSlotCount = 16;

    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 EnhancementStoneMaxStack = 99;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 0))
    int32 InitialPotionCharges = 2;

private:
    UPROPERTY()
    TArray<FInventorySlot> Slots;
};