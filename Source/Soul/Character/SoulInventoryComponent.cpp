#include "SoulInventoryComponent.h"

USoulInventoryComponent::USoulInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetSlotCount(InitialSlotCount);
}

void USoulInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    SetSlotCount(InitialSlotCount);
}

void USoulInventoryComponent::SetSlotCount(int32 NewSlotCount)
{
    if (NewSlotCount < 1)
    {
        return;
    }

    const int32 PreviousCount = Slots.Num();
    Slots.SetNum(NewSlotCount);

    if (PreviousCount != NewSlotCount)
    {
        OnInventoryChanged.Broadcast();
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
    return true;
}