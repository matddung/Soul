#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHPBarWidget.generated.h"

class UProgressBar;

UCLASS()
class SOUL_API UEnemyHPBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Enemy|UI")
    void SetHealth(float Current, float Max);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HPProgressBar;
};