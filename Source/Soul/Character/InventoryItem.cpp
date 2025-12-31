#include "InventoryItem.h"

FText GetInventoryItemDisplayName(EInventoryItemType Type)
{
    switch (Type)
    {
    case EInventoryItemType::Potion:
        return NSLOCTEXT("Inventory", "PotionName", "Potion");
    case EInventoryItemType::EnhancementStone:
        return NSLOCTEXT("Inventory", "EnhancementStoneName", "Enhancement Stone");
    default:
        return NSLOCTEXT("Inventory", "Empty", "Empty");
    }
}