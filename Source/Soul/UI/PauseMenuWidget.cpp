#include "PauseMenuWidget.h"
#include "../Game/SoulPlayerController.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/InputKeySelector.h"
#include "InputAction.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

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

	if (Btn_ResetKeys)
	{
		Btn_ResetKeys->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResetKeysClicked);
	}

	BuildKeyBindingList();

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
	RefreshKeySelectors();
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
	if (ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer()))
	{
		SoulPC->RequestClosePauseMenu();
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

void UPauseMenuWidget::OnKeySelected(FInputChord SelectedKey)
{
	const int32 SelectorIndex = ActiveSelectorIndex;
	ActiveSelectorIndex = INDEX_NONE;

	if (!KeySelectors.IsValidIndex(SelectorIndex) || !SelectorActionNames.IsValidIndex(SelectorIndex))
	{
		return;
	}

	const TWeakObjectPtr<UInputKeySelector> Selector = KeySelectors[SelectorIndex];
	if (!Selector.IsValid())
	{
		return;
	}

	if (ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer()))
	{
		const FName ActionName = SelectorActionNames[SelectorIndex];
		const TArray<FPlayerActionKeyMapping> Actions = SoulPC->GetRebindableActions();

		for (const FPlayerActionKeyMapping& Action : Actions)
		{
			if (Action.InputAction && Action.InputAction->GetFName() == ActionName)
			{
				SoulPC->UpdateKeyMapping(Action.InputAction, SelectedKey.Key);
				break;
			}
		}

		RefreshKeySelectors();
	}
}

void UPauseMenuWidget::OnKeySelectorSelectionChanged()
{
	for (int32 Index = 0; Index < KeySelectors.Num(); ++Index)
	{
		const TWeakObjectPtr<UInputKeySelector>& Selector = KeySelectors[Index];
		if (Selector.IsValid() && Selector->GetIsSelectingKey())
		{
			ActiveSelectorIndex = Index;
			return;
		}
	}
}

void UPauseMenuWidget::OnMasterVolumeChanged(float Value)
{
	if (!MasterSoundMix || !MasterSoundClass) return;

	const float SafeValue = FMath::Clamp(Value, 0.001f, 1.0f);
	CurrentMasterVolume = SafeValue;

	UGameplayStatics::SetSoundMixClassOverride(this, MasterSoundMix, MasterSoundClass, CurrentMasterVolume, 1.0f, 0.0f, true);

	if (ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer()))
	{
		SoulPC->SaveGameSettings(CurrentMasterVolume);
	}
}

void UPauseMenuWidget::OnResetKeysClicked()
{
	if (ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer()))
	{
		SoulPC->ResetKeyMappingsToDefault();
		RefreshKeySelectors();
	}
}

void UPauseMenuWidget::BuildKeyBindingList()
{
	KeySelectors.Empty();
	SelectorActionNames.Empty();
	ActiveSelectorIndex = INDEX_NONE;

	if (!VB_KeyBindings)
	{
		return;
	}

	VB_KeyBindings->ClearChildren();

	ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer());
	if (!SoulPC || !WidgetTree)
	{
		return;
	}

	const TArray<FPlayerActionKeyMapping> Actions = SoulPC->GetRebindableActions();
	for (const FPlayerActionKeyMapping& Action : Actions)
	{
		if (!Action.InputAction)
		{
			continue;
		}

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (!Row)
		{
			continue;
		}

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (Label)
		{
			Label->SetText(Action.DisplayName);
			Row->AddChildToHorizontalBox(Label);
		}

		UInputKeySelector* Selector = WidgetTree->ConstructWidget<UInputKeySelector>(UInputKeySelector::StaticClass());
		if (Selector)
		{
			Selector->SetAllowGamepadKeys(true);
			Selector->SetAllowModifierKeys(false);
			Selector->OnKeySelected.AddDynamic(this, &UPauseMenuWidget::OnKeySelected);
			Selector->OnIsSelectingKeyChanged.AddDynamic(this, &UPauseMenuWidget::OnKeySelectorSelectionChanged);
			Selector->SetSelectedKey(SoulPC->GetKeyForAction(Action.InputAction));

			if (UHorizontalBoxSlot* SelectorSlot = Row->AddChildToHorizontalBox(Selector))
			{
				SelectorSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
				SelectorSlot->SetHorizontalAlignment(HAlign_Fill);
			}

			KeySelectors.Add(Selector);
			SelectorActionNames.Add(Action.InputAction->GetFName());
		}

		VB_KeyBindings->AddChild(Row);
	}
}

void UPauseMenuWidget::RefreshKeySelectors()
{
	ASoulPlayerController* SoulPC = Cast<ASoulPlayerController>(GetOwningPlayer());
	if (!SoulPC)
	{
		return;
	}

	const TArray<FPlayerActionKeyMapping> Actions = SoulPC->GetRebindableActions();

	for (int32 Index = 0; Index < KeySelectors.Num(); ++Index)
	{
		if (!SelectorActionNames.IsValidIndex(Index))
		{
			continue;
		}

		UInputKeySelector* Selector = KeySelectors[Index].Get();
		if (!Selector)
		{
			continue;
		}

		const FName ActionName = SelectorActionNames[Index];
		for (const FPlayerActionKeyMapping& Action : Actions)
		{
			if (Action.InputAction && Action.InputAction->GetFName() == ActionName)
			{
				Selector->SetSelectedKey(SoulPC->GetKeyForAction(Action.InputAction));
				break;
			}
		}
	}
}