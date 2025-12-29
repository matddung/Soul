#include "PauseMenuWidget.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClicked);
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
	}

	if (Btn_BackFromSettings)
	{
		Btn_BackFromSettings->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnBackFromSettingsClicked);
	}

	if (Btn_QuitYes)
	{
		Btn_QuitYes->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitYesClicked);
	}

	if (Btn_QuitNo)
	{
		Btn_QuitNo->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitNoClicked);
	}

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UPauseMenuWidget::OnMasterVolumeChanged);
		Slider_MasterVolume->SetValue(CurrentMasterVolume);
	}

	ShowMain();
}

void UPauseMenuWidget::SetMasterAudioConfig(USoundMix* InSoundMix, USoundClass* InSoundClass, float InVolume)
{
	MasterSoundMix = InSoundMix;
	MasterSoundClass = InSoundClass;
	CurrentMasterVolume = FMath::Clamp(InVolume, 0.0f, 1.0f);

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetValue(CurrentMasterVolume);
	}

	OnMasterVolumeChanged(CurrentMasterVolume);
}

void UPauseMenuWidget::SetPage(EPausePage Page)
{
	if (!WS_Pages) return;

	switch (Page)
	{
	case EPausePage::Main:
		WS_Pages->SetActiveWidgetIndex(0);
		break;
	case EPausePage::Settings:
		WS_Pages->SetActiveWidgetIndex(1);
		break;
	case EPausePage::QuitConfirm:
		WS_Pages->SetActiveWidgetIndex(2);
		break;
	default:
		WS_Pages->SetActiveWidgetIndex(0);
		break;
	}
}

void UPauseMenuWidget::ShowMain()
{
	SetPage(EPausePage::Main);
}

void UPauseMenuWidget::ShowSettings()
{
	SetPage(EPausePage::Settings);
}

void UPauseMenuWidget::ShowQuitConfirm()
{
	SetPage(EPausePage::QuitConfirm);
}

EPausePage UPauseMenuWidget::GetCurrentPage() const
{
	if (!WS_Pages) return EPausePage::Main;

	switch (WS_Pages->GetActiveWidgetIndex())
	{
	case 1: return EPausePage::Settings;
	case 2: return EPausePage::QuitConfirm;
	default: return EPausePage::Main;
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		RemoveFromParent();
		PC->SetPause(false);
		PC->bShowMouseCursor = false;
		FInputModeGameOnly Mode;
		PC->SetInputMode(Mode);
	}
}

void UPauseMenuWidget::OnSettingsClicked()
{
	ShowSettings();
}

void UPauseMenuWidget::OnQuitClicked()
{
	ShowQuitConfirm();
}

void UPauseMenuWidget::OnBackFromSettingsClicked()
{
	ShowMain();
}

void UPauseMenuWidget::OnQuitNoClicked()
{
	ShowMain();
}

void UPauseMenuWidget::OnQuitYesClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

void UPauseMenuWidget::OnMasterVolumeChanged(float Value)
{
	if (!MasterSoundMix || !MasterSoundClass) return;

	const float SafeValue = FMath::Clamp(Value, 0.001f, 1.0f);
	CurrentMasterVolume = SafeValue;

	UGameplayStatics::SetSoundMixClassOverride(this, MasterSoundMix, MasterSoundClass, CurrentMasterVolume, 1.0f, 0.0f, true);
}