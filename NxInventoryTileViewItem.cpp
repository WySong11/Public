void UNxInventoryTileViewItem::NativeConstruct()
{
	Super::NativeConstruct();
	Clear();
}

void UNxInventoryTileViewItem::NativeDestruct()
{
	Clear();
	Super::NativeDestruct();
}

void UNxInventoryTileViewItem::NativeTick( const FGeometry& InGeometry, float InDeltaTime )
{
	Super::NativeTick( InGeometry, InDeltaTime );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 이벤트 함수
/// </summary>

void UNxInventoryTileViewItem::InitEvents()
{
	Super::InitEvents();
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemCoolTimeStart,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryTileViewItem::OnItemCoolTimeStart ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemAdd,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryTileViewItem::OnItemAdd ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemRemove,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryTileViewItem::OnItemRemove ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemState,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryTileViewItem::OnItemState ) );
		EventConnections.Add( connection );
	}
	{
		FxtEventConnectionPtr connection = FxtEventDispatcher::Instance()->Register(
			ExtEventTypes::ItemRefresh,
			FxtEventHandlerDelegate::CreateUObject( this, &UNxInventoryTileViewItem::OnItemRefresh ) );
		EventConnections.Add( connection );
	}
}

void UNxInventoryTileViewItem::OnItemCoolTimeStart( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemCoolTimeStart* eventData = static_cast<const FxtEvent_ItemCoolTimeStart*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetEventType() != ExtEventTypes::ItemCoolTimeStart )
		return;

	if ( eventData->GetItemId() != GetItemId() )
		return;

	if ( eventData->GetRemainTime() > 0.0f )
	{
		StartCoolTime( MS_TO_SECONDS( eventData->GetRemainTime() ) );
	}
}

void UNxInventoryTileViewItem::OnItemAdd( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemAdd* eventData = static_cast<const FxtEvent_ItemAdd*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetInvenIndex() != GetInvenIndex() )
		return;

	UpdateItemData( eventData->GetInvenIndex(), eventData->GetItemId() );
}

void UNxInventoryTileViewItem::OnItemRemove( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemRemove* eventData = static_cast<const FxtEvent_ItemRemove*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetInvenIndex() != GetInvenIndex() )
		return;

	UpdateItemData( eventData->GetInvenIndex(), eventData->GetItemId() );
}

void UNxInventoryTileViewItem::OnItemState( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemState* eventData = static_cast<const FxtEvent_ItemState*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetItemData()->GetInvenIndex() != GetInvenIndex() )
		return;

	if ( eventData->GetItemData()->GetItemId() != GetItemId() )
	{
		UpdateItemData( eventData->GetItemData() );
	}
	SetState( eventData->GetState(), ItemIcon->GetItemIcon() );
}

void UNxInventoryTileViewItem::OnItemRefresh( const class FxtEvent* InEvent )
{
	const FxtEvent_ItemRefresh* eventData = static_cast<const FxtEvent_ItemRefresh*>( InEvent );
	if ( eventData == nullptr )
		return;

	if ( eventData->GetInvenIndex() == GetInvenIndex() )
	{
		TObjectPtr<FNxItemInfo> tempItem = UNxItemManager::Instance()->GetItemByInvenIndex( GetInvenIndex() );
		if ( tempItem == nullptr )
		{
			UpdateItemData( GetInvenIndex(), 0 );
		}
		else
		{
			UpdateItemData( tempItem->InvenIndex , tempItem->Id );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// IUserObjectListEntry
/// </summary>

void UNxInventoryTileViewItem::NativeOnListItemObjectSet( UObject* ListItemObject )
{
	Super::NativeOnListItemObjectSet( ListItemObject );

	UNxInventoryTileViewItemData* itemdata = Cast<UNxInventoryTileViewItemData>( ListItemObject );

	if ( IsValid( itemdata ) == false )
	{
		return;
	}

	SetState( ENxUIState::Enable, ItemIcon->GetItemIcon() );
	UpdateItemData( itemdata );

	TObjectPtr< UNxInventoryTileView > tileView = GetTileView();
	if ( tileView != nullptr )
	{
		TObjectPtr< UNxInventoryTileViewItemData > selectedItemData = tileView->GetSelectedItemData();
		if ( selectedItemData->IsEmpty() == false )
		{
			// 기존에 선택된 아이템은 선택 처리
			if ( selectedItemData == itemdata )
			{
				tileView->SetSelectedItemData( GetItemData() );
				SetState( ENxUIState::Selected, ItemIcon->GetItemIcon() );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// IUserListEntry
/// </summary>

void UNxInventoryTileViewItem::NativeOnItemSelectionChanged( bool bIsSelected )
{
	Super::NativeOnItemSelectionChanged( bIsSelected );
}

void UNxInventoryTileViewItem::NativeOnItemExpansionChanged( bool bIsExpanded )
{
	Super::NativeOnItemExpansionChanged( bIsExpanded );
}

void UNxInventoryTileViewItem::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// TileView 함수
/// </summary>

TObjectPtr<UNxInventoryTileView> UNxInventoryTileViewItem::GetTileView()
{
	return Cast< UNxInventoryTileView >( GetOwningListView() );
}

TObjectPtr< UNxInventoryTileViewItemData > UNxInventoryTileViewItem::GetTileViewSelectedItemData()
{
	TObjectPtr< UNxInventoryTileView > tileView = GetTileView();

	return tileView->GetSelectedItemData();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 입력 관련 함수
/// </summary>

//void UNxInventoryTileViewItem::OnTileMouseHover( bool InHovered )
//{
//	ChangeBrushColor( InHovered == true ? ENxUIState::Hovered : ENxUIState::Enable , ItemIcon->GetItemIcon() );
//}

FReply UNxInventoryTileViewItem::NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	// 우클릭
	if ( InMouseEvent.IsMouseButtonDown( EKeys::RightMouseButton ) == true )
	{
		TObjectPtr< UNxInventoryTileViewItemData > selectedItemData = GetTileViewSelectedItemData();
		if ( selectedItemData->IsEmpty() == true )
		{
			GetTileView()->SetSelectedItemData( GetItemData() );

			// 사용 상태 설정하고, 이벤트를 날린다.
			SetState( ENxUIState::Used, ItemIcon->GetItemIcon() );
			FxtEvent_ItemMouseDown::Dispatch( GetItemData(), ENxUIState::Used, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
		}
		// 선택된 아이템이 있는 상태에서, 다른 슬롯에서 우클릭하면 해제한다.
		else if ( selectedItemData != GetItemData() )
		{
			FxtEvent_ItemMouseUp::Dispatch( selectedItemData, ENxUIState::Released, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
			SetState( ENxUIState::Enable );
		}
	}
	// 좌클릭
	else if ( InMouseEvent.IsMouseButtonDown( EKeys::LeftMouseButton ) == true )
	{
		if ( TileState == ENxUIState::Enable || TileState == ENxUIState::Hovered )
		{
			LocalPosition = InGeometry.GetLocalPositionAtCoordinates( FVector2D( 0.0f, 0.0f ) );

			TObjectPtr< UNxInventoryTileViewItemData > selectedItemData = GetTileViewSelectedItemData();
			if ( selectedItemData != GetItemData() && selectedItemData->IsEmpty() == true )
			{
				GetTileView()->SetSelectedItemData( GetItemData() );
				SetState( ENxUIState::Selected, ItemIcon->GetItemIcon() );
				FxtEvent_ItemMouseDown::Dispatch( GetItemData(), ENxUIState::Selected, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
			}
		}
		else if ( TileState == ENxUIState::Selected )
		{
			NX_LOG( TEXT( "WHY?????" ) );
		}
	}

	return Super::NativeOnMouseButtonDown( InGeometry, InMouseEvent );
}

FReply UNxInventoryTileViewItem::NativeOnMouseMove( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	// 마우스 좌클릭 또는 좌버튼 누른 상태에서는 입력 보낸다.
	if ( TileState == ENxUIState::Selected || TileState == ENxUIState::Locked )
	{
		FxtEvent_ItemMouseMove::Dispatch( GetItemData(), TileState, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
	}

	return Super::NativeOnMouseMove( InGeometry, InMouseEvent );
}

FReply UNxInventoryTileViewItem::NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	NX_LOG( TEXT( "TileViewItem Slot ->%d , ItemID -> %lld , Pos -> %f %f" ), GetInvenIndex(), GetItemId(), InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );
	NX_LOG( TEXT( "TileViewItem Type -> %s" ), *FNxConvert::EnumToString( TileState ) );
	SCREEN_LOG( "TileViewItem Slot -> %d , ItemID -> %lld , Pos -> %f %f", FColor::Red, GetInvenIndex(), GetItemId(), InMouseEvent.GetScreenSpacePosition().X, InMouseEvent.GetScreenSpacePosition().Y );
	SCREEN_LOG( "TileViewItem Type -> %s", FColor::Red, *FNxConvert::EnumToString( TileState ) );

	switch ( TileState )
	{
	case ENxUIState::None:
		break;
	case ENxUIState::Enable:
		break;
	case ENxUIState::Disable:
		break;
	case ENxUIState::Hovered:
	{
		// 현재 마우스 버튼이 떼진 곳의 아이템 정보를 보낸다.
		TObjectPtr< UNxInventoryTileViewItemData > selectedItemData = GetTileViewSelectedItemData();
		if ( selectedItemData->IsEmpty() == false )
		{
			ENxUIState resultType = ( GetItemId() != 0 ) ? ENxUIState::Exchanged : ENxUIState::Moved;
			FxtEvent_ItemMouseUp::Dispatch( GetItemData(), resultType, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
		}
	}
	break;

	case ENxUIState::Pressed:
		break;
	case ENxUIState::Released:
		break;

		// 좌클릭
	case ENxUIState::Selected:
	{
		TObjectPtr< UNxInventoryTileViewItemData > selectedItemData = GetTileViewSelectedItemData();
		
		// 선택된 아이템이랑 다르다면, 위치를 이동 또는 교환 한다.
		if ( selectedItemData != GetItemData() )
		{
			ENxUIState resultType = ( GetItemId() != 0 ) ? ENxUIState::Exchanged : ENxUIState::Moved;
			FxtEvent_ItemMouseUp::Dispatch( GetItemData(), resultType, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
		}
		// 선택된 아이템이랑 같다면, Locked 상태로 변경
		else
		{
			FxtEvent_ItemMouseUp::Dispatch( GetItemData(), ENxUIState::Locked, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
		}
	}
	break;

	case ENxUIState::Locked:
	{
		// 이미 선택된 아이템이라서, 아무 행동 없이 Lock 을 해제한다.
		FxtEvent_ItemMouseUp::Dispatch( GetItemData(), ENxUIState::Released, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
	}
	break;

	// 우클릭
	case ENxUIState::Used:
	{
		if ( bIsCanUsed == false )
		{
			UNxUiManager::Instance()->ShowToast( FString::Printf( TEXT( "Not Used Item ID -> %d" ), GetItemId() ) );
		}
		else if ( bIsCoolTimeActive == true )
		{
			UNxUiManager::Instance()->ShowToast( FString::Printf( TEXT( "Active Item CoolTime ID -> %d" ), GetItemId() ) );
		}
		else
		{
			// 아이템 사용 가능함을 이벤트로 날린다.
			FxtEvent_ItemMouseUp::Dispatch( GetItemData(), TileState, GetMousePosition( InGeometry, InMouseEvent.GetScreenSpacePosition() ) );
			SetState( ENxUIState::Enable );		
		}
	}
	break;

	case ENxUIState::Max:
		break;

	default:
		break;
	}

	return FReply::Handled().ReleaseMouseCapture();
}

void UNxInventoryTileViewItem::NativeOnMouseEnter( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	if ( TileState != ENxUIState::Selected )
	{
		SetState( ENxUIState::Hovered, ItemIcon->GetItemIcon() );
	}

	return Super::NativeOnMouseEnter( InGeometry, InMouseEvent );
}

void UNxInventoryTileViewItem::NativeOnMouseLeave( const FPointerEvent& InMouseEvent )
{
	if ( TileState != ENxUIState::Selected )
	{
		SetState( ENxUIState::Enable, ItemIcon->GetItemIcon() );
	}

	return Super::NativeOnMouseLeave( InMouseEvent );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 마우스 위치 구하는 함수
/// </summary>

FVector2D UNxInventoryTileViewItem::GetMousePosition( const FGeometry& InGeometry, const FVector2D InMousePosition )
{
	FVector2D mousePos = InGeometry.AbsoluteToLocal( InMousePosition );

	return mousePos + LocalPosition;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 아이템 데이터 입력 함수
/// </summary>

void UNxInventoryTileViewItem::UpdateItemData( const UNxInventoryTileViewItemData* InData )
{
	UpdateItemData( InData->GetInvenIndex(), InData->GetItemId() );
}

void UNxInventoryTileViewItem::UpdateItemData( const int16 InInvenIndex, const NxObjectId InItemId )
{
	if ( ItemData == nullptr )
	{
		ItemData = NewObject<UNxInventoryTileViewItemData>();
	}
	ItemData->SetItemData( InInvenIndex, InItemId );

	if ( GetItemId() != 0 )
	{
		bIsCanUsed = UNxItemManager::Instance()->IsCanUsedCharacterType( GetItemId() );
		const float remainsec = UNxItemManager::Instance()->GetRemainingCoolTime( GetItemId() );
		bIsCoolTimeActive = ( remainsec > 0 );

		if ( bIsCoolTimeActive == true )
		{
			StartCoolTime( remainsec );
		}

		DisplayItemInfo();
	}
	else
	{
		DisplayEmptyItem();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// // 아이템 데이터 얻는 함수
/// </summary>

TObjectPtr<UNxInventoryTileViewItemData> UNxInventoryTileViewItem::GetItemData() const
{
	return ItemData;
}

int16 UNxInventoryTileViewItem::GetInvenIndex() const
{
	return ItemData->GetInvenIndex();
}

NxObjectId UNxInventoryTileViewItem::GetItemId() const
{
	return ItemData->GetItemId();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 정리 함수
/// </summary>

void UNxInventoryTileViewItem::Clear()
{
	TileState = ENxUIState::Enable;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if ( TimerManager.TimerExists( CoolTimeDisplayHandle ) )
	{
		TimerManager.ClearTimer( CoolTimeDisplayHandle );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 아이템 표시 함수
/// </summary>

void UNxInventoryTileViewItem::DisplayItemInfo()
{
	ItemIcon->DisplayItemInfo( GetItemData() );
	DisplayItemDimBg();
	CoolTimeText->SetVisibility( ( bIsCoolTimeActive == true ) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed );
	if ( bIsCoolTimeActive == true )
	{
		DisplayItemCoolTime( UNxItemManager::Instance()->GetRemainingCoolTime( GetItemId() ) );
	}
}

void UNxInventoryTileViewItem::DisplayItemDimBg()
{
	DimBg->SetVisibility( ( bIsCanUsed == false || bIsCoolTimeActive == true ) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed );
}

void UNxInventoryTileViewItem::DisplayItemCoolTime( const float InRemainTime )
{
	FText textcooltime;
	float tempTime = FMath::CeilToFloat( InRemainTime );
	NX_LOG( TEXT( "DisplayItemCoolTime InRemainTime %f -> %f" ), InRemainTime, tempTime );

	CoolTimeText->SetVisibility( ( tempTime > 0.0f ) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed );
	if ( bIsCoolTimeActive == false ) return;

	if ( tempTime >= 60.0f )
	{
		textcooltime = FText::FromString( FString::Format( TEXT( "{0}M" ), { static_cast<int>( tempTime / 60.0f ) } ) );
	}
	else
	{
		textcooltime = FText::FromString( FString::Format( TEXT( "{0}" ), { static_cast<int>( tempTime ) } ) );
	}

	CoolTimeText->SetText( textcooltime );
}

void UNxInventoryTileViewItem::DisplayEmptyItem()
{
	bIsCanUsed = false;
	bIsCoolTimeActive = false;
	ItemIcon->SetVisible( false );
	DimBg->SetVisibility( ESlateVisibility::Collapsed );
	CoolTimeText->SetVisibility( ESlateVisibility::Collapsed );
	ClearTimerHandle();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 아이템 쿨타임 함수
/// </summary>

void UNxInventoryTileViewItem::StartCoolTime( const float InRemainSec )
{
	if ( InRemainSec > 0 )
	{
		// 쿨타임 표시 및 처리	
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject( this, &UNxInventoryTileViewItem::EndCoolTime );
			GetWorld()->GetTimerManager().SetTimer( CoolTimeUpdateHandle, TimerDelegate, InRemainSec, false );
			UNxItemManager::Instance()->AddCoolTimerHandel( GetItemId(), CoolTimeUpdateHandle );
		}
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject( this, &UNxInventoryTileViewItem::UpdateCoolTime );
			GetWorld()->GetTimerManager().SetTimer( CoolTimeDisplayHandle, TimerDelegate, 1.0f, true, 0.0f );
		}

		bIsCoolTimeActive = true;
	}
	else
	{
		bIsCoolTimeActive = false;
	}

	DisplayItemDimBg();
	CoolTimeText->SetVisibility( ( bIsCoolTimeActive == true ) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed );
	if ( bIsCoolTimeActive == true )
	{
		DisplayItemCoolTime( InRemainSec );
	}
}

void UNxInventoryTileViewItem::UpdateCoolTime()
{
	const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if ( TimerManager.TimerExists( CoolTimeUpdateHandle ) == false )
	{
		EndCoolTime();
		return;
	}

	const float RemainSec = TimerManager.GetTimerRemaining( CoolTimeUpdateHandle );

	if ( RemainSec >= 0.0f )
	{
		DisplayItemCoolTime( RemainSec );
	}
	else
	{
		EndCoolTime();
	}
}

void UNxInventoryTileViewItem::EndCoolTime()
{
	ClearTimerHandle();

	bIsCoolTimeActive = false;
	DisplayItemDimBg();
	CoolTimeText->SetVisibility( ESlateVisibility::Collapsed );
}

void UNxInventoryTileViewItem::ClearTimerHandle()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if ( TimerManager.TimerExists( CoolTimeUpdateHandle ) )
	{
		TimerManager.ClearTimer( CoolTimeUpdateHandle );
		UNxItemManager::Instance()->ClearCoolTimeHandle( GetItemId() );
	}

	if ( TimerManager.TimerExists( CoolTimeDisplayHandle ) )
	{
		TimerManager.ClearTimer( CoolTimeDisplayHandle );
	}
}