#include "InventoryWidget.h"
#include "../Character/SoulInventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

void UInventorySlotBorder::InitializeSlot(UInventoryWidget* InOwner, int32 InSlotIndex)
{
    Owner = InOwner;
    SlotIndex = InSlotIndex;

    OnMouseButtonDownEvent.BindDynamic(this, &UInventorySlotBorder::HandleMouseButtonDown);
}

FEventReply UInventorySlotBorder::HandleMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (Owner)
    {
        return Owner->HandleSlotMouseButtonDown(SlotIndex, MyGeometry, MouseEvent);
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

void UInventoryMenuEntryBorder::InitializeEntry(UInventoryWidget* InOwner, EInventoryContextAction InAction, int32 InQuickSlotNumber)
{
    Owner = InOwner;
    Action = InAction;
    QuickSlotNumber = InQuickSlotNumber;

    OnMouseButtonDownEvent.BindDynamic(this, &UInventoryMenuEntryBorder::HandleMouseButtonDown);
}

FEventReply UInventoryMenuEntryBorder::HandleMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (Owner)
    {
        return Owner->HandleMenuEntryAction(Action, QuickSlotNumber, MouseEvent);
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

void UInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (CachedSlotCount <= 0)
    {
        CachedSlotCount = DefaultSlotCount;
    }

    EnsureRootPanel();
    EnsureContextMenu();
    BuildGrid();
}

FReply UInventoryWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (ContextMenuBorder && ContextMenuBorder->GetVisibility() == ESlateVisibility::Visible)
    {
        const FGeometry& MenuGeometry = ContextMenuBorder->GetCachedGeometry();
        if (!MenuGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
        {
            HideContextMenu();
        }
    }

    return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
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
    EnsureRootPanel();
    EnsureContextMenu();

    if (!GridPanel)
    {
        if (!WidgetTree || !RootPanel)
        {
            return;
        }

        GridPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("GridPanel"));
        RootPanel->AddChild(GridPanel);
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

    UInventorySlotBorder* SlotBorder = WidgetTree->ConstructWidget<UInventorySlotBorder>(UInventorySlotBorder::StaticClass());
    if (!SlotBorder)
    {
        return;
    }

    SlotBorder->InitializeSlot(this, SlotIndex);

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

        return FText::Format(NSLOCTEXT("InventoryWidget", "SlotItemWithCount", "{0} ({1})"), ItemName, FText::AsNumber(Item.Quantity));
    }

    return NSLOCTEXT("InventoryWidget", "SlotEmpty", "Empty");
}

FEventReply UInventoryWidget::HandleSlotMouseButtonDown(int32 SlotIndex, const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        const bool bHasItem = CachedSlots.IsValidIndex(SlotIndex) && !CachedSlots[SlotIndex].IsEmpty();
        if (bHasItem)
        {
            ShowContextMenu(SlotIndex, MouseEvent.GetScreenSpacePosition());
            return UWidgetBlueprintLibrary::Handled();
        }

        HideContextMenu();
        return UWidgetBlueprintLibrary::Unhandled();
    }

    HideContextMenu();
    return UWidgetBlueprintLibrary::Unhandled();
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

void UInventoryWidget::EnsureRootPanel()
{
    if (RootPanel || !WidgetTree)
    {
        return;
    }

    RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootPanel"));

    if (RootPanel)
    {
        WidgetTree->RootWidget = RootPanel;
    }
}

UBorder* UInventoryWidget::CreateMenuEntry(const FText& Label, EInventoryContextAction Action, int32 QuickSlotNumber) const
{
    if (!WidgetTree)
    {
        return nullptr;
    }

    UInventoryMenuEntryBorder* Entry = WidgetTree->ConstructWidget<UInventoryMenuEntryBorder>(UInventoryMenuEntryBorder::StaticClass());
    if (!Entry)
    {
        return nullptr;
    }

    Entry->InitializeEntry(const_cast<UInventoryWidget*>(this), Action, QuickSlotNumber);

    Entry->SetPadding(FMargin(6.0f));
    Entry->SetBrushColor(FLinearColor(0.12f, 0.12f, 0.12f, 0.95f));
    Entry->SetHorizontalAlignment(HAlign_Fill);
    Entry->SetVerticalAlignment(VAlign_Center);

    UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (LabelText)
    {
        LabelText->SetText(Label);
        Entry->SetContent(LabelText);
    }

    return Entry;
}

FEventReply UInventoryWidget::HandleMenuEntryAction(EInventoryContextAction Action, int32 QuickSlotNumber, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return UWidgetBlueprintLibrary::Unhandled();
    }

    if (ContextMenuBorder && ContextMenuBorder->GetVisibility() != ESlateVisibility::Visible)
    {
        return UWidgetBlueprintLibrary::Unhandled();
    }

    if (Action == EInventoryContextAction::Use && UseMenuEntry && !UseMenuEntry->GetIsEnabled())
    {
        return UWidgetBlueprintLibrary::Unhandled();
    }

    if (Action == EInventoryContextAction::Remove && RemoveMenuEntry && !RemoveMenuEntry->GetIsEnabled())
    {
        return UWidgetBlueprintLibrary::Unhandled();
    }

    switch (Action)
    {
    case EInventoryContextAction::Use:
        HandleUseItem();
        break;
    case EInventoryContextAction::QuickSlotToggle:
        ToggleQuickSlotSelector();
        break;
    case EInventoryContextAction::Remove:
        HandleRemoveItem();
        break;
    case EInventoryContextAction::AssignQuickSlot:
        AssignQuickSlot(QuickSlotNumber);
        break;
    default:
        break;
    }

    return UWidgetBlueprintLibrary::Handled();
}

void UInventoryWidget::EnsureContextMenu()
{
    if (ContextMenuBorder || !WidgetTree)
    {
        return;
    }

    EnsureRootPanel();

    if (!RootPanel)
    {
        return;
    }

    ContextMenuBlocker = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContextMenuBlocker"));
    ContextMenuBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContextMenu"));
    if (!ContextMenuBlocker || !ContextMenuBorder)
    {
        return;
    }

    ContextMenuBlocker->OnMouseButtonDownEvent.BindDynamic(this, &UInventoryWidget::HandleMenuBlockerMouseButtonDown);
    ContextMenuBlocker->SetVisibility(ESlateVisibility::Collapsed);
    ContextMenuBlocker->SetHorizontalAlignment(HAlign_Fill);
    ContextMenuBlocker->SetVerticalAlignment(VAlign_Fill);
    ContextMenuBlocker->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));

    ContextMenuBorder->SetPadding(FMargin(6.0f));
    ContextMenuBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.95f));
    ContextMenuBorder->SetHorizontalAlignment(HAlign_Fill);
    ContextMenuBorder->SetVerticalAlignment(VAlign_Fill);
    ContextMenuBorder->SetVisibility(ESlateVisibility::Collapsed);

    ContextMenuBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContextMenuBox"));
    if (ContextMenuBox)
    {
        ContextMenuBorder->SetContent(ContextMenuBox);

        UseMenuEntry = Cast<UInventoryMenuEntryBorder>(CreateMenuEntry(NSLOCTEXT("InventoryWidget", "ContextUse", "Use"), EInventoryContextAction::Use));
        if (UseMenuEntry)
        {
            ContextMenuBox->AddChildToVerticalBox(UseMenuEntry);
        }

        if (UBorder* QuickSlotEntry = CreateMenuEntry(NSLOCTEXT("InventoryWidget", "ContextQuickSlot", "Quick Slot Registration"), EInventoryContextAction::QuickSlotToggle))
        {
            ContextMenuBox->AddChildToVerticalBox(QuickSlotEntry);
        }

        QuickSlotSelectorBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("QuickSlotSelector"));
        if (QuickSlotSelectorBox)
        {
            QuickSlotSelectorBox->SetVisibility(ESlateVisibility::Collapsed);
            ContextMenuBox->AddChildToVerticalBox(QuickSlotSelectorBox);
        }

        RemoveMenuEntry = Cast<UInventoryMenuEntryBorder>(CreateMenuEntry(NSLOCTEXT("InventoryWidget", "ContextDiscard", "Remove"), EInventoryContextAction::Remove));
        if (RemoveMenuEntry)
        {
            ContextMenuBox->AddChildToVerticalBox(RemoveMenuEntry);
        }
    }

    if (UCanvasPanelSlot* BlockerSlot = RootPanel->AddChildToCanvas(ContextMenuBlocker))
    {
        BlockerSlot->SetAutoSize(false);
        BlockerSlot->SetOffsets(FMargin(0.0f));
        BlockerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        BlockerSlot->SetZOrder(9);
    }

    if (UCanvasPanelSlot* CanvasSlot = RootPanel->AddChildToCanvas(ContextMenuBorder))
    {
        CanvasSlot->SetAutoSize(true);
        CanvasSlot->SetZOrder(10);
    }

    RefreshQuickSlotSelectors();
}

void UInventoryWidget::ShowContextMenu(int32 SlotIndex, const FVector2D& ScreenSpacePosition)
{
    EnsureContextMenu();

    if (!ContextMenuBorder || !RootPanel)
    {
        return;
    }

    CachedContextSlotIndex = SlotIndex;
    UpdateMenuEntryStates();

    if (QuickSlotSelectorBox)
    {
        QuickSlotSelectorBox->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ContextMenuBorder->Slot))
    {
        const FGeometry& RootGeometry = RootPanel->GetCachedGeometry();
        const FVector2D LocalPosition = RootGeometry.AbsoluteToLocal(ScreenSpacePosition);
        CanvasSlot->SetPosition(LocalPosition);
    }

    ContextMenuBorder->SetVisibility(ESlateVisibility::Visible);

    if (ContextMenuBlocker)
    {
        ContextMenuBlocker->SetVisibility(ESlateVisibility::Visible);
    }
}

void UInventoryWidget::HideContextMenu()
{
    if (ContextMenuBorder)
    {
        ContextMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ContextMenuBlocker)
    {
        ContextMenuBlocker->SetVisibility(ESlateVisibility::Collapsed);
    }

    ResetMenuSelection();
}

FEventReply UInventoryWidget::HandleMenuBlockerMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (ContextMenuBorder)
    {
        const FGeometry& MenuGeometry = ContextMenuBorder->GetCachedGeometry();
        if (MenuGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition()))
        {
            return UWidgetBlueprintLibrary::Unhandled();
        }
    }

    HideContextMenu();
    return UWidgetBlueprintLibrary::Handled();
}

void UInventoryWidget::RefreshQuickSlotSelectors()
{
    if (!QuickSlotSelectorBox)
    {
        return;
    }

    QuickSlotSelectorBox->ClearChildren();

    for (int32 SlotNumber = 1; SlotNumber <= QuickSlotCount; ++SlotNumber)
    {
        const FText Label = FText::Format(NSLOCTEXT("InventoryWidget", "QuickSlotEntry", "Quick Slot {0}"), FText::AsNumber(SlotNumber));

        if (UBorder* Entry = CreateMenuEntry(Label, EInventoryContextAction::AssignQuickSlot, SlotNumber))
        {
            QuickSlotSelectorBox->AddChildToVerticalBox(Entry);
        }
    }
}

void UInventoryWidget::HandleUseItem()
{
    if (CachedInventoryComponent.IsValid() && CachedContextSlotIndex != INDEX_NONE)
    {
        CachedInventoryComponent->UseItemAtIndex(CachedContextSlotIndex);
    }

    HideContextMenu();
}

void UInventoryWidget::HandleRemoveItem()
{
    if (CachedInventoryComponent.IsValid() && CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const int32 Quantity = CachedSlots[CachedContextSlotIndex].Item.Quantity;
        if (Quantity > 0)
        {
            CachedInventoryComponent->UseItemAtIndex(CachedContextSlotIndex, Quantity);
        }
    }

    HideContextMenu();
}

void UInventoryWidget::ToggleQuickSlotSelector()
{
    if (!QuickSlotSelectorBox)
    {
        return;
    }

    const bool bShouldShow = QuickSlotSelectorBox->GetVisibility() != ESlateVisibility::Visible;
    QuickSlotSelectorBox->SetVisibility(bShouldShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UInventoryWidget::AssignQuickSlot(int32 SlotNumber)
{
    UE_LOG(LogTemp, Log, TEXT("Assigning inventory slot %d to quick slot %d (stub)"), CachedContextSlotIndex, SlotNumber);
    HideContextMenu();
}

void UInventoryWidget::ResetMenuSelection()
{
    CachedContextSlotIndex = INDEX_NONE;
}

void UInventoryWidget::UpdateMenuEntryStates()
{
    bool bCanUse = false;
    bool bCanRemove = false;

    if (CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const FInventoryItem& Item = CachedSlots[CachedContextSlotIndex].Item;
        if (!Item.IsEmpty())
        {
            const bool bHasQuantity = Item.Quantity > 0;
            const bool bKeepWhenEmpty = Item.Definition.EmptyPolicy == EInventoryEmptyPolicy::KeepWhenEmpty;

            bCanUse = bHasQuantity;
            bCanRemove = bHasQuantity || (!bKeepWhenEmpty && Item.Definition.EmptyPolicy == EInventoryEmptyPolicy::RemoveWhenEmpty);

            if (bKeepWhenEmpty && !bHasQuantity)
            {
                bCanRemove = false;
            }
        }
    }

    if (UseMenuEntry)
    {
        UseMenuEntry->SetIsEnabled(bCanUse);
    }

    if (RemoveMenuEntry)
    {
        RemoveMenuEntry->SetIsEnabled(bCanRemove);
    }
}

void UInventoryWidget::OnInventoryChanged()
{
    if (CachedInventoryComponent.IsValid())
    {
        RefreshInventory(CachedInventoryComponent->GetSlots(), CachedInventoryComponent->GetSlotCount());
    }

    HideContextMenu();
}