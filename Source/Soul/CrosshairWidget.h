#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

UCLASS()
class SOUL_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Crosshair")
	void OnShot();

	UFUNCTION(BlueprintImplementableEvent, Category = "Crosshair")
	void OnReset();
};