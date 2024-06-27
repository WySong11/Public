// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/NxCommonTypes.h"
#include "UI/Control/NxTileViewItem.h"
#include "NxInventoryTileViewItem.generated.h"


/**
 *
 */
UCLASS( BlueprintType, Blueprintable )
class NEXTER_API UNxInventoryTileViewItem : public UNxTileViewItem
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr<class UNxInventoryItemIcon> ItemIcon;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr<class UNxImage> DimBg;

	UPROPERTY( BlueprintReadWrite, meta = ( BindWidgetOptional ) )
		TObjectPtr<class UNxTextBlock> CoolTimeText;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& InGeometry, float InDeltaTime ) override;

	// 이벤트 함수
	virtual void InitEvents() override;
	void OnItemCoolTimeStart( const class FxtEvent* InEvent );
	void OnItemAdd( const class FxtEvent* InEvent );
	void OnItemRemove( const class FxtEvent* InEvent );
	void OnItemState( const class FxtEvent* InEvent );
	void OnItemRefresh( const class FxtEvent* InEvent );

	// IUserObjectListEntry
	virtual void NativeOnListItemObjectSet( UObject* ListItemObject ) override;

	// IUserListEntry
	virtual void NativeOnItemSelectionChanged( bool bIsSelected ) override;
	virtual void NativeOnItemExpansionChanged( bool bIsExpanded ) override;
	virtual void NativeOnEntryReleased() override;

	// TileView 받는 함수
	TObjectPtr< class UNxInventoryTileView> GetTileView();
	TObjectPtr< class UNxInventoryTileViewItemData > GetTileViewSelectedItemData();

	// callback 함수
	//DECLARE_CALLBACK( CBOnMouseDown, int16, ENxUIState, FVector2D );
	//DECLARE_CALLBACK( CBOnMouseUp, int16, ENxUIState, FVector2D );
	//DECLARE_CALLBACK( CBOnMouseMove, int16, ENxUIState, FVector2D );

	// 마우스 위치 구하는 함수
	FVector2D GetMousePosition( const FGeometry& InGeometry, const FVector2D InMousePosition );

	// 아이템 데이터 입력 함수
	void UpdateItemData( const class UNxInventoryTileViewItemData* InData );
	void UpdateItemData( const int16 InInvenIndex, const NxObjectId InItemId );

	// 아이템 데이터 얻는 함수
	TObjectPtr<UNxInventoryTileViewItemData> GetItemData() const;
	int16 GetInvenIndex() const;
	NxObjectId GetItemId() const;

	// 정리 함수
	void Clear();

	// 아이템 표시 함수
	void DisplayItemInfo();
	void DisplayItemDimBg();
	void DisplayItemCoolTime( const float InRemainTime = 0.0f );
	void DisplayEmptyItem();

	// 아이템 쿨타임 함수
	void StartCoolTime( const float InRemainSec );
	void UpdateCoolTime();
	void EndCoolTime();
	void ClearTimerHandle();

protected:
	// 입력 관련 함수
	//virtual FReply NativeOnPreviewMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseMove( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual void NativeOnMouseEnter( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual void NativeOnMouseLeave( const FPointerEvent& InMouseEvent ) override;

	//virtual FReply NativeOnMouseWheel( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	//virtual FReply NativeOnMouseButtonDoubleClick( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

	//virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	//virtual void NativeOnDragCancelled( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual void NativeOnDragEnter( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual void NativeOnDragLeave( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual bool NativeOnDragOver( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;
	//virtual bool NativeOnDrop( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;

	//virtual void OnTileMouseHover( bool InHovered ) override;

protected:
	bool bIsCanUsed;
	bool bIsCoolTimeActive;

private:
	TObjectPtr<UNxInventoryTileViewItemData> ItemData;

	UPROPERTY()
		FTimerHandle CoolTimeUpdateHandle;

	UPROPERTY()
		FTimerHandle CoolTimeDisplayHandle;
};