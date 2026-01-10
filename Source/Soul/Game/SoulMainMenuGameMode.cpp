#include "SoulMainMenuGameMode.h"
#include "../UI/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"

ASoulMainMenuGameMode::ASoulMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = nullptr;
}

void ASoulMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ASoulMainMenuGameMode BeginPlay"));

	if (!MainMenuWidgetClass)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);

	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}