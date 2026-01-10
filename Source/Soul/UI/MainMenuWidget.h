#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS()
class SOUL_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnNewGameClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void HandleSettingsBackRequested();

	void RefreshContinueVisibility();
	void ShowMainMenu();
	bool HasSavedGame() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Continue;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_NewGame;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName MainGameLevelName = FName("MainMap");
};