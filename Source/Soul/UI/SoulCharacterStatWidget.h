#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Character/SoulCharacterStatComponent.h"
#include "SoulCharacterStatWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestAdjustStat, ECharacterStatType, StatType, int32, Delta);

UCLASS()
class SOUL_API USoulCharacterStatWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RefreshStats(const USoulCharacterStatComponent* StatComponent);

    UPROPERTY(BlueprintAssignable, Category = "Stats")
    FOnRequestAdjustStat OnRequestAdjustStat;

protected:
    virtual void NativeOnInitialized() override;

    void BindPlusButton(UButton* Button, ECharacterStatType StatType);
    void BindMinusButton(UButton* Button, ECharacterStatType StatType);
    void UpdateStatRow(UTextBlock* StatText, int32 Value) const;
    void UpdateDerivedRow(UTextBlock* TextWidget, const FString& Label, float Current, float Next) const;
    void UpdateEnduranceRow(const FCharacterDerivedStats& Current, const FCharacterDerivedStats& Preview) const;
    void UpdateButtonStates(const USoulCharacterStatComponent* StatComponent);
    void SetButtonEnabled(UButton* Button, bool bEnabled) const;
    void BroadcastAdjust(ECharacterStatType StatType, int32 Delta);

    UFUNCTION()
    void OnSTRPlusClicked();

    UFUNCTION()
    void OnDEXPlusClicked();

    UFUNCTION()
    void OnVITPlusClicked();

    UFUNCTION()
    void OnENDPlusClicked();

    UFUNCTION()
    void OnSTRMinusClicked();

    UFUNCTION()
    void OnDEXMinusClicked();

    UFUNCTION()
    void OnVITMinusClicked();

    UFUNCTION()
    void OnENDMinusClicked();

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Level;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Souls;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_InvestCost;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_STR;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_DEX;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_VIT;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_END;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_MaxHP;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_Stamina;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_StaminaRegen;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_SwordDamage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Txt_GunDamage;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_StrEffect;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_DexEffect;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_VitEffect;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* Txt_EndEffect;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_StrPlus;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_DexPlus;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_VitPlus;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_EndPlus;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Btn_StrMinus;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Btn_DexMinus;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Btn_VitMinus;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* Btn_EndMinus;
};