#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "SoulInventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_MULTICAST_DELEGATE(FOnQuickSlotChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOUL_API USoulInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USoulInventoryComponent();

    FORCEINLINE const TArray<FInventorySlot>& GetSlots() const { return Slots; }
    FORCEINLINE int32 GetSlotCount() const { return Slots.Num(); }
    FORCEINLINE int32 GetQuickSlotCount() const { return QuickSlotAssignments.Num(); }
    FORCEINLINE const TArray<int32>& GetQuickSlotAssignments() const { return QuickSlotAssignments; }
    FORCEINLINE int32 GetActiveQuickSlotIndex() const { return ActiveQuickSlotIndex; }

    void SetSlotCount(int32 NewSlotCount, bool bBroadcastChange = true);
    void SetQuickSlotCount(int32 NewQuickSlotCount);
    void CycleQuickSlots(int32 Delta);
    bool AssignQuickSlot(int32 InventorySlotIndex, int32 QuickSlotNumber);

    bool GainItem(EInventoryItemType Type, int32 Quantity);
    bool UseItemAtIndex(int32 SlotIndex, int32 Quantity = 1);

    void SaveInventory() const;
    bool LoadInventory();

protected:
    virtual void BeginPlay() override;

    FInventoryItemDefinition GetDefinition(EInventoryItemType Type) const;
    FInventoryItem MakeItem(EInventoryItemType Type, int32 Quantity) const;
    int32 FindEmptySlot() const;

    bool TryMergeStack(FInventorySlot& Slot, const FInventoryItemDefinition& Definition, int32& RemainingQuantity);
    bool TryCreateNewStack(const FInventoryItemDefinition& Definition, int32& RemainingQuantity);

    void HandleInventoryMutated();
    void ValidateQuickSlots();

public:
    FOnInventoryChanged OnInventoryChanged;
    FOnQuickSlotChanged OnQuickSlotChanged;

protected:
    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 InitialSlotCount = 16;

    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 EnhancementStoneMaxStack = 99;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 0))
    int32 InitialPotionCharges = 2;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 0))
    int32 InitialEnhancementStones = 150;

private:
    UPROPERTY()
    TArray<FInventorySlot> Slots;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 InitialQuickSlotCount = 3;

    UPROPERTY()
    TArray<int32> QuickSlotAssignments;

    int32 ActiveQuickSlotIndex = 0;

    bool bHasInitializedInventory = false;
};