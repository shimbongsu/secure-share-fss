#pragma once
#include "resource.h"
#include "Digitals.h"
#include "Extends\TabViewCtrl.h"

class CFreakTabView :
	public CFreakTabViewCtrl
{
protected:
	enum
	{
		TABWINDOW_DETAIL		= 0,
		TABWINDOW_SECURITY	= 1,
		TABWINDOW_SHARING	= 2,
		TABWINDOW_SYS_APPCONTROL	= 3,
	};

private:
	CDigitalDetailForm m_AppDetailView;
	CDigitalSecurityForm m_AppSecurityView;
	CDigitalShareForm m_AppShareView;
	GingkoDigitalPack *pack;
public:
	CFreakTabView(void){
		pack = NULL;
	}

	~CFreakTabView(void){
		pack = NULL;
	}


	CFreakTabView( HWND hWnd ):
	CFreakTabViewCtrl( hWnd )
	{
		pack = NULL;
	}

	void InitializeTabs()
	{		
		m_AppDetailView.Create( *this );
		
		AddTab( GetGlobalCaption( CAPTION_TAB_DETAIL, _T("Detail") ), m_AppDetailView, TRUE, TABWINDOW_DETAIL, (LPARAM) &m_AppDetailView);

		m_AppSecurityView.Create( *this );
		AddTab( GetGlobalCaption( CAPTION_TAB_SECURITY, _T("Security")), m_AppSecurityView, FALSE, TABWINDOW_SECURITY, (LPARAM) &m_AppSecurityView );
		
		m_AppShareView.Create( *this );
		AddTab( GetGlobalCaption( CAPTION_TAB_SHARING, _T("Share") ), m_AppShareView, FALSE, TABWINDOW_SHARING, (LPARAM) &m_AppShareView );
	}

	void SetDigitalPack(GingkoDigitalPack *pck, GingkoPermissionPack *Perms = NULL)
	{
		pack = pck;

		if( Perms == NULL )
		{
			m_AppDetailView.EnableWindow( TRUE );
			m_AppSecurityView.EnableWindow( TRUE );
			m_AppShareView.EnableWindow( TRUE );
		}else if( pck->permissionType != NULL ){
			if( Perms->deleted == FALSE && (Perms->holder == TRUE || Perms->owner == TRUE ) )
			{
				///THE USER WILL CAN CHANGE THE SOMETHING
				if( _tcscmp( _T("OWN"), pck->permissionType ) == 0 )
				{
					m_AppDetailView.EnableWindow( Perms->owner );
					m_AppSecurityView.EnableWindow( Perms->owner );
					m_AppShareView.EnableWindow( Perms->owner );
				}else if( _tcscmp( _T("HAE"), pck->permissionType ) == 0 )
				{
					m_AppDetailView.EnableWindow( Perms->owner || Perms->holder );
					m_AppSecurityView.EnableWindow( Perms->owner || Perms->holder );
					m_AppShareView.EnableWindow( Perms->owner || Perms->holder );
				}else if( _tcscmp( _T("HAA"), pck->permissionType ) == 0 )
				{
					m_AppDetailView.EnableWindow( Perms->owner || Perms->holder );
					m_AppSecurityView.EnableWindow( Perms->owner || Perms->holder );
					m_AppShareView.EnableWindow( Perms->owner || Perms->holder );
				}
			}else{
					m_AppDetailView.EnableWindow( FALSE );
					m_AppSecurityView.EnableWindow( FALSE );
					m_AppShareView.EnableWindow( FALSE );
			}
		}

		m_AppDetailView.SetDigitalPack( pck,   Perms );
		m_AppSecurityView.SetDigitalPack( pck, Perms );
		m_AppShareView.SetDigitalPack( pck,    Perms );
	}

	void GetDigitalPack(GingkoDigitalPack *pck)
	{
		m_AppDetailView.GetDigitalPack( pck );
		m_AppSecurityView.GetDigitalPack( pck );
		m_AppShareView.GetDigitalPack( pck );
	}

	void UpdateAssignedPermission()
	{
		m_AppShareView.UpdateAssignedPermission();
	}



	//==============================================================================
	//	WTL
	//==============================================================================

	public:
		
		//LRESULT OnSelectionChanged( LPNMHDR )
		//{
		//	SetActiveTab( GetCurSel( ) );
		//
		//	return 0;
		//}

		virtual BOOL PreTranslateMessage(MSG* pMsg)
		{
			
			pMsg;
			return FALSE;
		}

		//BEGIN_MSG_MAP_EX(CFreakTabView)
		//	CHAIN_MSG_MAP(CFreakTabViewCtrl)
		//	//CHAIN_MSG_MAP_MEMBER(CFreakTabView::GetWindow())
		//	//REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelectionChanged)
		//END_MSG_MAP()

};
