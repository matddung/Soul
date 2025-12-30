#include "SoulCharacterStatWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void USoulCharacterStatWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    BindPlusButton(Btn_StrPlus, ECharacterStatType::STR);
    BindPlusButton(Btn_DexPlus, ECharacterStatType::DEX);
    BindPlusButton(Btn_VitPlus, ECharacterStatType::VIT);
    BindPlusButton(Btn_EndPlus, ECharacterStatType::END);

    BindMinusButton(Btn_StrMinus, ECharacterStatType::STR);
    BindMinusButton(Btn_DexMinus, ECharacterStatType::DEX);
    BindMinusButton(Btn_VitMinus, ECharacterStatType::VIT);
    BindMinusButton(Btn_EndMinus, ECharacterStatType::END);
}

void USoulCharacterStatWidget::RefreshStats(const USoulCharacterStatComponent* StatComponent)
{
    const bool bHasStat = StatComponent != nullptr;
    const int32 Level = bHasStat ? StatComponent->GetLevel() : 0;
    const int32 Souls = bHasStat ? StatComponent->Souls : 0;
    const int32 Cost = bHasStat ? StatComponent->GetCurrentInvestCost() : 0;
    const bool bCanInvest = bHasStat && Souls >= Cost;

    if (Txt_Level)
    {
        Txt_Level->SetText(FText::FromString(FString::Printf(TEXT("LV : %d"), Level)));
    }

    if (Txt_Souls)
    {
        Txt_Souls->SetText(FText::FromString(FString::Printf(TEXT("MySouls : %d"), Souls)));
    }

    if (Txt_InvestCost)
    {
        Txt_InvestCost->SetText(FText::FromString(FString::Printf(TEXT("NeedSouls : %d"), Cost)));
    }

    UpdateStatRow(Txt_STR, bHasStat ? StatComponent->STR : 0);
    UpdateStatRow(Txt_DEX, bHasStat ? StatComponent->DEX : 0);
    UpdateStatRow(Txt_VIT, bHasStat ? StatComponent->VIT : 0);
    UpdateStatRow(Txt_END, bHasStat ? StatComponent->END : 0);

    const FCharacterDerivedStats CurrentDerived = bHasStat ? StatComponent->GetCurrentDerivedStats() : FCharacterDerivedStats();
    const FCharacterDerivedStats STRPreview = bHasStat ? StatComponent->GetPreviewDerivedStats(ECharacterStatType::STR) : FCharacterDerivedStats();
    const FCharacterDerivedStats DEXPreview = bHasStat ? StatComponent->GetPreviewDerivedStats(ECharacterStatType::DEX) : FCharacterDerivedStats();
    const FCharacterDerivedStats VITPreview = bHasStat ? StatComponent->GetPreviewDerivedStats(ECharacterStatType::VIT) : FCharacterDerivedStats();
    const FCharacterDerivedStats ENDPreview = bHasStat ? StatComponent->GetPreviewDerivedStats(ECharacterStatType::END) : FCharacterDerivedStats();

    UpdateDerivedRow(Txt_MaxHP, TEXT("HP"), CurrentDerived.MaxHP, CurrentDerived.MaxHP);
    UpdateDerivedRow(Txt_Stamina, TEXT("Stamina"), CurrentDerived.MaxStamina, CurrentDerived.MaxStamina);
    UpdateDerivedRow(Txt_StaminaRegen, TEXT("Regen"), CurrentDerived.StaminaRegenRate, CurrentDerived.StaminaRegenRate);
    UpdateDerivedRow(Txt_SwordDamage, TEXT("Sword"), CurrentDerived.SwordDamage, CurrentDerived.SwordDamage);
    UpdateDerivedRow(Txt_GunDamage, TEXT("Gun"), CurrentDerived.GunDamage, CurrentDerived.GunDamage);

    UpdateDerivedRow(Txt_StrEffect, TEXT("Sword Damage"), CurrentDerived.SwordDamage, STRPreview.SwordDamage);
    UpdateDerivedRow(Txt_DexEffect, TEXT("Gun Damage"), CurrentDerived.GunDamage, DEXPreview.GunDamage);
    UpdateDerivedRow(Txt_VitEffect, TEXT("Max HP"), CurrentDerived.MaxHP, VITPreview.MaxHP);
    UpdateDerivedRow(Txt_VitEffect, TEXT("Max Stamina"), CurrentDerived.MaxStamina, VITPreview.MaxStamina);
    UpdateDerivedRow(Txt_VitEffect, TEXT("Stamina Regen Rate"), CurrentDerived.StaminaRegenRate, VITPreview.StaminaRegenRate);
    UpdateEnduranceRow(CurrentDerived, ENDPreview);

    UpdateButtonStates(StatComponent);
}

void USoulCharacterStatWidget::BindPlusButton(UButton* Button, ECharacterStatType StatType)
{
    if (!Button)
    {
        return;
    }

    switch (StatType)
    {
    case ECharacterStatType::STR: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnSTRPlusClicked); break;
    case ECharacterStatType::DEX: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnDEXPlusClicked); break;
    case ECharacterStatType::VIT: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnVITPlusClicked); break;
    case ECharacterStatType::END: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnENDPlusClicked); break;
    default: break;
    }
}

void USoulCharacterStatWidget::BindMinusButton(UButton* Button, ECharacterStatType StatType)
{
    if (!Button)
    {
        return;
    }

    switch (StatType)
    {
    case ECharacterStatType::STR: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnSTRMinusClicked); break;
    case ECharacterStatType::DEX: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnDEXMinusClicked); break;
    case ECharacterStatType::VIT: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnVITMinusClicked); break;
    case ECharacterStatType::END: Button->OnClicked.AddDynamic(this, &USoulCharacterStatWidget::OnENDMinusClicked); break;
    default: break;
    }
}

void USoulCharacterStatWidget::UpdateStatRow(UTextBlock* StatText, int32 Value) const
{
    if (StatText)
    {
        StatText->SetText(FText::AsNumber(Value));
    }
}

void USoulCharacterStatWidget::UpdateDerivedRow(UTextBlock* TextWidget, const FString& Label, float Current, float Next) const
{
    if (!TextWidget)
    {
        return;
    }

    if (FMath::IsNearlyEqual(Current, Next))
    {
        TextWidget->SetText(FText::FromString(FString::Printf(TEXT("%s %.1f"), *Label, Current)));
    }
    else
    {
        TextWidget->SetText(FText::FromString(FString::Printf(TEXT("%s %.1f -> %.1f"), *Label, Current, Next)));
    }
}

void USoulCharacterStatWidget::UpdateEnduranceRow(const FCharacterDerivedStats& Current, const FCharacterDerivedStats& Preview) const
{
    if (!Txt_EndEffect)
    {
        return;
    }

    const FString StaminaString = FString::Printf(TEXT("Stamina %.1f -> %.1f"), Current.MaxStamina, Preview.MaxStamina);
    const FString RegenString = FString::Printf(TEXT("Regen %.1f -> %.1f"), Current.StaminaRegenRate, Preview.StaminaRegenRate);
    Txt_EndEffect->SetText(FText::FromString(FString::Printf(TEXT("%s / %s"), *StaminaString, *RegenString)));
}

void USoulCharacterStatWidget::UpdateButtonStates(const USoulCharacterStatComponent* StatComponent)
{
    const bool bHasStat = StatComponent != nullptr;
    const bool bHasSoulsForInvest = bHasStat && StatComponent->Souls >= StatComponent->GetCurrentInvestCost();

    const int32 STRValue = bHasStat ? StatComponent->STR : 0;
    const int32 DEXValue = bHasStat ? StatComponent->DEX : 0;
    const int32 VITValue = bHasStat ? StatComponent->VIT : 0;
    const int32 ENDValue = bHasStat ? StatComponent->END : 0;

    SetButtonEnabled(Btn_StrPlus, bHasSoulsForInvest && STRValue < USoulCharacterStatComponent::MaxStatValue);
    SetButtonEnabled(Btn_DexPlus, bHasSoulsForInvest && DEXValue < USoulCharacterStatComponent::MaxStatValue);
    SetButtonEnabled(Btn_VitPlus, bHasSoulsForInvest && VITValue < USoulCharacterStatComponent::MaxStatValue);
    SetButtonEnabled(Btn_EndPlus, bHasSoulsForInvest && ENDValue < USoulCharacterStatComponent::MaxStatValue);

    SetButtonEnabled(Btn_StrMinus, bHasStat && STRValue > USoulCharacterStatComponent::MinStatValue);
    SetButtonEnabled(Btn_DexMinus, bHasStat && DEXValue > USoulCharacterStatComponent::MinStatValue);
    SetButtonEnabled(Btn_VitMinus, bHasStat && VITValue > USoulCharacterStatComponent::MinStatValue);
    SetButtonEnabled(Btn_EndMinus, bHasStat && ENDValue > USoulCharacterStatComponent::MinStatValue);
}

void USoulCharacterStatWidget::SetButtonEnabled(UButton* Button, bool bEnabled) const
{
    if (Button)
    {
        Button->SetIsEnabled(bEnabled);
    }
}

void USoulCharacterStatWidget::BroadcastAdjust(ECharacterStatType StatType, int32 Delta)
{
    OnRequestAdjustStat.Broadcast(StatType, Delta);
}

void USoulCharacterStatWidget::OnSTRPlusClicked()
{
    BroadcastAdjust(ECharacterStatType::STR, 1);
}

void USoulCharacterStatWidget::OnDEXPlusClicked()
{
    BroadcastAdjust(ECharacterStatType::DEX, 1);
}

void USoulCharacterStatWidget::OnVITPlusClicked()
{
    BroadcastAdjust(ECharacterStatType::VIT, 1);
}

void USoulCharacterStatWidget::OnENDPlusClicked()
{
    BroadcastAdjust(ECharacterStatType::END, 1);
}

void USoulCharacterStatWidget::OnSTRMinusClicked()
{
    BroadcastAdjust(ECharacterStatType::STR, -1);
}

void USoulCharacterStatWidget::OnDEXMinusClicked()
{
    BroadcastAdjust(ECharacterStatType::DEX, -1);
}

void USoulCharacterStatWidget::OnVITMinusClicked()
{
    BroadcastAdjust(ECharacterStatType::VIT, -1);
}

void USoulCharacterStatWidget::OnENDMinusClicked()
{
    BroadcastAdjust(ECharacterStatType::END, -1);
}