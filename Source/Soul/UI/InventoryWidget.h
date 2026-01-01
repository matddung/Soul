#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Input/Reply.h"
#include "../Character/InventoryItem.h"
#include "InventoryWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UUniformGridPanel;
class UVerticalBox;
class USoulInventoryComponent;
class UInventoryWidget;

UENUM()
enum class EInventoryContextAction : uint8
{
    Use,
    QuickSlotToggle,
    Remove,
    AssignQuickSlot
};

UCLASS()
class SOUL_API UInventorySlotBorder : public UBorder
{
    GENERATED_BODY()

public:
    void InitializeSlot(UInventoryWidget* InOwner, int32 InSlotIndex);

private:
    UFUNCTION()
    FEventReply HandleMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

    UPROPERTY()
    UInventoryWidget* Owner = nullptr;

    int32 SlotIndex = INDEX_NONE;
};

UCLASS()
class SOUL_API UInventoryMenuEntryBorder : public UBorder
{
    GENERATED_BODY()

public:
    void InitializeEntry(UInventoryWidget* InOwner, EInventoryContextAction InAction, int32 InQuickSlotNumber = 0);

private:
    UFUNCTION()
    FEventReply HandleMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

    UPROPERTY()
    UInventoryWidget* Owner = nullptr;

    EInventoryContextAction Action = EInventoryContextAction::Use;

    int32 QuickSlotNumber = 0;
};

UCLASS()
class SOUL_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetInventoryComponent(USoulInventoryComponent* InventoryComponent);
    void RefreshInventory(const TArray<FInventorySlot>& Slots, int32 TotalSlotCount);
    FEventReply HandleSlotMouseButtonDown(int32 SlotIndex, const FGeometry& Geometry, const FPointerEvent& MouseEvent);
    FEventReply HandleMenuEntryAction(EInventoryContextAction Action, int32 QuickSlotNumber, const FPointerEvent& MouseEvent);

    virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
    virtual void NativeOnInitialized() override;

    void BuildGrid();
    void ResetGrid();
    void AddSlotWidget(int32 SlotIndex, int32 Row, int32 Column);
    FText BuildSlotLabel(int32 SlotIndex) const;
    int32 GetColumnCount(int32 SlotCount) const;

    void EnsureRootPanel();
    void EnsureContextMenu();
    void ShowContextMenu(int32 SlotIndex, const FVector2D& ScreenSpacePosition);
    void HideContextMenu();
    void RefreshQuickSlotSelectors();
    FEventReply HandleMenuBlockerMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
    void HandleUseItem();
    void HandleRemoveItem();
    void ToggleQuickSlotSelector();
    void AssignQuickSlot(int32 SlotNumber);
    void ResetMenuSelection();
    void UpdateMenuEntryStates();
    UBorder* CreateMenuEntry(const FText& Label, EInventoryContextAction Action, int32 QuickSlotNumber = 0) const;

    void OnInventoryChanged();

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    UCanvasPanel* RootPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    UUniformGridPanel* GridPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* ContextMenuBlocker;

    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* ContextMenuBorder;

    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* ContextMenuBox;

    UPROPERTY(meta = (BindWidgetOptional))
    UVerticalBox* QuickSlotSelectorBox;

    UPROPERTY()
    UInventoryMenuEntryBorder* UseMenuEntry = nullptr;

    UPROPERTY()
    UInventoryMenuEntryBorder* RemoveMenuEntry = nullptr;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 DefaultColumns = 4;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 DefaultSlotCount = 16;

    UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 1))
    int32 QuickSlotCount = 4;

    TWeakObjectPtr<USoulInventoryComponent> CachedInventoryComponent;
    TArray<FInventorySlot> CachedSlots;
    int32 CachedSlotCount = 0;
    int32 CachedContextSlotIndex = INDEX_NONE;
};