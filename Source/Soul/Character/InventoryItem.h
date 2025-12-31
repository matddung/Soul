#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
    None,
    Potion,
    EnhancementStone,
};

UENUM(BlueprintType)
enum class EInventoryEmptyPolicy : uint8
{
    KeepWhenEmpty,
    RemoveWhenEmpty,
};

USTRUCT(BlueprintType)
struct FInventoryItemDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EInventoryItemType Type = EInventoryItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bStackable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EInventoryEmptyPolicy EmptyPolicy = EInventoryEmptyPolicy::RemoveWhenEmpty;
};

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EInventoryItemType Type = EInventoryItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FInventoryItemDefinition Definition;

    bool IsEmpty() const
    {
        return Type == EInventoryItemType::None;
    }
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FInventoryItem Item;

    bool IsEmpty() const
    {
        return Item.IsEmpty();
    }

    void Clear()
    {
        Item = FInventoryItem();
    }
};

FText GetInventoryItemDisplayName(EInventoryItemType Type);