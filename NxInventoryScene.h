// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/NxCommonTypes.h"
#include "UI/NxUserScene.h"
#include "NxInventoryScene.generated.h"


/**
 *
 */
UCLASS( BlueprintType, Blueprintable )
class NEXTER_API UNxInventoryScene : public UNxUserScene
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr< class UCanvasPanel > InventoryWindow;

	UPROPERTY( meta = ( BindWidget, AllowPrivateAccess = true ), Transient )
		TObjectPtr< class UNxUserButton > CloseButton;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr< class UCanvasPanel > ItemListObj;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr< class UNxInventoryTileView > ItemList;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr< class UNxInventoryDragWindow > ItemIcon;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr< class UCanvasPanel > DragArena;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "NxInventory|Test" )
		bool TestData = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "NxInventory|Test", meta = ( EditCondition = "TestData", EditConditionHides ) )
		int TestCount = 1;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 보여주기 함수
	void ShowInventory( bool InVisible );
	void ShowDestroyPopup( const NxObjectId InItemId );
	UFUNCTION()
		void OnDestroyButton( int32 Param );
	void ShowDividePopup( const NxObjectId InItemId );
	UFUNCTION()
		void OnDivideButton( int32 Param );

	// 아이템 리스트 함수
	void InitItemList();
	void UpdateItem();
	void SetTestData( int InCount );

	// 선택된 아이템 데이터 함수
	TObjectPtr< class UNxInventoryTileViewItemData > GetSelectedItemData() const;
	int16 GetSelectedInvenIndex() const;
	NxObjectId GetSelectedItemId() const;

	// 슬롯 함수
	int GetInvenSlotCount() { return InventorySlotCount; }
	void SetInvenSlotCount( int InCount );

	// callback 받는 함수
	void OnMouseDown( const class UNxInventoryTileViewItemData* InItemData, ENxUIState InType, const FVector2D InPos );
	void OnMouseUp( const class UNxInventoryTileViewItemData* InItemData, ENxUIState InType, const FVector2D InPos );
	void OnMouseMove( const class UNxInventoryTileViewItemData* InItemData, ENxUIState InType, const FVector2D InPos );

	// ItemIcon Drag 함수
	void ShowDragItemIcon( const class UNxInventoryTileViewItemData* InItemData, const FVector2D InPos );
	void MoveDragItemIcon( const class UNxInventoryTileViewItemData* InItemData, const FVector2D InPos );
	void HideDragItemIcon();
	FVector2D GetDragItemIconPosition( const FVector2D InPos );

	// Inventory Window Drag 함수
	void MoveInventoryWindow( const FVector2D InPos );

protected:

	// 이벤트 함수
	virtual void InitEvents() override;
	void OnRefresh( const class FxtEvent* InEvent );
	void OnItemMouseDown( const class FxtEvent* InEvent );
	void OnItemMouseMove( const class FxtEvent* InEvent );
	void OnItemMouseUp( const class FxtEvent* InEvent );

	// 입력 관련 함수
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseMove( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

	//virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	//virtual void NativeOnDragEnter( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual void NativeOnDragLeave( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual bool NativeOnDragOver( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual bool NativeOnDrop( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual void NativeOnDragCancelled( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;

protected:

	ULocalPlayer* Localplayer;
	float ViewportScale;
	FVector2D ScaleSize;
	FGeometry WindowGeometry;
	FGeometry TileGeometry;

	bool bMovingInventoryWindow;
	FVector2D MouseDownPos;
	FMargin InitInventoryWindowMargin;

	int InventorySlotCount;
	int InventoryMaxSlotCount;

	NxObjectId SelectedItemId ;

private:
	UFUNCTION()
		void OnCloseButton( UNxUserButton* Button );
};