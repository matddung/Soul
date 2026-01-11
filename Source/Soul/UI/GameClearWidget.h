#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameClearWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameClearReturnRequested);

UCLASS()
class SOUL_API UGameClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetClearTimeText(const FText& InText);

	UPROPERTY(BlueprintAssignable)
	FOnGameClearReturnRequested OnReturnRequested;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnReturnClicked();

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Return;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_ClearTime;
};