#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CharacterStatSaveData.generated.h"

UCLASS()
class SOUL_API UCharacterStatSaveData : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    int32 STR = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 DEX = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 VIT = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 END = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 Souls = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 InvestCount = 0;

    FORCEINLINE static FString GetSlotName() { return TEXT("CharacterStatSaveData"); }
};