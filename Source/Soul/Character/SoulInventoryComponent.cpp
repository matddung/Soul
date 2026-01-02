#include "SoulInventoryComponent.h"
#include "../Game/InventorySaveData.h"

#include "Kismet/GameplayStatics.h"

USoulInventoryComponent::USoulInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSlotCount(InitialSlotCount, false);
}

void USoulInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    const bool bLoaded = LoadInventory();

    bHasInitializedInventory = true;

    if (!bLoaded)
    {
        SetSlotCount(InitialSlotCount, false);

        if (InitialPotionCharges > 0)
        {
            GainItem(EInventoryItemType::Potion, InitialPotionCharges);
        }

        if (InitialEnhancementStones > 0)
        {
            GainItem(EInventoryItemType::EnhancementStone, InitialEnhancementStones);
        }
    }

    OnInventoryChanged.Broadcast();
    SaveInventory();
}

void USoulInventoryComponent::SetSlotCount(int32 NewSlotCount, bool bBroadcastChange)
{
    if (NewSlotCount < 1)
    {
        return;
    }

    const int32 PreviousCount = Slots.Num();
    Slots.SetNum(NewSlotCount);

    if (PreviousCount != NewSlotCount && bBroadcastChange)
    {
        OnInventoryChanged.Broadcast();
        SaveInventory();
    }
}

FInventoryItemDefinition USoulInventoryComponent::GetDefinition(EInventoryItemType Type) const
{
    FInventoryItemDefinition Definition;
    Definition.Type = Type;

    switch (Type)
    {
    case EInventoryItemType::Potion:
        Definition.bStackable = false;
        Definition.MaxStackSize = 1;
        Definition.EmptyPolicy = EInventoryEmptyPolicy::KeepWhenEmpty;
        break;
    case EInventoryItemType::EnhancementStone:
        Definition.bStackable = true;
        Definition.MaxStackSize = EnhancementStoneMaxStack;
        Definition.EmptyPolicy = EInventoryEmptyPolicy::RemoveWhenEmpty;
        break;
    default:
        Definition.Type = EInventoryItemType::None;
        Definition.bStackable = false;
        Definition.MaxStackSize = 1;
        Definition.EmptyPolicy = EInventoryEmptyPolicy::RemoveWhenEmpty;
        break;
    }

    return Definition;
}

FInventoryItem USoulInventoryComponent::MakeItem(EInventoryItemType Type, int32 Quantity) const
{
    FInventoryItem Item;
    Item.Type = Type;
    Item.Quantity = FMath::Max(0, Quantity);
    Item.Definition = GetDefinition(Type);
    return Item;
}

int32 USoulInventoryComponent::FindEmptySlot() const
{
    for (int32 Index = 0; Index < Slots.Num(); ++Index)
    {
        if (Slots[Index].IsEmpty())
        {
            return Index;
        }
    }

    return INDEX_NONE;
}

bool USoulInventoryComponent::TryMergeStack(FInventorySlot& Slot, const FInventoryItemDefinition& Definition, int32& RemainingQuantity)
{
    if (Slot.IsEmpty() || Slot.Item.Type != Definition.Type || !Definition.bStackable)
    {
        return false;
    }

    const int32 SpaceLeft = Definition.MaxStackSize - Slot.Item.Quantity;
    if (SpaceLeft <= 0)
    {
        return false;
    }

    const int32 Added = FMath::Min(RemainingQuantity, SpaceLeft);
    Slot.Item.Quantity += Added;
    RemainingQuantity -= Added;
    return Added > 0;
}

bool USoulInventoryComponent::TryCreateNewStack(const FInventoryItemDefinition& Definition, int32& RemainingQuantity)
{
    const int32 EmptyIndex = FindEmptySlot();
    if (EmptyIndex == INDEX_NONE)
    {
        return false;
    }

    const int32 StackSize = Definition.bStackable ? FMath::Min(RemainingQuantity, Definition.MaxStackSize) : RemainingQuantity;

    Slots[EmptyIndex].Item = MakeItem(Definition.Type, StackSize);
    RemainingQuantity -= StackSize;
    return true;
}

bool USoulInventoryComponent::GainItem(EInventoryItemType Type, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return false;
    }

    FInventoryItemDefinition Definition = GetDefinition(Type);
    if (Definition.Type == EInventoryItemType::None)
    {
        return false;
    }

    int32 RemainingQuantity = Quantity;
    bool bChanged = false;

    if (Definition.bStackable)
    {
        for (FInventorySlot& Slot : Slots)
        {
            if (RemainingQuantity <= 0)
            {
                break;
            }

            if (TryMergeStack(Slot, Definition, RemainingQuantity))
            {
                bChanged = true;
            }
        }
    }

    while (RemainingQuantity > 0)
    {
        if (!TryCreateNewStack(Definition, RemainingQuantity))
        {
            break;
        }

        bChanged = true;

        if (!Definition.bStackable)
        {
            break;
        }
    }

    if (bChanged)
    {
        OnInventoryChanged.Broadcast();
        SaveInventory();
    }

    return RemainingQuantity == 0;
}

bool USoulInventoryComponent::UseItemAtIndex(int32 SlotIndex, int32 Quantity)
{
    if (Quantity <= 0 || !Slots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    FInventorySlot& Slot = Slots[SlotIndex];
    if (Slot.IsEmpty())
    {
        return false;
    }

    Slot.Item.Quantity = FMath::Max(0, Slot.Item.Quantity - Quantity);

    if (Slot.Item.Quantity <= 0 && Slot.Item.Definition.EmptyPolicy == EInventoryEmptyPolicy::RemoveWhenEmpty)
    {
        Slot.Clear();
    }

    OnInventoryChanged.Broadcast();
    SaveInventory();
    return true;
}

void USoulInventoryComponent::SaveInventory() const
{
    if (!bHasInitializedInventory)
    {
        return;
    }

    UInventorySaveData* SaveData = Cast<UInventorySaveData>(UGameplayStatics::CreateSaveGameObject(UInventorySaveData::StaticClass()));

    if (!SaveData)
    {
        return;
    }

    SaveData->SlotCount = Slots.Num();
    SaveData->Slots = Slots;

    UGameplayStatics::SaveGameToSlot(SaveData, UInventorySaveData::GetSlotName(), 0);
}

bool USoulInventoryComponent::LoadInventory()
{
    const FString SlotName = UInventorySaveData::GetSlotName();

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return false;
    }

    const UInventorySaveData* SaveData = Cast<UInventorySaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveData)
    {
        return false;
    }

    const int32 SavedSlotCount = FMath::Max(1, SaveData->SlotCount > 0 ? SaveData->SlotCount : SaveData->Slots.Num());
    SetSlotCount(SavedSlotCount, false);

    const int32 CopyCount = FMath::Min(Slots.Num(), SaveData->Slots.Num());

    for (int32 Index = 0; Index < CopyCount; ++Index)
    {
        Slots[Index] = SaveData->Slots[Index];
    }

    for (int32 Index = CopyCount; Index < Slots.Num(); ++Index)
    {
        Slots[Index].Clear();
    }

    return true;
}