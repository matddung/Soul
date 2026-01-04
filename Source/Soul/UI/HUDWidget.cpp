#include "HUDWidget.h"
#include "../Character/SoulCharacterStatComponent.h"
#include "../Character/SoulInventoryComponent.h"
#include "../Character/InventoryItem.h"

#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RefreshStats();
    RefreshQuickSlots();
}

void UHUDWidget::InitializeWidget(USoulCharacterStatComponent* StatComponent, USoulInventoryComponent* InventoryComponent)
{
    SetStatComponent(StatComponent);
    SetInventoryComponent(InventoryComponent);

    RefreshStats();
    RefreshQuickSlots();
}

void UHUDWidget::SetStatComponent(USoulCharacterStatComponent* StatComponent)
{
    if (CachedStatComponent.Get() == StatComponent)
    {
        return;
    }

    if (CachedStatComponent.IsValid())
    {
        CachedStatComponent->OnStatChanged.RemoveDynamic(this, &UHUDWidget::HandleStatChanged);
    }

    CachedStatComponent = StatComponent;

    if (CachedStatComponent.IsValid())
    {
        CachedStatComponent->OnStatChanged.AddDynamic(this, &UHUDWidget::HandleStatChanged);
    }
}

void UHUDWidget::SetInventoryComponent(USoulInventoryComponent* InventoryComponent)
{
    if (CachedInventoryComponent.Get() == InventoryComponent)
    {
        return;
    }

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
        CachedInventoryComponent->OnQuickSlotChanged.RemoveAll(this);
    }

    CachedInventoryComponent = InventoryComponent;

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UHUDWidget::RefreshQuickSlots);
        CachedInventoryComponent->OnQuickSlotChanged.AddUObject(this, &UHUDWidget::RefreshQuickSlots);
    }
}

void UHUDWidget::RefreshStats()
{
    if (!CachedStatComponent.IsValid())
    {
        UpdateBar(Box_Health, Prg_Health, 0.f, 1.f);
        UpdateBar(Box_Stamina, Prg_Stamina, 0.f, 1.f);
        return;
    }

    UpdateBar(Box_Health, Prg_Health, CachedStatComponent->GetHP(), CachedStatComponent->GetMaxHP());
    UpdateBar(Box_Stamina, Prg_Stamina, CachedStatComponent->GetStamina(), CachedStatComponent->GetMaxStamina());
}

void UHUDWidget::RefreshQuickSlots()
{
    if (!CachedInventoryComponent.IsValid())
    {
        if (Txt_QuickSlotA)
        {
            Txt_QuickSlotA->SetText(FText::GetEmpty());
        }

        if (Txt_QuickSlotB)
        {
            Txt_QuickSlotB->SetText(FText::GetEmpty());
        }

        return;
    }

    if (Txt_QuickSlotA)
    {
        Txt_QuickSlotA->SetText(BuildQuickSlotLabel(0));
    }

    if (Txt_QuickSlotB)
    {
        Txt_QuickSlotB->SetText(BuildQuickSlotLabel(1));
    }
}

void UHUDWidget::HandleStatChanged()
{
    RefreshStats();
}

void UHUDWidget::UpdateBar(USizeBox* Box, UProgressBar* Bar, float Current, float Max) const
{
    if (Box)
    {
        Box->SetWidthOverride(FMath::Max(0.f, Current));
    }

    if (Bar)
    {
        const float Percent = Max > 0.f ? Current / Max : 0.f;
        Bar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
    }
}

FText UHUDWidget::BuildQuickSlotLabel(int32 QuickSlotOffset) const
{
    const TArray<int32>& QuickSlots = CachedInventoryComponent->GetQuickSlotAssignments();
    const int32 QuickSlotCount = QuickSlots.Num();

    if (QuickSlotCount <= 0)
    {
        return FText::GetEmpty();
    }

    const int32 ActiveIndex = CachedInventoryComponent->GetActiveQuickSlotIndex();
    const int32 SlotArrayIndex = (ActiveIndex + QuickSlotOffset + 1) % QuickSlotCount;
    const int32 InventoryIndex = QuickSlots.IsValidIndex(SlotArrayIndex) ? QuickSlots[SlotArrayIndex] : INDEX_NONE;
    const TArray<FInventorySlot>& Slots = CachedInventoryComponent->GetSlots();

    if (!Slots.IsValidIndex(InventoryIndex) || Slots[InventoryIndex].IsEmpty())
    {
        return NSLOCTEXT("SoulHUD", "EmptyQuickSlot", "Empty");
    }

    const FInventoryItem& Item = Slots[InventoryIndex].Item;
    const FText ItemName = GetInventoryItemDisplayName(Item.Type);

    if (Item.Quantity > 1)
    {
        const FText QuantityText = FText::AsNumber(Item.Quantity);
        return FText::Format(NSLOCTEXT("SoulHUD", "QuickSlotLabel", "{0} x{1}"), ItemName, QuantityText);
    }

    return ItemName;
}