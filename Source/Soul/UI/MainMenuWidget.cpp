#include "MainMenuWidget.h"
#include "PauseMenuWidget.h"
#include "../Game/GameProgressSaveData.h"
#include "../Game/GameSettingSaveData.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Continue)
	{
		Btn_Continue->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueClicked);
	}

	if (Btn_NewGame)
	{
		Btn_NewGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);
	}

	if (Btn_Settings)
	{
		Btn_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	RefreshContinueVisibility();
	ApplyInitialAudioSettings();
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ShowMainMenu();
}

void UMainMenuWidget::OnContinueClicked()
{
	if (!HasSavedGame() || MainGameLevelName.IsNone())
	{
		return;
	}

	UGameplayStatics::OpenLevel(this, MainGameLevelName);
}

void UMainMenuWidget::OnNewGameClicked()
{
	if (MainGameLevelName.IsNone())
	{
		return;
	}

	UGameplayStatics::DeleteGameInSlot(UGameProgressSaveData::GetSlotName(), 0);
	UGameplayStatics::OpenLevel(this, MainGameLevelName);
}

void UMainMenuWidget::OnSettingsClicked()
{
	ShowSettingsMenu();
}

void UMainMenuWidget::OnQuitClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

void UMainMenuWidget::HandleSettingsBackRequested()
{
	ShowMainMenu();
}

void UMainMenuWidget::RefreshContinueVisibility()
{
	const bool bHasSave = HasSavedGame();

	if (Btn_Continue)
	{
		Btn_Continue->SetVisibility(bHasSave ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMainMenuWidget::ApplyInitialAudioSettings()
{
	CachedMasterVolume = DefaultMasterVolume;

	const FString SlotName = UGameSettingSaveData::GetSlotName();
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		if (UGameSettingSaveData* SaveData = Cast<UGameSettingSaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
		{
			CachedMasterVolume = FMath::Clamp(SaveData->MasterVolume, 0.0f, 1.0f);
		}
	}

	if (MasterSoundMix && MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, MasterSoundMix, MasterSoundClass, CachedMasterVolume, 1.0f, 0.0f, true);
		UGameplayStatics::PushSoundMixModifier(this, MasterSoundMix);
	}
}

void UMainMenuWidget::ShowSettingsMenu()
{
	if (!SettingsWidget && SettingsWidgetClass)
	{
		SettingsWidget = CreateWidget<UPauseMenuWidget>(GetOwningPlayer(), SettingsWidgetClass);
		if (SettingsWidget)
		{
			SettingsWidget->SetMasterAudioConfig(MasterSoundMix, MasterSoundClass, CachedMasterVolume);
			SettingsWidget->OnBackFromSettingsRequested.RemoveAll(this);
			SettingsWidget->OnBackFromSettingsRequested.AddDynamic(this, &UMainMenuWidget::HandleSettingsBackRequested);
		}
	}

	if (!SettingsWidget)
	{
		return;
	}

	SetVisibility(ESlateVisibility::Hidden);

	if (!SettingsWidget->IsInViewport())
	{
		SettingsWidget->AddToViewport(200);
	}

	SettingsWidget->ShowSettings();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(SettingsWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void UMainMenuWidget::ShowMainMenu()
{
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		SettingsWidget->RemoveFromParent();
	}

	SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

bool UMainMenuWidget::HasSavedGame() const
{
	const FString SlotName = UGameProgressSaveData::GetSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	UGameProgressSaveData* SaveData = Cast<UGameProgressSaveData>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveData || SaveData->LevelName.IsEmpty())
	{
		return false;
	}

	return true;
}