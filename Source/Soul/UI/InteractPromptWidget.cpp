#include "InteractPromptWidget.h"

#include "Components/TextBlock.h"

void UInteractPromptWidget::SetPromptText(const FText& InText)
{
    if (PromptText)
    {
        PromptText->SetText(InText);
    }
}