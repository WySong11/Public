void UNxInventoryScene::NativeConstruct()
{
	Super::NativeConstruct();

	USER_BUTTON_BIND_CLICK( CloseButton, &UNxInventoryScene::OnCloseButton );
	InventoryMaxSlotCount = FNxGlobalFactor::InvenSlotMaxRowCount * FNxGlobalFactor::InvenSlotColumnCount;	

	if (const UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(InventoryWindow->Slot))
	{
		 InitInventoryWindowMargin = CanvasPanelSlot->GetOffsets();
	}
}

void UNxInventoryScene::OnCloseButton( UNxUserButton* Button )
{
	UI_HIDE( ENxUIType::Inventory );
}

void UNxInventoryScene::NativeDestruct()
{
	Super::NativeDestruct();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 보여주기 함수
/// </summary>

void UNxInventoryScene::ShowInventory( bool InVisible )
{
	if ( InVisible == true )
	{
		InitItemList();
	}

	SetVisible( InVisible );
}

void UNxInventoryScene::ShowDestroyPopup( const NxObjectId InItemId )
{
	SelectedItemId = InItemId;
	FString StrPopup = UxtLocalize::Instance()->GetLocal( TEXT( "ClientString_UI.Inventory.ItemDestroy.Popup" ) );

	using FStringFormatArgs = TMap<FString, FStringFormatArg>;
	FStringFormatArgs args;
	args.Emplace( TEXT( "itemname" ), UNxItemManager::Instance()->GetItemNameText( InItemId ) );
	FString ContentsString = FString::Format( *StrPopup, args );

	FxtPopupInfo info;
	info.PopupId = UxtPopupManager::Instance()->MakePopupId( ExtMainPopupType::System, (int32)ENxPopupType::ItemDestroy );
	info.Contents.Add( ContentsString );
	info.Callback.BindUFunction( this, GET_FUNCTION_NAME_CHECKED_OneParam( UNxInventoryScene, OnDestroyButton, int32 ) );
	info.UiType = ENxUIType::YesNoPopup;
	info.PopupPriority = ExtPopupPriority::TopPriority;
	UxtPopupManager::Instance()->OpenPopup( info );

	auto PopupUI = UI_GET( UNxYesNoPopup, ENxUIType::YesNoPopup );
	if ( PopupUI )
	{
		PopupUI->UseCloseSound = false;
	}
}

void UNxInventoryScene::OnDestroyButton( int32 Param )
{
	UxtPopupManager::Instance()->ClosePopup();

	if ( Param == 0 )
	{
		UNxItemManager::Instance()->DestroyItem( SelectedItemId );
	}
}

void UNxInventoryScene::ShowDividePopup( const NxObjectId InItemId )
{
	SelectedItemId = InItemId;
	FString StrPopup = UxtLocalize::Instance()->GetLocal( TEXT( "ClientString_UI.Inventory.ItemDivide.Popup" ) );

	using FStringFormatArgs = TMap<FString, FStringFormatArg>;
	FStringFormatArgs args;
	args.Emplace( TEXT( "itemname" ), UNxItemManager::Instance()->GetItemNameText( InItemId ) );
	FString ContentsString = FString::Format( *StrPopup, args );

	FxtPopupInfo info;
	info.PopupId = UxtPopupManager::Instance()->MakePopupId( ExtMainPopupType::System, (int32)ENxPopupType::ItemDivide );
	info.Contents.Add( ContentsString );
	info.Callback.BindUFunction( this, GET_FUNCTION_NAME_CHECKED_OneParam( UNxInventoryScene, OnDivideButton, int32 ) );
	info.UiType = ENxUIType::TextInputPopup;
	info.PopupPriority = ExtPopupPriority::TopPriority;
	UxtPopupManager::Instance()->OpenPopup( info );

	auto PopupUI = UI_GET( UNxTextInputPopup, ENxUIType::TextInputPopup );
	if ( PopupUI )
	{
		PopupUI->UseCloseSound = false;
	}
}

void UNxInventoryScene::OnDivideButton( int32 Param )
{
	UxtPopupManager::Instance()->ClosePopup();

	auto PopupUI = UI_GET( UNxTextInputPopup, ENxUIType::TextInputPopup );

	FText itemcount = PopupUI->GetEditableText();

	if ( Param == 0 )
	{
		NX_LOG( TEXT( "Divide" ) );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 아이템 리스트 함수
/// </summary>

void UNxInventoryScene::InitItemList()
{
	ItemList->ClearSelection();
	ItemList->ClearListItems();

	const TArray<FNxItemInfo> itemlist = UNxItemManager::Instance()->GetItemList();
	UNxInventoryTileViewItemData* data;

	for ( int i = 0; i < InventoryMaxSlotCount; i++ )
	{
		data = NewObject<UNxInventoryTileViewItemData>();

		// InvenIndex 랑 맞는 아이템 찾아야 함
		const FNxItemInfo* tempItem = itemlist.FindByPredicate( [ InInvenIndex = i ]( FNxItemInfo iteminfodata )
			{
				return iteminfodata.InvenIndex == InInvenIndex;
			} );

		if ( tempItem != NULL )
		{
			data->SetItemData( i, tempItem->Id );
		}
		else
		{
			data->SetItemData( i, 0 );
		}

		ItemList->AddItem( data );
	}
}

void UNxInventoryScene::UpdateItem()
{
	//for ( int i = 0; i < ItemList->GetNumItems(); i++ )
	//{
	//	TObjectPtr<UNxInventoryTileViewItemData> tempItem = Cast<UNxInventoryTileViewItemData>( ItemList->GetItemAt( i ) );

	//	tempItem->SetItemData( tempItem->GetInvenIndex(), tempItem->GetItemId() );

	//	
	//}

	//ItemList->RequestRefresh();
}

void UNxInventoryScene::SetTestData( int InCount )
{
	TestData = InCount > 0;
	TestCount = InCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 선택된 아이템 데이터 함수
/// </summary>

TObjectPtr< class UNxInventoryTileViewItemData > UNxInventoryScene::GetSelectedItemData() const
{
	return ItemList->GetSelectedItemData();
}

int16 UNxInventoryScene::GetSelectedInvenIndex() const
{
	return ItemList->GetSelectedInvenIndex();
}

NxObjectId UNxInventoryScene::GetSelectedItemId() const
{
	return ItemList->GetSelectedItemId();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 슬롯 함수
/// </summary>

void UNxInventoryScene::SetInvenSlotCount( int InSlotCount )
{
	InventorySlotCount = InSlotCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// callback 받는 함수
/// </summary>

void UNxInventoryScene::OnMouseDown( const class UNxInventoryTileViewItemData* InItemData, const ENxUIState InType, const FVector2D InPos )
{
	//NX_LOG( TEXT( "ItemID -> %lld , Pos -> %f %f" ), InItemID, InPos.X , InPos.Y );
	//SCREEN_LOG( "ItemID -> %lld , Pos -> %f %f", FColor::Red, InItemID, InPos.X , InPos.Y );

	switch ( InType )
	{
	case ENxUIState::Used:
	{

	}
	break;

	case ENxUIState::Selected:
	{
		// ItemList 에 설정된 데이터와 이벤트로 날아온 데이터가 같다면, Drag 표시
		if ( GetSelectedItemData() == InItemData )
		{
			if ( ItemIcon != nullptr )
			{
				UI_HIDE( ENxUIType::ItemWidget );
				ItemIcon = nullptr;
			}

			FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Selected );
			ShowDragItemIcon( InItemData, InPos );
		}
		else
		{

		}
	}
	break;
	}
}

void UNxInventoryScene::OnMouseMove( const class UNxInventoryTileViewItemData* InItemData, const ENxUIState InType, const FVector2D InPos )
{
	//NX_LOG( TEXT( "ItemID -> %lld , Pos -> %f %f" ), InItemID, InPos.X , InPos.Y );
	//SCREEN_LOG( "ItemID -> %lld , Pos -> %f %f", FColor::Red, InItemID, InPos.X , InPos.Y );

	switch ( InType )
	{
	case ENxUIState::Selected:
	case ENxUIState::Locked:
	{
		MoveDragItemIcon( InItemData, GetDragItemIconPosition( InPos ) );
	}
	break;
	}
}

void UNxInventoryScene::OnMouseUp( const class UNxInventoryTileViewItemData* InItemData, const ENxUIState InType, const FVector2D InPos )
{
	NX_LOG( TEXT( "InvenScene InvenIndex -> %d , Pos -> %f %f" ), InItemData->GetInvenIndex(), InPos.X, InPos.Y );
	NX_LOG( TEXT( "InvenScene Type -> %s" ), *FNxConvert::EnumToString( InType ) );
	SCREEN_LOG( "InvenScene InvenIndex -> %d , Pos -> %f %f", FColor::Red, InItemData->GetInvenIndex(), InPos.X, InPos.Y );
	SCREEN_LOG( "InvenScene  Type -> %s", FColor::Red, *FNxConvert::EnumToString( InType ) );

	switch ( InType )
	{
	case ENxUIState::Used:
	{
		if ( ItemList->GetSelectedItemData() == InItemData )
		{
			UNxItemManager::Instance()->UseItem( InItemData->GetItemId() );
		}
		else
		{
			FxtEvent_ItemState::Dispatch( ItemList->GetSelectedItemData(), ENxUIState::Enable );
			FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Enable );
		}
		ItemList->ClearSelectedItemData();
	}
	break;

	case ENxUIState::Selected:
	{

	}
	break;

	case ENxUIState::Locked:
	{
		// 선택된 아이템의 Locked Event 왔으면, Locked 오로 상태 설정해준다.
		if ( ItemList->GetSelectedItemData() == InItemData )
		{
			FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Locked );
		}
	}
	break;

	case ENxUIState::Moved:
	{
		UNxItemManager::Instance()->MoveItem( ItemList->GetSelectedItemId(), InItemData->GetInvenIndex() );
		FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Enable );
		FxtEvent_ItemState::Dispatch( ItemList->GetSelectedItemData(), ENxUIState::Enable );
		HideDragItemIcon();
	}
	break;

	case ENxUIState::Exchanged:
	{
		UNxItemManager::Instance()->MoveItem( ItemList->GetSelectedItemId(), InItemData->GetInvenIndex() );
		FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Enable );
		FxtEvent_ItemState::Dispatch( ItemList->GetSelectedItemData(), ENxUIState::Enable );
		HideDragItemIcon();
	}
	break;

	case ENxUIState::Released:
	{
		FxtEvent_ItemState::Dispatch( InItemData, ENxUIState::Enable );
		HideDragItemIcon();
	}
	break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ItemIcon Drag 함수
/// </summary>

void UNxInventoryScene::ShowDragItemIcon( const class UNxInventoryTileViewItemData* InItemData, const FVector2D InPos )
{
	ItemIcon = UI_SHOW( UNxInventoryDragWindow, ENxUIType::ItemWidget );

	Localplayer = GetWorld()->GetFirstLocalPlayerFromController();
	// 현재 뷰포트 스케일 구함
	ViewportScale = UWidgetLayoutLibrary::GetViewportScale( CastChecked < ULocalPlayer>( Localplayer )->ViewportClient );
	ScaleSize = FVector2D( 1 * ViewportScale, 1 * ViewportScale );

	WindowGeometry = InventoryWindow->GetCachedGeometry();
	TileGeometry = ItemListObj->GetCachedGeometry();

	ItemIcon->SetItemInfo( InItemData->GetItemId(), InItemData->GetInvenIndex() );
	ItemIcon->SetDesiredSizeInViewport( ScaleSize );
	ItemIcon->SetPositionInViewport( GetDragItemIconPosition( InPos ), false );
	ItemIcon->SetVisibility( ESlateVisibility::HitTestInvisible );
}

void UNxInventoryScene::MoveDragItemIcon( const class UNxInventoryTileViewItemData* InItemData, const FVector2D InPos )
{
	if ( ItemIcon != nullptr )
	{
		ItemIcon->SetPositionInViewport( InPos, false );
	}
}

void UNxInventoryScene::HideDragItemIcon()
{
	if ( ItemIcon != nullptr )
	{
		ItemIcon->SetVisibility( ESlateVisibility::Hidden );
		UI_HIDE( ENxUIType::ItemWidget );
		ItemIcon = nullptr;
	}
	ItemList->ClearSelectedItemData();
}

FVector2D UNxInventoryScene::GetDragItemIconPosition( const FVector2D InPos )
{
	FVector2D Position = InPos;
	FVector2D tilePos = TileGeometry.GetLocalPositionAtCoordinates( FVector2D( 0.0f, 0.0f ) );
	FVector2D windowPos = WindowGeometry.GetLocalPositionAtCoordinates( FVector2D( 0.0f, 0.0f ) );

	Position += ( tilePos + windowPos );
	return Position;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Inventory Window Drag 함수
/// </summary>

void UNxInventoryScene::MoveInventoryWindow( const FVector2D InPos )
{
	FVector2D movePos = MouseDownPos - InPos;

	FMargin NewMargin = InitInventoryWindowMargin;
	NewMargin.Top    = InitInventoryWindowMargin.Top - movePos.Y;
	NewMargin.Left   = InitInventoryWindowMargin.Left - movePos.X;
	
	NxUtilWidget::SetOffsets( InventoryWindow->Slot, NewMargin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 입력 관련 함수
/// </summary>

FReply UNxInventoryScene::NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	//NX_LOG( TEXT( "Pos -> %f %f" ), InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );
	//SCREEN_LOG( "Pos -> %f %f", FColor::Red, InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );

	if ( ItemIcon != nullptr )
	{
		// 우클릭
		if ( InMouseEvent.IsMouseButtonDown( EKeys::RightMouseButton ) == true )
		{
			FxtEvent_ItemState::Dispatch( ItemList->GetSelectedItemData(), ENxUIState::Enable );
			HideDragItemIcon();
		}
		// 좌클릭
		else if ( InMouseEvent.IsMouseButtonDown( EKeys::LeftMouseButton ) == true )
		{
			ShowDestroyPopup( ItemList->GetSelectedItemId() );
			//ShowDividePopup( ItemList->GetSelectedItemId() );
			FxtEvent_ItemState::Dispatch( ItemList->GetSelectedItemData(), ENxUIState::Enable );
			HideDragItemIcon();
		}
	}
	else
	{
		if ( DragArena->GetCachedGeometry().IsUnderLocation( InMouseEvent.GetScreenSpacePosition() ) == true )
		{
			bMovingInventoryWindow = true;
			MouseDownPos = InGeometry.AbsoluteToLocal( InMouseEvent.GetScreenSpacePosition() );
		}
	}

	TSharedPtr<SWidget> CapturingSlateWidget = this->GetCachedWidget();
	if ( CapturingSlateWidget.IsValid() )
	{
		return FReply::Handled().CaptureMouse( CapturingSlateWidget.ToSharedRef() );
	}
	else
	{
		return FReply::Handled();
	}
}

FReply UNxInventoryScene::NativeOnMouseMove( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	if ( ItemIcon != nullptr )
	{
		FVector2D mousePos = InGeometry.AbsoluteToLocal( InMouseEvent.GetScreenSpacePosition() );
		MoveDragItemIcon( ItemList->GetSelectedItemData(), mousePos );
	}
	else if( bMovingInventoryWindow == true )
	{
		FVector2D mousePos = InGeometry.AbsoluteToLocal( InMouseEvent.GetScreenSpacePosition() );
		MoveInventoryWindow( mousePos );
	}

	return FReply::Handled();
}

FReply UNxInventoryScene::NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	if ( ItemIcon != nullptr )
	{

	}
	else if( bMovingInventoryWindow == true )
	{
		bMovingInventoryWindow = false;
	}

	return FReply::Handled().ReleaseMouseCapture();
}

//void UNxInventoryScene::NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation )
//{
//	NX_LOG( TEXT( "Pos -> %f %f" ), InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %f %f", FColor::Red, InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );
//}
//
//void UNxInventoryScene::NativeOnDragCancelled( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation )
//{
//	NX_LOG( TEXT( "Pos -> %d %d" ), InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %d %d", FColor::Red, InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//}
//
//void UNxInventoryScene::NativeOnDragEnter( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation )
//{
//	NX_LOG( TEXT( "Pos -> %f %f" ), InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %f %f", FColor::Red, InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//}
//
//void UNxInventoryScene::NativeOnDragLeave( const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation )
//{
//	NX_LOG( TEXT( "Pos -> %f %f" ), InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %f %f", FColor::Red, InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//}
//
//bool UNxInventoryScene::NativeOnDragOver( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation )
//{
//	NX_LOG( TEXT( "Pos -> %f %f" ), InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %f %f", FColor::Red, InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//
//	return false;
//}
//
//bool UNxInventoryScene::NativeOnDrop( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation )
//{
//	NX_LOG( TEXT( "Pos -> %f %f" ), InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//	SCREEN_LOG( "Pos -> %f %f", FColor::Red, InDragDropEvent.GetScreenSpacePosition().X, InDragDropEvent.GetScreenSpacePosition().Y );
//
//	return false;
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 이벤트 함수
/// </summary>

void UNxInventoryScene::InitEvents()
{
	Super::InitEvents();
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::UIRefresh,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryScene::OnRefresh ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemMouseDown,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryScene::OnItemMouseDown ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemMouseMove,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryScene::OnItemMouseMove ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemMouseUp,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryScene::OnItemMouseUp ) );
		EventConnections.Add( connection );
	}
}

void UNxInventoryScene::OnRefresh( const class FxtEvent* InEvent )
{
	const FxtEvent_UIRefresh* eventData = static_cast<const FxtEvent_UIRefresh*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetValue() == ENxUIType::Inventory )
	{
		NX_LOG( TEXT( "UI Type -> %s" ), *FNxConvert::EnumToString( eventData->GetValue() ) );
		UpdateItem();
	}
}

void UNxInventoryScene::OnItemMouseDown( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemMouseDown* eventData = static_cast<const FxtEvent_ItemMouseDown*>( InEvent );
	if ( eventData == nullptr )
		return;

	OnMouseDown( eventData->GetItemData(), eventData->GetState(), eventData->GetMousePos() );
}

void UNxInventoryScene::OnItemMouseMove( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemMouseMove* eventData = static_cast<const FxtEvent_ItemMouseMove*>( InEvent );
	if ( eventData == nullptr )
		return;

	OnMouseMove( eventData->GetItemData(), eventData->GetState(), eventData->GetMousePos() );
}

void UNxInventoryScene::OnItemMouseUp( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemMouseUp* eventData = static_cast<const FxtEvent_ItemMouseUp*>( InEvent );
	if ( eventData == nullptr )
		return;

	OnMouseUp( eventData->GetItemData(), eventData->GetState(), eventData->GetMousePos() );
}