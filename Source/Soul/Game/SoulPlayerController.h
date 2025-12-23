#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoulPlayerController.generated.h"

class UUserWidget;
class UCrosshairWidget;
class UInteractPromptWidget;

UCLASS()
class SOUL_API ASoulPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ShowCrosshair(bool bShow);

	void OnCrosshairShot();
	void OnCrosshairReset();

	void ShowInteractPrompt(bool bShow, const FText& Text);

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY()
	TObjectPtr<UCrosshairWidget> CrosshairWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInteractPromptWidget> InteractPromptClass;

	UPROPERTY()
	TObjectPtr<UInteractPromptWidget> InteractPromptWidget;
};
