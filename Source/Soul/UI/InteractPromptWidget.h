#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPromptWidget.generated.h"

UCLASS()
class SOUL_API UInteractPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void SetPromptText(const FText& InText);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UTextBlock> PromptText;

};