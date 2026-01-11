#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameProgressSaveData.generated.h"

UCLASS()
class SOUL_API UGameProgressSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(BlueprintReadWrite)
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite)
    float ElapsedPlayTimeSeconds = 0.0f;

    FORCEINLINE static FString GetSlotName() { return TEXT("GameProgressSaveData"); }
};