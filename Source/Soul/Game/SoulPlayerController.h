#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoulPlayerController.generated.h"

class UUserWidget;

UCLASS()
class SOUL_API ASoulPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ShowCrosshair(bool bShow);

	void OnCrosshairShot();
	void OnCrosshairReset();

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	class UCrosshairWidget* CrosshairWidget;
};
