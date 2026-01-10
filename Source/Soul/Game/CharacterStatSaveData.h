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

    UPROPERTY(BlueprintReadWrite)
    float SavedSwordDamageBase = 20;

    UPROPERTY(BlueprintReadWrite)
    float SavedGunDamageBase = 15;

    UPROPERTY(BlueprintReadWrite)
    int32 SwordEnhancementLevel = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 GunEnhancementLevel = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bHasGunFromBox = false;

    UPROPERTY(BlueprintReadWrite)
    float SavedHP = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float SavedStamina = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bHasSavedVitals = false;

    UPROPERTY(BlueprintReadWrite)
    int32 SavedRemainingGunShots = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 SavedMaxGunShots = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bHasSavedAmmo = false;

    FORCEINLINE static FString GetSlotName() { return TEXT("CharacterStatSaveData"); }
};