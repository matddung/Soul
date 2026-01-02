#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "../Character/InventoryItem.h"
#include "InventorySaveData.generated.h"

UCLASS()
class SOUL_API UInventorySaveData : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TArray<FInventorySlot> Slots;

    UPROPERTY(BlueprintReadWrite)
    int32 SlotCount = 0;

    FORCEINLINE static FString GetSlotName() { return TEXT("InventorySaveData"); }
};