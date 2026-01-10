#include "MainMenuWidget.h"
#include "../Game/GameProgressSaveData.h"
#include "../Game/GameSettingSaveData.h"
#include "../Game/CharacterStatSaveData.h"
#include "../Game/InventorySaveData.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

	if (Btn_Quit)
	{
		Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	RefreshContinueVisibility();
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

	if (UWorld* World = GetWorld())
	{
		RemoveFromParent();
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
		UGameplayStatics::OpenLevel(World, MainGameLevelName);
	}
}

void UMainMenuWidget::OnNewGameClicked()
{
	if (MainGameLevelName.IsNone())
	{
		return;
	}

	UGameplayStatics::DeleteGameInSlot(UGameProgressSaveData::GetSlotName(), 0);
	UGameplayStatics::DeleteGameInSlot(UCharacterStatSaveData::GetSlotName(), 0);
	UGameplayStatics::DeleteGameInSlot(UInventorySaveData::GetSlotName(), 0);
	if (UWorld* World = GetWorld())
	{
		RemoveFromParent();
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
		UGameplayStatics::OpenLevel(World, MainGameLevelName);
	}
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

void UMainMenuWidget::ShowMainMenu()
{
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