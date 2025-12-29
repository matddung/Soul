#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameSettingSaveData.generated.h"

UCLASS()
class SOUL_API UGameSettingSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	float MasterVolume = 1.0f;
};