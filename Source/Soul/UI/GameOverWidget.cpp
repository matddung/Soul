#include "GameOverWidget.h"

#include "Components/Button.h"

void UGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Retry)
	{
		Btn_Retry->OnClicked.AddDynamic(this, &UGameOverWidget::OnRetryClicked);
	}

	if (Btn_Main)
	{
		Btn_Main->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainClicked);
	}
}

void UGameOverWidget::OnRetryClicked()
{
	OnRetryRequested.Broadcast();
}

void UGameOverWidget::OnMainClicked()
{
	OnMainRequested.Broadcast();
}