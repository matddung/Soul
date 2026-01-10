#include "SoulMainMenuGameMode.h"
#include "../UI/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ASoulMainMenuGameMode::ASoulMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
}

void ASoulMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!MainMenuWidgetClass)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(PC, MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}