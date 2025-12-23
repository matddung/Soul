#include "SoulPlayerController.h"
#include "../UI/CrosshairWidget.h"
#include "../UI/InteractPromptWidget.h"

#include "Blueprint/UserWidget.h"

void ASoulPlayerController::BeginPlay()
{
	Super::BeginPlay();

    if (CrosshairWidgetClass)
    {
        CrosshairWidget = CreateWidget<UCrosshairWidget>(this, CrosshairWidgetClass);
        if (CrosshairWidget)
        {
            CrosshairWidget->AddToViewport();
            CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void ASoulPlayerController::ShowCrosshair(bool bShow)
{
    if (!CrosshairWidget)
    {
        return;
    }

    CrosshairWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void ASoulPlayerController::OnCrosshairShot()
{
    if (CrosshairWidget)
    {
        CrosshairWidget->OnShot();
    }
}

void ASoulPlayerController::OnCrosshairReset()
{
    if (CrosshairWidget)
    {
        CrosshairWidget->OnReset();
    }
}

void ASoulPlayerController::ShowInteractPrompt(bool bShow, const FText& Text)
{
    if (bShow)
    {
        if (!InteractPromptWidget)
        {
            if (!InteractPromptClass)
            {
                return;
            }

            InteractPromptWidget = CreateWidget<UInteractPromptWidget>(this, InteractPromptClass);

            if (InteractPromptWidget)
            {
                InteractPromptWidget->AddToViewport(10);
            }
        }

        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Visible);
            InteractPromptWidget->SetPromptText(Text);
        }
    }
    else
    {
        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}