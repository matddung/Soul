#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class USlider;
class USoundMix;
class USoundClass;
class UVerticalBox;
class UInputKeySelector;
class UTextBlock;
class UHorizontalBox;

UENUM(BlueprintType)
enum class EPausePage : uint8
{
	Main,
	Settings,
	QuitConfirm
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseMenuBackRequested);

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

	UPROPERTY(BlueprintAssignable, Category = "Pause")
	FOnPauseMenuBackRequested OnBackFromSettingsRequested;

protected:
	virtual void NativeOnInitialized() override;

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
	void OnSaveClicked();

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnKeySelected(FInputChord SelectedKey);

	UFUNCTION()
	void OnKeySelectorSelectionChanged();

	UFUNCTION()
	void OnResetKeysClicked();

	void BuildKeyBindingList();
	void RefreshKeySelectors();

public:
	UPROPERTY(EditDefaultsOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultMasterVolume = 1.0f;

protected:
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
	UButton* Btn_Save;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;

	float CurrentMasterVolume = 1.0f;

	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* VB_KeyBindings;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_ResetKeys;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UInputKeySelector>> KeySelectors;

	UPROPERTY(Transient)
	TArray<FName> SelectorActionNames;

	int32 ActiveSelectorIndex = INDEX_NONE;
};