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
}

void USoulCharacterStatWidget::RefreshStats(const USoulCharacterStatComponent* StatComponent)
{
    if (!StatComponent)
    {
        SetButtonsEnabled(false);
        return;
    }

    const int32 Level = StatComponent->GetLevel();
    const int32 Souls = StatComponent->Souls;
    const int32 Cost = StatComponent->GetCurrentInvestCost();
    const bool bCanInvest = Souls >= Cost;

    if (Txt_Level)
    {
        Txt_Level->SetText(FText::AsNumber(Level));
    }

    if (Txt_Souls)
    {
        Txt_Souls->SetText(FText::AsNumber(Souls));
    }

    if (Txt_InvestCost)
    {
        Txt_InvestCost->SetText(FText::AsNumber(Cost));
    }

    UpdateStatRow(Txt_STR, StatComponent->STR);
    UpdateStatRow(Txt_DEX, StatComponent->DEX);
    UpdateStatRow(Txt_VIT, StatComponent->VIT);
    UpdateStatRow(Txt_END, StatComponent->END);

    const FCharacterDerivedStats CurrentDerived = StatComponent->GetCurrentDerivedStats();
    const FCharacterDerivedStats STRPreview = StatComponent->GetPreviewDerivedStats(ECharacterStatType::STR);
    const FCharacterDerivedStats DEXPreview = StatComponent->GetPreviewDerivedStats(ECharacterStatType::DEX);
    const FCharacterDerivedStats VITPreview = StatComponent->GetPreviewDerivedStats(ECharacterStatType::VIT);
    const FCharacterDerivedStats ENDPreview = StatComponent->GetPreviewDerivedStats(ECharacterStatType::END);

    UpdateDerivedRow(Txt_MaxHP, TEXT("HP"), CurrentDerived.MaxHP, CurrentDerived.MaxHP);
    UpdateDerivedRow(Txt_Stamina, TEXT("Stamina"), CurrentDerived.MaxStamina, CurrentDerived.MaxStamina);
    UpdateDerivedRow(Txt_StaminaRegen, TEXT("Regen"), CurrentDerived.StaminaRegenRate, CurrentDerived.StaminaRegenRate);
    UpdateDerivedRow(Txt_SwordDamage, TEXT("Sword"), CurrentDerived.SwordDamage, CurrentDerived.SwordDamage);
    UpdateDerivedRow(Txt_GunDamage, TEXT("Gun"), CurrentDerived.GunDamage, CurrentDerived.GunDamage);

    UpdateDerivedRow(Txt_StrEffect, TEXT("Sword Damage"), CurrentDerived.SwordDamage, STRPreview.SwordDamage);
    UpdateDerivedRow(Txt_DexEffect, TEXT("Gun Damage"), CurrentDerived.GunDamage, DEXPreview.GunDamage);
    UpdateDerivedRow(Txt_VitEffect, TEXT("Max HP"), CurrentDerived.MaxHP, VITPreview.MaxHP);
    UpdateDerivedRow(Txt_EndEffect, TEXT("Max Stamina"), CurrentDerived.MaxStamina, VITPreview.MaxStamina);
    UpdateEnduranceRow(CurrentDerived, ENDPreview);

    SetButtonsEnabled(bCanInvest);
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
        TextWidget->SetText(FText::FromString(FString::Printf(TEXT("%s %.1f ¡æ %.1f"), *Label, Current, Next)));
    }
}

void USoulCharacterStatWidget::UpdateEnduranceRow(const FCharacterDerivedStats& Current, const FCharacterDerivedStats& Preview) const
{
    if (!Txt_EndEffect)
    {
        return;
    }

    const FString StaminaString = FString::Printf(TEXT("Stamina%.1f ¡æ %.1f"), Current.MaxStamina, Preview.MaxStamina);
    const FString RegenString = FString::Printf(TEXT("Regen %.1f ¡æ %.1f"), Current.StaminaRegenRate, Preview.StaminaRegenRate);
    Txt_EndEffect->SetText(FText::FromString(FString::Printf(TEXT("%s / %s"), *StaminaString, *RegenString)));
}

void USoulCharacterStatWidget::SetButtonsEnabled(bool bEnabled)
{
    if (Btn_StrPlus)
    {
        Btn_StrPlus->SetIsEnabled(bEnabled);
    }
    if (Btn_DexPlus)
    {
        Btn_DexPlus->SetIsEnabled(bEnabled);
    }
    if (Btn_VitPlus)
    {
        Btn_VitPlus->SetIsEnabled(bEnabled);
    }
    if (Btn_EndPlus)
    {
        Btn_EndPlus->SetIsEnabled(bEnabled);
    }
}

void USoulCharacterStatWidget::BroadcastInvest(ECharacterStatType StatType)
{
    OnRequestInvestStat.Broadcast(StatType);
}

void USoulCharacterStatWidget::OnSTRPlusClicked()
{
    BroadcastInvest(ECharacterStatType::STR);
}

void USoulCharacterStatWidget::OnDEXPlusClicked()
{
    BroadcastInvest(ECharacterStatType::DEX);
}

void USoulCharacterStatWidget::OnVITPlusClicked()
{
    BroadcastInvest(ECharacterStatType::VIT);
}

void USoulCharacterStatWidget::OnENDPlusClicked()
{
    BroadcastInvest(ECharacterStatType::END);
}