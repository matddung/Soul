#include "GameClearWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UGameClearWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Return)
	{
		Btn_Return->OnClicked.AddDynamic(this, &UGameClearWidget::OnReturnClicked);
	}
}

void UGameClearWidget::SetClearTimeText(const FText& InText)
{
	if (Txt_ClearTime)
	{
		Txt_ClearTime->SetText(InText);
	}
}

void UGameClearWidget::OnReturnClicked()
{
	OnReturnRequested.Broadcast();
}