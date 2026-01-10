#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UPauseMenuWidget;
class USoundMix;
class USoundClass;

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
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void HandleSettingsBackRequested();

	void RefreshContinueVisibility();
	void ApplyInitialAudioSettings();
	void ShowSettingsMenu();
	void ShowMainMenu();
	bool HasSavedGame() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Continue;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_NewGame;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPauseMenuWidget> SettingsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName MainGameLevelName = FName("MainMap");

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultMasterVolume = 1.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> SettingsWidget;

	float CachedMasterVolume = 1.0f;
};