#include "InventoryWidget.h"
#include "../Character/SoulInventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (CachedSlotCount <= 0)
    {
        CachedSlotCount = DefaultSlotCount;
    }

    BuildGrid();
}

void UInventoryWidget::SetInventoryComponent(USoulInventoryComponent* InventoryComponent)
{
    if (CachedInventoryComponent.Get() == InventoryComponent)
    {
        return;
    }

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.RemoveAll(this);
    }

    CachedInventoryComponent = InventoryComponent;

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UInventoryWidget::OnInventoryChanged);
        RefreshInventory(CachedInventoryComponent->GetSlots(), CachedInventoryComponent->GetSlotCount());
    }
    else
    {
        CachedSlots.Reset();
        CachedSlotCount = DefaultSlotCount;
        BuildGrid();
    }
}

void UInventoryWidget::RefreshInventory(const TArray<FInventorySlot>& Slots, int32 TotalSlotCount)
{
    CachedSlots = Slots;
    CachedSlotCount = TotalSlotCount > 0 ? TotalSlotCount : DefaultSlotCount;

    BuildGrid();
}

void UInventoryWidget::BuildGrid()
{
    if (!GridPanel)
    {
        if (!WidgetTree)
        {
            return;
        }

        GridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("GridPanel"));
        WidgetTree->RootWidget = GridPanel;
    }

    ResetGrid();

    const int32 SlotCountToBuild = CachedSlotCount > 0 ? CachedSlotCount : DefaultSlotCount;
    const int32 Columns = GetColumnCount(SlotCountToBuild);

    for (int32 SlotIndex = 0; SlotIndex < SlotCountToBuild; ++SlotIndex)
    {
        const int32 Row = SlotIndex / Columns;
        const int32 Column = SlotIndex % Columns;
        AddSlotWidget(SlotIndex, Row, Column);
    }
}

void UInventoryWidget::ResetGrid()
{
    if (GridPanel)
    {
        GridPanel->ClearChildren();
    }
}

void UInventoryWidget::AddSlotWidget(int32 SlotIndex, int32 Row, int32 Column)
{
    if (!GridPanel || !WidgetTree)
    {
        return;
    }

    UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    if (!SlotBorder)
    {
        return;
    }

    SlotBorder->SetPadding(FMargin(8.0f));
    SlotBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.75f));
    SlotBorder->SetHorizontalAlignment(HAlign_Center);
    SlotBorder->SetVerticalAlignment(VAlign_Center);

    UTextBlock* SlotText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (SlotText)
    {
        SlotText->SetText(BuildSlotLabel(SlotIndex));
        SlotText->SetJustification(ETextJustify::Center);
        SlotBorder->SetContent(SlotText);
    }

    if (UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(SlotBorder, Row, Column))
    {
        GridSlot->SetHorizontalAlignment(HAlign_Fill);
        GridSlot->SetVerticalAlignment(VAlign_Fill);
    }
}

FText UInventoryWidget::BuildSlotLabel(int32 SlotIndex) const
{
    if (CachedSlots.IsValidIndex(SlotIndex) && !CachedSlots[SlotIndex].IsEmpty())
    {
        const FInventoryItem& Item = CachedSlots[SlotIndex].Item;
        const FText ItemName = GetInventoryItemDisplayName(Item.Type);

        if (Item.Definition.bStackable || Item.Definition.MaxStackSize > 1)
        {
            return FText::Format(NSLOCTEXT("InventoryWidget", "SlotItemWithCount", "{0} (x{1})"), ItemName, FText::AsNumber(Item.Quantity));
        }

        return ItemName;
    }

    return NSLOCTEXT("InventoryWidget", "SlotEmpty", "Empty");
}

int32 UInventoryWidget::GetColumnCount(int32 SlotCount) const
{
    if (DefaultColumns > 0)
    {
        return DefaultColumns;
    }

    const int32 ColumnCount = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(SlotCount)));
    return FMath::Max(1, ColumnCount);
}

void UInventoryWidget::OnInventoryChanged()
{
    if (CachedInventoryComponent.IsValid())
    {
        RefreshInventory(CachedInventoryComponent->GetSlots(), CachedInventoryComponent->GetSlotCount());
    }
}