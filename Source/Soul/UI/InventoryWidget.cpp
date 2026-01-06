#include "InventoryWidget.h"
#include "../Character/SoulInventoryComponent.h"
#include "../Character/SoulCharacter.h"
#include "../Character/SoulWeaponComponent.h"
#include "../Character/SoulCharacterStatComponent.h"

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
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/SizeBox.h"
#include "Layout/Clipping.h"

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
        CachedInventoryComponent->OnQuickSlotChanged.RemoveAll(this);
    }

    CachedInventoryComponent = InventoryComponent;

    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->OnInventoryChanged.AddUObject(this, &UInventoryWidget::OnInventoryChanged);
        CachedInventoryComponent->OnQuickSlotChanged.AddUObject(this, &UInventoryWidget::RefreshQuickSlotSelectors);
        QuickSlotCount = CachedInventoryComponent->GetQuickSlotCount();
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
    EnsureRemoveQuantityDialog();
    EnsureEnhancementDialog();

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
        SlotText->SetAutoWrapText(true);
        SlotText->SetWrapTextAt(200.f);
        SlotText->SetClipping(EWidgetClipping::ClipToBounds);
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

        QuickSlotMenuEntry = Cast<UInventoryMenuEntryBorder>(CreateMenuEntry(NSLOCTEXT("InventoryWidget", "ContextQuickSlot", "Quick Slot Registration"), EInventoryContextAction::QuickSlotToggle));
        if (QuickSlotMenuEntry)
        {
            ContextMenuBox->AddChildToVerticalBox(QuickSlotMenuEntry);
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

void UInventoryWidget::EnsureRemoveQuantityDialog()
{
    if (RemoveQuantityBorder || !WidgetTree)
    {
        return;
    }

    EnsureRootPanel();

    if (!RootPanel)
    {
        return;
    }

    RemoveDialogBlocker = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RemoveDialogBlocker"));
    RemoveQuantityBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RemoveQuantityDialog"));

    if (!RemoveDialogBlocker || !RemoveQuantityBorder)
    {
        return;
    }

    RemoveDialogBlocker->OnMouseButtonDownEvent.BindDynamic(this, &UInventoryWidget::HandleRemoveDialogBlockerMouseButtonDown);
    RemoveDialogBlocker->SetVisibility(ESlateVisibility::Collapsed);
    RemoveDialogBlocker->SetHorizontalAlignment(HAlign_Fill);
    RemoveDialogBlocker->SetVerticalAlignment(VAlign_Fill);
    RemoveDialogBlocker->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));

    RemoveQuantityBorder->SetPadding(FMargin(16.0f, 14.0f));
    RemoveQuantityBorder->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.95f));
    RemoveQuantityBorder->SetHorizontalAlignment(HAlign_Center);
    RemoveQuantityBorder->SetVerticalAlignment(VAlign_Center);
    RemoveQuantityBorder->SetVisibility(ESlateVisibility::Collapsed);

    RemoveQuantityBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RemoveQuantityBox"));
    if (RemoveQuantityBox)
    {
        RemoveQuantityBorder->SetContent(RemoveQuantityBox);

        if (UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
        {
            TitleText->SetText(NSLOCTEXT("InventoryWidget", "RemoveDialogTitle", "Remove Items"));
            TitleText->SetJustification(ETextJustify::Center);

            if (UVerticalBoxSlot* TitleSlot = RemoveQuantityBox->AddChildToVerticalBox(TitleText))
            {
                TitleSlot->SetHorizontalAlignment(HAlign_Center);
                TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
            }
        }

        if (UTextBlock* InstructionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
        {
            InstructionText->SetText(NSLOCTEXT("InventoryWidget", "RemoveDialogInstruction", "Quantity"));
            InstructionText->SetJustification(ETextJustify::Center);

            if (UVerticalBoxSlot* InstructionSlot = RemoveQuantityBox->AddChildToVerticalBox(InstructionText))
            {
                InstructionSlot->SetHorizontalAlignment(HAlign_Center);
                InstructionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
            }
        }

        RemoveQuantityValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (RemoveQuantityValueText)
        {
            RemoveQuantityValueText->SetJustification(ETextJustify::Center);

            if (UVerticalBoxSlot* QuantitySlot = RemoveQuantityBox->AddChildToVerticalBox(RemoveQuantityValueText))
            {
                QuantitySlot->SetHorizontalAlignment(HAlign_Center);
                QuantitySlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 8.0f));
            }
        }

        if (UHorizontalBox* AdjustBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass()))
        {
            auto AddAdjustButton = [&](const FText& Label, void (UInventoryWidget::* Handler)())
                {
                    if (UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass()))
                    {
                        if (Handler == &UInventoryWidget::HandleRemoveMinusTen)
                        {
                            Button->OnClicked.AddDynamic(this, &UInventoryWidget::HandleRemoveMinusTen);
                        }
                        else if (Handler == &UInventoryWidget::HandleRemoveMinusOne)
                        {
                            Button->OnClicked.AddDynamic(this, &UInventoryWidget::HandleRemoveMinusOne);
                        }
                        else if (Handler == &UInventoryWidget::HandleRemovePlusOne)
                        {
                            Button->OnClicked.AddDynamic(this, &UInventoryWidget::HandleRemovePlusOne);
                        }
                        else if (Handler == &UInventoryWidget::HandleRemovePlusTen)
                        {
                            Button->OnClicked.AddDynamic(this, &UInventoryWidget::HandleRemovePlusTen);
                        }

                        if (UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
                        {
                            ButtonText->SetText(Label);
                            ButtonText->SetJustification(ETextJustify::Center);
                            Button->SetContent(ButtonText);
                        }

                        if (UHorizontalBoxSlot* Slot = AdjustBox->AddChildToHorizontalBox(Button))
                        {
                            Slot->SetPadding(FMargin(4.0f, 8.0f));
                        }
                    }
                };

            AddAdjustButton(NSLOCTEXT("InventoryWidget", "RemoveMinusTen", "-10"), &UInventoryWidget::HandleRemoveMinusTen);
            AddAdjustButton(NSLOCTEXT("InventoryWidget", "RemoveMinusOne", "-1"), &UInventoryWidget::HandleRemoveMinusOne);
            AddAdjustButton(NSLOCTEXT("InventoryWidget", "RemovePlusOne", "+1"), &UInventoryWidget::HandleRemovePlusOne);
            AddAdjustButton(NSLOCTEXT("InventoryWidget", "RemovePlusTen", "+10"), &UInventoryWidget::HandleRemovePlusTen);

            if (UVerticalBoxSlot* AdjustSlot = RemoveQuantityBox->AddChildToVerticalBox(AdjustBox))
            {
                AdjustSlot->SetHorizontalAlignment(HAlign_Center);
                AdjustSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 6.0f));
            }
        }

        if (USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass()))
        {
            Spacer->SetSize(FVector2D(1.0f, 1.0f));

            if (UVerticalBoxSlot* SpacerSlot = RemoveQuantityBox->AddChildToVerticalBox(Spacer))
            {
                SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            }
        }

        if (UHorizontalBox* ConfirmationBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass()))
        {
            if (UButton* ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass()))
            {
                ConfirmButton->OnClicked.AddDynamic(this, &UInventoryWidget::ConfirmRemoveQuantity);

                if (UTextBlock* ConfirmText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
                {
                    ConfirmText->SetText(NSLOCTEXT("InventoryWidget", "RemoveConfirm", "Check"));
                    ConfirmText->SetJustification(ETextJustify::Center);
                    ConfirmButton->SetContent(ConfirmText);
                }

                if (UHorizontalBoxSlot* HBSlot = ConfirmationBox->AddChildToHorizontalBox(ConfirmButton))
                {
                    HBSlot->SetPadding(FMargin(6.0f, 10.0f, 4.0f, 0.0f));
                }
            }

            if (UButton* CancelButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass()))
            {
                CancelButton->OnClicked.AddDynamic(this, &UInventoryWidget::CancelRemoveQuantity);

                if (UTextBlock* CancelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
                {
                    CancelText->SetText(NSLOCTEXT("InventoryWidget", "RemoveCancel", "Cancel"));
                    CancelText->SetJustification(ETextJustify::Center);
                    CancelButton->SetContent(CancelText);
                }

                if (UHorizontalBoxSlot* HBSlot = ConfirmationBox->AddChildToHorizontalBox(CancelButton))
                {
                    HBSlot->SetPadding(FMargin(4.0f, 10.0f, 6.0f, 0.0f));
                }
            }

            if (UVerticalBoxSlot* ConfirmationSlot = RemoveQuantityBox->AddChildToVerticalBox(ConfirmationBox))
            {
                ConfirmationSlot->SetHorizontalAlignment(HAlign_Center);
                ConfirmationSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
            }
        }
    }

    if (UCanvasPanelSlot* BlockerSlot = RootPanel->AddChildToCanvas(RemoveDialogBlocker))
    {
        BlockerSlot->SetAutoSize(false);
        BlockerSlot->SetOffsets(FMargin(0.0f));
        BlockerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        BlockerSlot->SetZOrder(19);
    }

    if (UCanvasPanelSlot* DialogSlot = RootPanel->AddChildToCanvas(RemoveQuantityBorder))
    {
        DialogSlot->SetAutoSize(false);
        DialogSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
        DialogSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        DialogSlot->SetSize(FVector2D(520.0f, 320.0f));
        DialogSlot->SetZOrder(20);
    }
}

void UInventoryWidget::EnsureEnhancementDialog()
{
    if (EnhancementDialogBorder || !WidgetTree)
    {
        return;
    }

    EnsureRootPanel();

    if (!RootPanel)
    {
        return;
    }

    EnhancementDialogBlocker = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EnhancementDialogBlocker"));
    EnhancementDialogBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EnhancementDialog"));

    if (!EnhancementDialogBlocker || !EnhancementDialogBorder)
    {
        return;
    }

    EnhancementDialogBlocker->OnMouseButtonDownEvent.BindDynamic(this, &UInventoryWidget::HandleEnhancementDialogBlockerMouseButtonDown);
    EnhancementDialogBlocker->SetVisibility(ESlateVisibility::Collapsed);
    EnhancementDialogBlocker->SetHorizontalAlignment(HAlign_Fill);
    EnhancementDialogBlocker->SetVerticalAlignment(VAlign_Fill);
    EnhancementDialogBlocker->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));

    EnhancementDialogBorder->SetPadding(FMargin(24.0f, 20.0f));
    EnhancementDialogBorder->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.04f, 0.95f));
    EnhancementDialogBorder->SetHorizontalAlignment(HAlign_Center);
    EnhancementDialogBorder->SetVerticalAlignment(VAlign_Center);
    EnhancementDialogBorder->SetVisibility(ESlateVisibility::Collapsed);

    if (UVerticalBox* DialogBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EnhancementDialogContent")))
    {
        EnhancementDialogBorder->SetContent(DialogBox);

        if (UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
        {
            TitleText->SetText(NSLOCTEXT("InventoryWidget", "EnhancementDialogTitle", "Select Weapon to Enhance"));
            TitleText->SetJustification(ETextJustify::Center);

            FSlateFontInfo TitleFont = TitleText->GetFont();
            TitleFont.Size = 22;
            TitleText->SetFont(TitleFont);

            if (UVerticalBoxSlot* TitleSlot = DialogBox->AddChildToVerticalBox(TitleText))
            {
                TitleSlot->SetHorizontalAlignment(HAlign_Center);
                TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
            }
        }

        EnhancementChoiceBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("EnhancementChoice"));
        if (EnhancementChoiceBox)
        {
            auto CreateEnhanceButton = [&](const FText& Label, UButton*& OutButton)
                {
                    USizeBox* ButtonContainer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
                    if (!ButtonContainer)
                    {
                        return;
                    }

                    ButtonContainer->SetWidthOverride(160.0f);
                    ButtonContainer->SetHeightOverride(70.0f);

                    OutButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
                    if (!OutButton)
                    {
                        return;
                    }

                    if (UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()))
                    {
                        ButtonText->SetText(Label);
                        ButtonText->SetJustification(ETextJustify::Center);

                        FSlateFontInfo ButtonFont = ButtonText->GetFont();
                        ButtonFont.Size = 20;
                        ButtonText->SetFont(ButtonFont);
                        OutButton->SetContent(ButtonText);
                    }

                    ButtonContainer->AddChild(OutButton);

                    if (UHorizontalBoxSlot* Slot = EnhancementChoiceBox->AddChildToHorizontalBox(ButtonContainer))
                    {
                        Slot->SetPadding(FMargin(12.0f, 4.0f));
                        Slot->SetHorizontalAlignment(HAlign_Center);
                        Slot->SetVerticalAlignment(VAlign_Center);
                    }
                };

            CreateEnhanceButton(NSLOCTEXT("InventoryWidget", "EnhanceSword", "Sword"), SwordEnhanceButton);
            CreateEnhanceButton(NSLOCTEXT("InventoryWidget", "EnhanceGun", "Gun"), GunEnhanceButton);

            if (UVerticalBoxSlot* ButtonsSlot = DialogBox->AddChildToVerticalBox(EnhancementChoiceBox))
            {
                ButtonsSlot->SetHorizontalAlignment(HAlign_Center);
                ButtonsSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
            }
        }
    }

    if (UCanvasPanelSlot* BlockerSlot = RootPanel->AddChildToCanvas(EnhancementDialogBlocker))
    {
        BlockerSlot->SetAutoSize(false);
        BlockerSlot->SetOffsets(FMargin(0.0f));
        BlockerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        BlockerSlot->SetZOrder(19);
    }

    if (UCanvasPanelSlot* DialogSlot = RootPanel->AddChildToCanvas(EnhancementDialogBorder))
    {
        DialogSlot->SetAutoSize(false);
        DialogSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
        DialogSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        DialogSlot->SetSize(FVector2D(540.0f, 260.0f));
        DialogSlot->SetZOrder(20);
    }
}

void UInventoryWidget::ShowRemoveQuantityDialog(int32 MaxQuantity)
{
    EnsureRemoveQuantityDialog();

    if (!RemoveQuantityBorder || !RemoveDialogBlocker)
    {
        return;
    }

    MaxRemoveQuantity = FMath::Max(1, MaxQuantity);
    SelectedRemoveQuantity = 1;

    UpdateRemoveQuantityDisplay();

    RemoveQuantityBorder->SetVisibility(ESlateVisibility::Visible);
    RemoveDialogBlocker->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryWidget::HideRemoveQuantityDialog()
{
    if (RemoveQuantityBorder)
    {
        RemoveQuantityBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (RemoveDialogBlocker)
    {
        RemoveDialogBlocker->SetVisibility(ESlateVisibility::Collapsed);
    }

    ResetMenuSelection();
}

void UInventoryWidget::UpdateRemoveQuantityDisplay()
{
    if (!RemoveQuantityValueText)
    {
        return;
    }

    const FText DisplayText = FText::Format(NSLOCTEXT("InventoryWidget", "RemoveQuantityLabel", "{0} / {1}"), FText::AsNumber(SelectedRemoveQuantity), FText::AsNumber(MaxRemoveQuantity));
    RemoveQuantityValueText->SetText(DisplayText);
}

void UInventoryWidget::AdjustRemoveQuantity(int32 Delta)
{
    SelectedRemoveQuantity = FMath::Clamp(SelectedRemoveQuantity + Delta, 1, MaxRemoveQuantity);
    UpdateRemoveQuantityDisplay();
}

void UInventoryWidget::ShowEnhancementDialog()
{
    EnsureEnhancementDialog();

    if (!EnhancementDialogBorder || !EnhancementDialogBlocker)
    {
        return;
    }

    if (ContextMenuBorder)
    {
        ContextMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ContextMenuBlocker)
    {
        ContextMenuBlocker->SetVisibility(ESlateVisibility::Collapsed);
    }

    ASoulCharacter* OwnerCharacter = CachedInventoryComponent.IsValid() ? Cast<ASoulCharacter>(CachedInventoryComponent->GetOwner()) : nullptr;
    USoulWeaponComponent* WeaponComponent = OwnerCharacter ? OwnerCharacter->FindComponentByClass<USoulWeaponComponent>() : nullptr;
    USoulCharacterStatComponent* StatComponent = OwnerCharacter ? OwnerCharacter->FindComponentByClass<USoulCharacterStatComponent>() : nullptr;

    const bool bHasSword = WeaponComponent && WeaponComponent->HasWeapon(EWeaponType::Sword);
    const bool bHasGun = WeaponComponent && WeaponComponent->HasWeapon(EWeaponType::Gun);

    const bool bCanEnhanceSword = bHasSword && StatComponent && StatComponent->GetSwordEnhancementLevel() < USoulCharacterStatComponent::MaxWeaponEnhancementLevel;
    const bool bCanEnhanceGun = bHasGun && StatComponent && StatComponent->GetGunEnhancementLevel() < USoulCharacterStatComponent::MaxWeaponEnhancementLevel;

    if (SwordEnhanceButton)
    {
        SwordEnhanceButton->SetIsEnabled(bCanEnhanceSword);
        SwordEnhanceButton->OnClicked.RemoveAll(this);
        SwordEnhanceButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleEnhanceSword);
    }

    if (GunEnhanceButton)
    {
        GunEnhanceButton->SetIsEnabled(bCanEnhanceGun);
        GunEnhanceButton->OnClicked.RemoveAll(this);
        GunEnhanceButton->OnClicked.AddDynamic(this, &UInventoryWidget::HandleEnhanceGun);
    }

    EnhancementDialogBorder->SetVisibility(ESlateVisibility::Visible);
    EnhancementDialogBlocker->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryWidget::HideEnhancementDialog()
{
    if (EnhancementDialogBorder)
    {
        EnhancementDialogBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (EnhancementDialogBlocker)
    {
        EnhancementDialogBlocker->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventoryWidget::HandleEnhanceWeapon(EWeaponType WeaponType)
{
    ASoulCharacter* OwnerCharacter = CachedInventoryComponent.IsValid() ? Cast<ASoulCharacter>(CachedInventoryComponent->GetOwner()) : nullptr;

    if (OwnerCharacter && OwnerCharacter->EnhanceWeapon(WeaponType))
    {
        if (CachedInventoryComponent.IsValid() && CachedSlots.IsValidIndex(CachedContextSlotIndex))
        {
            CachedInventoryComponent->UseItemAtIndex(CachedContextSlotIndex);
        }
    }

    HideEnhancementDialog();
    HideContextMenu();
}

void UInventoryWidget::HandleEnhanceSword()
{
    HandleEnhanceWeapon(EWeaponType::Sword);
}

void UInventoryWidget::HandleEnhanceGun()
{
    HandleEnhanceWeapon(EWeaponType::Gun);
}

void UInventoryWidget::HandleRemoveMinusTen()
{
    AdjustRemoveQuantity(-10);
}

void UInventoryWidget::HandleRemoveMinusOne()
{
    AdjustRemoveQuantity(-1);
}

void UInventoryWidget::HandleRemovePlusOne()
{
    AdjustRemoveQuantity(1);
}

void UInventoryWidget::HandleRemovePlusTen()
{
    AdjustRemoveQuantity(10);
}

void UInventoryWidget::ConfirmRemoveQuantity()
{
    if (CachedInventoryComponent.IsValid() && CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const int32 QuantityToRemove = FMath::Clamp(SelectedRemoveQuantity, 1, MaxRemoveQuantity);
        CachedInventoryComponent->UseItemAtIndex(CachedContextSlotIndex, QuantityToRemove);
    }

    HideRemoveQuantityDialog();
    HideContextMenu();
}

void UInventoryWidget::CancelRemoveQuantity()
{
    HideRemoveQuantityDialog();
    HideContextMenu();
}

void UInventoryWidget::ShowContextMenu(int32 SlotIndex, const FVector2D& ScreenSpacePosition)
{
    EnsureContextMenu();
    HideRemoveQuantityDialog();

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

void UInventoryWidget::HideContextMenu(bool bResetSelection /*= true*/)
{
    if (ContextMenuBorder)
    {
        ContextMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ContextMenuBlocker)
    {
        ContextMenuBlocker->SetVisibility(ESlateVisibility::Collapsed);
    }

    HideEnhancementDialog();

    if (bResetSelection)
    {
        ResetMenuSelection();
    }
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

FEventReply UInventoryWidget::HandleRemoveDialogBlockerMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (RemoveQuantityBorder)
    {
        const FGeometry& DialogGeometry = RemoveQuantityBorder->GetCachedGeometry();
        if (DialogGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition()))
        {
            return UWidgetBlueprintLibrary::Unhandled();
        }
    }

    CancelRemoveQuantity();
    return UWidgetBlueprintLibrary::Handled();
}

FEventReply UInventoryWidget::HandleEnhancementDialogBlockerMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (EnhancementDialogBorder)
    {
        const FGeometry& DialogGeometry = EnhancementDialogBorder->GetCachedGeometry();
        if (DialogGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition()))
        {
            return UWidgetBlueprintLibrary::Unhandled();
        }
    }

    HideEnhancementDialog();
    HideContextMenu();
    return UWidgetBlueprintLibrary::Handled();
}

void UInventoryWidget::RefreshQuickSlotSelectors()
{
    if (!QuickSlotSelectorBox)
    {
        return;
    }

    if (CachedInventoryComponent.IsValid())
    {
        QuickSlotCount = CachedInventoryComponent->GetQuickSlotCount();
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
    if (CachedInventoryComponent.IsValid() && CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const FInventoryItem& Item = CachedSlots[CachedContextSlotIndex].Item;
        if (Item.Type == EInventoryItemType::EnhancementStone)
        {
            ShowEnhancementDialog();
            return;
        }

        CachedInventoryComponent->UseItemAtIndex(CachedContextSlotIndex);
    }

    HideContextMenu();
}

void UInventoryWidget::HandleRemoveItem()
{
    if (CachedInventoryComponent.IsValid() && CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const int32 Quantity = CachedSlots[CachedContextSlotIndex].Item.Quantity;

        if (Quantity > 1)
        {
            ShowRemoveQuantityDialog(Quantity);
            HideContextMenu(false);
            return;
        }

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
    if (CachedInventoryComponent.IsValid())
    {
        CachedInventoryComponent->AssignQuickSlot(CachedContextSlotIndex, SlotNumber);
    }
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
    bool bCanAssignQuickSlot = false;

    if (CachedSlots.IsValidIndex(CachedContextSlotIndex))
    {
        const FInventoryItem& Item = CachedSlots[CachedContextSlotIndex].Item;
        if (!Item.IsEmpty())
        {
            const bool bHasQuantity = Item.Quantity > 0;
            const bool bKeepWhenEmpty = Item.Definition.EmptyPolicy == EInventoryEmptyPolicy::KeepWhenEmpty;

            bCanUse = bHasQuantity;
            bCanAssignQuickSlot = CachedInventoryComponent.IsValid() && CachedInventoryComponent->CanAssignToQuickSlot(CachedContextSlotIndex);
            
            if (!bKeepWhenEmpty)
            {
                bCanRemove = bHasQuantity || Item.Definition.EmptyPolicy == EInventoryEmptyPolicy::RemoveWhenEmpty;
            }
        }
    }

    if (UseMenuEntry)
    {
        UseMenuEntry->SetIsEnabled(bCanUse);
    }

    if (QuickSlotMenuEntry)
    {
        QuickSlotMenuEntry->SetIsEnabled(bCanAssignQuickSlot);
    }

    if (RemoveMenuEntry)
    {
        RemoveMenuEntry->SetIsEnabled(bCanRemove);
    }

    if (QuickSlotSelectorBox && !bCanAssignQuickSlot)
    {
        QuickSlotSelectorBox->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventoryWidget::OnInventoryChanged()
{
    if (CachedInventoryComponent.IsValid())
    {
        RefreshInventory(CachedInventoryComponent->GetSlots(), CachedInventoryComponent->GetSlotCount());
    }

    HideRemoveQuantityDialog();
    HideContextMenu();
}

void UInventoryWidget::UseItemAtSlotIndex(int32 SlotIndex)
{
    CachedContextSlotIndex = SlotIndex;
    HandleUseItem();
}