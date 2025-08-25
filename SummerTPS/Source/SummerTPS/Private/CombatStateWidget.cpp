#include "CombatStateWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UCombatStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass());
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;

	StateTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Root->AddChild(StateTextBlock);

if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(StateTextBlock->Slot))
{
	// Top-right corner with some padding
	CanvasSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
	CanvasSlot->SetAlignment(FVector2D(1.f, 0.f));
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetPosition(FVector2D(-20.f, 20.f));
}

	StateTextBlock->SetText(FText::FromString(TEXT("State")));
	StateTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.1f, 1.f)));
}

void UCombatStateWidget::UpdateStateText(FText InText)
{
	if (StateTextBlock)
	{
		StateTextBlock->SetText(InText);
	}
}
