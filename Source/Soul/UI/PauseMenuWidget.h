#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class USlider;
class USoundMix;
class USoundClass;

UENUM(BlueprintType)
enum class EPausePage : uint8
{
	Main,
	Settings,
	QuitConfirm
};

UCLASS()
class SOUL_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ShowMain();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ShowSettings();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ShowQuitConfirm();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	EPausePage GetCurrentPage() const;

	void SetMasterAudioConfig(USoundMix* InSoundMix, USoundClass* InSoundClass, float InVolume);

protected:
	void SetPage(EPausePage Page);

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnBackFromSettingsClicked();

	UFUNCTION()
	void OnQuitYesClicked();

	UFUNCTION()
	void OnQuitNoClicked();

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultMasterVolume = 1.0f;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WS_Pages;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Settings;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Quit;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_BackFromSettings;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_QuitYes;

	UPROPERTY(meta = (BindWidget))
	UButton* Btn_QuitNo;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;

	float CurrentMasterVolume = 1.0f;

};