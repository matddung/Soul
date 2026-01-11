#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverRetryRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverMainRequested);

UCLASS()
class SOUL_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnGameOverRetryRequested OnRetryRequested;

	UPROPERTY(BlueprintAssignable)
	FOnGameOverMainRequested OnMainRequested;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnMainClicked();

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Retry;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Main;
};