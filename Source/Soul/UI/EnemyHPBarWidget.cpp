#include "EnemyHPBarWidget.h"

#include "Components/ProgressBar.h"

void UEnemyHPBarWidget::SetHealth(float Current, float Max)
{
    if (HPProgressBar == nullptr)
    {
        return;
    }

    const float Percent = (Max > 0.0f) ? FMath::Clamp(Current / Max, 0.0f, 1.0f) : 0.0f;
    HPProgressBar->SetPercent(Percent);
}