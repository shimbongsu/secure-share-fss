// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DIGITALS_H__
#define __DIGITALS_H__

#include "resource.h"
#include "ListCtrl.h"
#include <psapi.h>
#include "LoginDialog.h"
#include "CGingkoAssociateDialog.h"
#include "AeroSupports.h"

class CDigitalShareForm;

BOOL IsValidEmail( CString& strEmail);
extern const CDigitalShareForm  *_g_pDigitalSharedDialog; 

class CRequestPermissionDialog : 
	public CResizableDialogImplT<CRequestPermissionDialog, CWindow, aero::CDialogImpl<CRequestPermissionDialog> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CRequestPermissionDialog, CWindow, aero::CDialogImpl<CRequestPermissionDialog> > baseClass;

private:
	aero::CButton m_ChkHolder;
	aero::CButton m_ChkOwner;
	aero::CButton m_ChkReadable;
	aero::CButton m_ChkWritable;
	aero::CButton m_ChkPrintable;
	aero::CButton m_ChkDeletable;
	aero::CEdit	m_TxtMessage;
	CPictureBox m_PictureBox;

	const GingkoDigitalPack*	m_DigitalPack;
	const GingkoPermissionPack* m_PermissionPack;

public:
	enum { IDD = IDD_REQ_PERM_DIALOG };

	CRequestPermissionDialog(const GingkoDigitalPack *DigitalPack, 
		const GingkoPermissionPack *PermissionPack)
	{
		m_DigitalPack = DigitalPack;
		m_PermissionPack = PermissionPack;
		m_EnabledSizeGrip = FALSE;
	}

	~CRequestPermissionDialog()
	{
	}
 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	void SetInitalValue()
	{
		if( m_PermissionPack != NULL )
		{
			m_ChkHolder.SetCheck( m_PermissionPack->holder );
			m_ChkOwner.SetCheck( m_PermissionPack->owner );
			m_ChkReadable.SetCheck( m_PermissionPack->readable );
			m_ChkWritable.SetCheck( m_PermissionPack->writable );
			m_ChkPrintable.SetCheck( m_PermissionPack->printable );
			m_ChkDeletable.SetCheck( m_PermissionPack->deletable );
		}
	}

	BEGIN_MSG_MAP(CRequestPermissionDialog)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsCompositionEnabled() )
		{
			CRect rcGlassArea;
			CPaintDC dc(m_hWnd);
			GetClientRect( rcGlassArea );
			//rcGlassArea.bottom = 150;
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		UpdateDialogUI( *this, IDD_REQ_PERM_DIALOG );

		CWindow picBox = GetDlgItem( IDC_RQP_TOP_BAR );
		RECT rc;
		picBox.GetClientRect( &rc );
		picBox.ShowWindow( SW_HIDE );
		///m_PictureBox.SubclassWindow( GetDlgItem(IDC_MAIN_CONTAINER) );
		
		m_PictureBox.Create( *this, rc, NULL,
				WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0);
		m_PictureBox.ClippedPicture(true);
		CString szLogoFile;
		::GetRelatedPath( CString(LOGO_FILENAME), szLogoFile );
		m_PictureBox.LoadBitmapFromFile( szLogoFile );
		m_PictureBox.UseMenu(false);
		m_PictureBox.CenterPicture(true);
		//m_PictureBox.ClippedPicture(true);
		m_PictureBox.ClippedPicture(true);
		m_PictureBox.UseMenu(false);

		AddAnchor( IDC_RQP_TOP_BAR, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_PERMS_REQ_GROUP, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_HOLDER, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_OWNER, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_READABLE, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_WRITABLE, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_PRINTABLE, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_CHK_DELETABLE, TOP_LEFT, TOP_RIGHT );

		AddAnchor( IDC_LBL_MESSAGE, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_EDIT_MESSAGE, TOP_LEFT, BOTTOM_RIGHT );
		
		AddAnchor( IDCANCEL,	BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDOK,		BOTTOM_RIGHT, BOTTOM_RIGHT );

		m_ChkHolder.SubclassWindow( GetDlgItem(IDC_CHK_HOLDER) );
		m_ChkOwner.SubclassWindow( GetDlgItem(IDC_CHK_OWNER) );
		m_ChkReadable.SubclassWindow( GetDlgItem(IDC_CHK_READABLE) );
		m_ChkWritable.SubclassWindow( GetDlgItem(IDC_CHK_WRITABLE) );
		m_ChkPrintable.SubclassWindow( GetDlgItem(IDC_CHK_PRINTABLE) );
		m_ChkDeletable.SubclassWindow( GetDlgItem(IDC_CHK_DELETABLE) );
		m_TxtMessage.SubclassWindow( GetDlgItem(IDC_EDIT_MESSAGE) );

		AERO_CONTROL( CButton, _grpPerms, GetDlgItem(IDC_PERMS_REQ_GROUP))
		AERO_CONTROL( CStatic, _lblMessage, GetDlgItem(IDC_LBL_MESSAGE))
		//AERO_CONTROL( CEdit,   _txtMessage, GetDlgItem(IDC_EDIT_MESSAGE))
		AERO_CONTROL( CButton, _btnOK, GetDlgItem(IDOK))
		AERO_CONTROL( CButton, _btnCancel, GetDlgItem(IDCANCEL))
		AERO_CONTROL( CStatic, _lblReqPerms, GetDlgItem(IDC_LBL_REQPERMS))
		

		SetInitalValue();

		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}

		SetOpaqueUnder(IDC_EDIT_MESSAGE);

		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXSMICON), 
											  ::GetSystemMetrics(SM_CYSMICON), 
											  LR_DEFAULTCOLOR);

		SetIcon(hIconSmall, FALSE );

		HICON hIconBig = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXICON), 
											  ::GetSystemMetrics(SM_CYICON), 
											  LR_DEFAULTCOLOR);

		SetIcon(hIconBig, TRUE );
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		///UIAddChildWindowContainer(m_hWnd);

		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString szMessage;
		m_TxtMessage.GetWindowText( szMessage );

		if( m_DigitalPack != NULL )
		{
			const GingkoUserPack *m_UserPack = NULL;

			if( IsSessionValid( &m_UserPack ) == FALSE )
			{
				CLoginDialog loginDlg;
				
				if( IDOK != loginDlg.DoModal() )
				{
					return 0;
				}

				if( IsSessionValid( &m_UserPack ) == FALSE )
				{
					return 0;
				}
			}

			PostClientProcessId();
			
			if( m_UserPack != NULL )
			{
				if( !requestPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, m_UserPack->gingkoId, 
					m_ChkOwner.GetCheck(), m_ChkHolder.GetCheck(), 
					m_ChkReadable.GetCheck(), m_ChkWritable.GetCheck(), 
					m_ChkPrintable.GetCheck(), m_ChkDeletable.GetCheck(), szMessage ) )
				{
					const TCHAR* pMessage = NULL;
					GetLastServiceStatus( &pMessage );
					MessageBox( pMessage, MESSAGE_ERROR_TITLE );
					return 0;
				}
			}
		}
		EndDialog( wID );
		return 0;
	}


	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog( wID );
		return 0;
	}

};




class CDigitalDetailForm : 
		public CResizableDialogImplT<CDigitalDetailForm, CWindow, aero::CDialogImpl<CDigitalDetailForm> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CDigitalDetailForm, CWindow, aero::CDialogImpl<CDigitalDetailForm> > baseClass;

private:
	aero::CEdit m_TitleCtrl;
	aero::CEdit m_AuthorCtrl;
	aero::CEdit m_KeywordCtrl;
	aero::CEdit m_DescCtrl;

public:
	enum { IDD = IDD_DETAIL };
	~CDigitalDetailForm()
	{
	}
 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	virtual void SetDigitalPack( GingkoDigitalPack *pack, GingkoPermissionPack *Perms = NULL )
	{
		if( pack != NULL )
		{
			m_TitleCtrl.SetWindowText( pack->title );
			m_AuthorCtrl.SetWindowText( pack->author );
			m_KeywordCtrl.SetWindowText( pack->keyword );
			m_DescCtrl.SetWindowText( pack->description );
		}

		if( Perms == NULL )
		{
			BOOL EnabledWnd = !IsValidGingkoDigital( pack );
			m_TitleCtrl.EnableWindow( EnabledWnd );
			m_AuthorCtrl.EnableWindow( EnabledWnd );
			m_KeywordCtrl.EnableWindow( EnabledWnd );
			m_DescCtrl.EnableWindow( EnabledWnd );
		}else if( Perms->deleted == FALSE )
		{
			if( pack->permissionType != NULL )
			{
				if( _tcscmp( _T("OWN"), pack->permissionType ) == 0 )
				{
					m_TitleCtrl.EnableWindow( Perms->owner );
					m_AuthorCtrl.EnableWindow( Perms->owner );
					m_KeywordCtrl.EnableWindow( Perms->owner );
					m_DescCtrl.EnableWindow( Perms->owner );
				}else{
					m_TitleCtrl.EnableWindow( Perms->owner || Perms->holder );
					m_AuthorCtrl.EnableWindow( Perms->owner || Perms->holder );
					m_KeywordCtrl.EnableWindow( Perms->owner || Perms->holder );
					m_DescCtrl.EnableWindow( Perms->owner || Perms->holder );
				}
			}
		}else{
			m_TitleCtrl.EnableWindow( FALSE );
			m_AuthorCtrl.EnableWindow( FALSE );
			m_KeywordCtrl.EnableWindow( FALSE );
			m_DescCtrl.EnableWindow( FALSE );
		}
	}


	virtual void GetDigitalPack( GingkoDigitalPack *pack )
	{
		if( pack != NULL )
		{
			CString strTitle;
			CString strAuthor;
			CString strKeyword;
			CString strDesc;
			m_TitleCtrl.GetWindowText( strTitle );
			SetPackValue( pack, title, strTitle );
			m_AuthorCtrl.GetWindowText( strAuthor );
			SetPackValue( pack, author, strAuthor );
			m_KeywordCtrl.GetWindowText( strKeyword );
			SetPackValue( pack, keyword, strKeyword );
			m_DescCtrl.GetWindowText( strDesc );
			SetPackValue( pack, description, strDesc );
		}
	}


	BEGIN_MSG_MAP(CDigitalDetailForm)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsCompositionEnabled() )
		{
			CRect rcGlassArea;
			CPaintDC dc(m_hWnd);
			GetClientRect( rcGlassArea );
			//rcGlassArea.bottom = 150;
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		UpdateDialogUI( *this, IDD_DETAIL );
		AddAnchor( IDC_TITLE, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_TITLE, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_AUTHOR, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_AUTHOR, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_KEYWORDS, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_KEYWORDS, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_DESCRIPTION, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_DESCRIPTION, TOP_LEFT, BOTTOM_RIGHT );

		m_TitleCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_TITLE) );
		m_AuthorCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_AUTHOR) );
		m_KeywordCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_KEYWORDS) );
		m_DescCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_DESCRIPTION) );

		if( IsCompositionEnabled() )
		{
			//AERO_CONTROL(CButton, _grpDetail, GetDlgItem(IDC_GROUP_DETAIL))
			AERO_CONTROL(CStatic, _lblTitle, GetDlgItem(IDC_TITLE))
			AERO_CONTROL(CStatic, _lblAuthor, GetDlgItem(IDC_AUTHOR))
			AERO_CONTROL(CStatic, _lblKeyword, GetDlgItem(IDC_KEYWORDS))
			AERO_CONTROL(CStatic, _lblDesc, GetDlgItem(IDC_DESCRIPTION))
		}
		
		SetOpaqueUnder(IDC_DESCRIPTION);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		///UIAddChildWindowContainer(m_hWnd);

		return TRUE;
	}
};





//public final static String SECURITY_PUBLIC_LIMIT    ="PL";
//public final static String SECURITY_PUBLIC_FULL     ="PF";
//public final static String SECURITY_PRIVATE         ="PR";
//public final static String SECURITY_REQUEST         ="RQ";
//public final static String SECURITY_NONE            ="NN";
//public final static String PERMISSION_OWNER_ONLY    ="OWN";
//public final static String PERMISSION_HOLDER_ENABLE ="HAE";
//public final static String PERMISSION_HOLDER_ASSIGN ="HAA";
//public final static String PERMISSION_ADMIN_ENABLE  ="SAE";
//public final static String PERMISSION_ADMIN_ASSIGN  ="SAA";
//public final static String LIMITATION_WORLDWIDE     ="WW";
//public final static String LIMITATION_UNIT          ="UN";
//public final static String LIMITATION_ORG           ="ORG";

extern TCHAR*       m_SharedOptionsArray[5][2];
extern TCHAR*       m_SecurityOptionsArray[5][2];

class CDigitalSecurityForm : 
		public CResizableDialogImplT<CDigitalSecurityForm, CWindow, aero::CDialogImpl<CDigitalSecurityForm> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CDigitalSecurityForm, CWindow, aero::CDialogImpl<CDigitalSecurityForm> > baseClass;
private:
	aero::CComboBoxEx m_SharedOptions;
	aero::CComboBoxEx m_SecurityOptions;
	aero::CButton     m_Worldwide;
	aero::CButton     m_Organization;
	aero::CButton     m_Unit;
	CImageList  m_ImageList;

public:
	enum { IDD = IDD_SECURITY };
	~CDigitalSecurityForm()
	{
		((ATL::CWindow)m_SharedOptions).Detach();
		((ATL::CWindow)m_SecurityOptions).Detach();
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	virtual void SetDigitalPack( GingkoDigitalPack *pack, GingkoPermissionPack *Perms = NULL )
	{
		if( pack )
		{
			m_SharedOptions.SetCurSel( 0 );
			if( pack->securityType )
			{
				for( int i = 0; i < 5; i ++ )
				{
					if( _tcscmp( m_SharedOptionsArray[i][0], pack->securityType ) == 0 )
					{
						m_SharedOptions.SetCurSel( i );
					}
				}
			}
			m_SecurityOptions.SetCurSel( 0 );
			if( pack->permissionType )
			{
				for( int i = 0; i < 5; i ++ )
				{
					if( _tcscmp( m_SecurityOptionsArray[i][0], pack->permissionType ) == 0 )
					{
						m_SecurityOptions.SetCurSel( i );
					}
				}
			}
			if( pack->limitationType )
			{
				if( _tcscmp( _T("WW"), pack->limitationType ) == 0 )
				{
					m_Worldwide.SetCheck(1);
					m_Organization.SetCheck(0);
					m_Unit.SetCheck(0);
				}else if( _tcscmp( _T("ORG"), pack->limitationType ) == 0 )
				{
					m_Worldwide.SetCheck(0);
					m_Organization.SetCheck(1);
					m_Unit.SetCheck(0);
				}else if( _tcscmp( _T("UNIT"), pack->limitationType ) == 0 )
				{
					m_Worldwide.SetCheck(0);
					m_Unit.SetCheck(1);
					m_Organization.SetCheck(0);
				}else 
				{
					m_Worldwide.SetCheck(1);
					m_Unit.SetCheck(0);
					m_Organization.SetCheck(0);
				}
			}
		}

		if( Perms == NULL )
		{
			BOOL EnabledWnd = !IsValidGingkoDigital( pack );
			m_Worldwide.EnableWindow( EnabledWnd );
			m_Unit.EnableWindow( EnabledWnd );
			m_Organization.EnableWindow( EnabledWnd );
			m_SecurityOptions.EnableWindow( EnabledWnd );
			m_SharedOptions.EnableWindow( EnabledWnd );
		}else if( Perms->deleted == FALSE )
		{
			if( pack->permissionType != NULL )
			{
				if( _tcscmp( _T("OWN"), pack->permissionType ) == 0 )
				{
					m_Worldwide.EnableWindow( Perms->owner  );
					m_Unit.EnableWindow( Perms->owner  );
					m_Organization.EnableWindow( Perms->owner  );
					m_SecurityOptions.EnableWindow( Perms->owner  );
					m_SharedOptions.EnableWindow( Perms->owner  );
				}else{
					m_Worldwide.EnableWindow( Perms->owner || Perms->holder  );
					m_Unit.EnableWindow( Perms->owner || Perms->holder  );
					m_Organization.EnableWindow( Perms->owner || Perms->holder );
					m_SecurityOptions.EnableWindow( Perms->owner || Perms->holder );
					m_SharedOptions.EnableWindow( Perms->owner || Perms->holder );
				}
			}
		}else{
			m_Worldwide.EnableWindow( FALSE );
			m_Unit.EnableWindow( FALSE );
			m_Organization.EnableWindow( FALSE );
			m_SecurityOptions.EnableWindow( FALSE );
			m_SharedOptions.EnableWindow( FALSE );
		}
	}

	virtual void GetDigitalPack( GingkoDigitalPack *pack )
	{
		if( pack )
		{
			if( m_Worldwide.GetCheck() )
			{
				SetPackValue( pack, limitationType, _T("WW") );
			}
			if( m_Organization.GetCheck() )
			{
				SetPackValue( pack, limitationType, _T("ORG") );
			}
			if( m_Unit.GetCheck() )
			{
				SetPackValue( pack, limitationType, _T("UNIT") );
			}
		//m_SharedOptions.SetCurSel( 3 );

		//m_SecurityOptions.SetCurSel( 3 );
			COMBOBOXEXITEM Item = {0};
			Item.mask = CBEIF_LPARAM;
			Item.iItem = m_SharedOptions.GetCurSel();
			if( m_SharedOptions.GetItem( &Item ) )
			{
				SetPackValue( pack, securityType, ((const TCHAR*)Item.lParam ));
			}

			Item.iItem = m_SecurityOptions.GetCurSel();
			if( m_SecurityOptions.GetItem( &Item ) )
			{
				SetPackValue( pack, permissionType, ((const TCHAR*)Item.lParam ));
			}

			//CComboBoxEx::GetCurSel();
			//CComboBoxEx::SetCurSel( 2 );
			//m_SharedOptions
		}
	}


	BEGIN_MSG_MAP(CDigitalDetailForm)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}
	
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsCompositionEnabled() )
		{
			CRect rcGlassArea;
			CPaintDC dc(m_hWnd);
			GetClientRect( rcGlassArea );
			//rcGlassArea.bottom = 150;
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		UpdateDialogUI( *this, IDD_SECURITY );

		m_ImageList.Create( IDB_LISTITEMS, 16, 0, 0 );
		
		m_SharedOptions.SubclassWindow( GetDlgItem(IDC_CMB_SHARED_OPTIONS) );
		for( int i = 0; i < 5; i ++ )
		{
			m_SharedOptions.AddItem( CBEIF_TEXT | CBEIF_LPARAM, GetGlobalCaption( i + CAPTION_SHARED_OPTIONS_START, m_SharedOptionsArray[i][1] ), 0, 0, 0, 0, (LPARAM)(m_SharedOptionsArray[i][0]) );
		}

		m_SecurityOptions.SubclassWindow( GetDlgItem(IDC_CMB_SECURITY_OPTIONS) );
		for( int i = 0; i < 5; i ++ )
		{
			m_SecurityOptions.AddItem( CBEIF_TEXT | CBEIF_LPARAM, GetGlobalCaption( i + CAPTION_SECURITY_OPTIONS_START, m_SecurityOptionsArray[i][1]), 0, 0, 0, 0, (LPARAM)(m_SecurityOptionsArray[i][0]) );
		}

		//AddAnchor( IDC_SECURITY_GROUP, TOP_LEFT, BOTTOM_RIGHT );
		AddAnchor( IDC_SHARED_OPTIONS, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_CMB_SHARED_OPTIONS, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_SECURITY_OPTIONS, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_CMB_SECURITY_OPTIONS, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_GROUP_LIMITS, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_RD_WW, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_RD_ORG, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_RD_UNIT, TOP_LEFT, TOP_RIGHT );

		m_Worldwide.SubclassWindow( GetDlgItem( IDC_RD_WW ) );
		
		m_Worldwide.SetCheck(1);
		
		m_Organization.SubclassWindow( GetDlgItem( IDC_RD_ORG ) );
		
		m_Unit.SubclassWindow( GetDlgItem( IDC_RD_UNIT ) );
		
		m_SharedOptions.SetCurSel(0);

		m_SecurityOptions.SetCurSel(0);

		if( IsCompositionEnabled() )
		{
			//AERO_CONTROL( CButton, _grpSecurity, GetDlgItem(IDC_SECURITY_GROUP) )
			AERO_CONTROL( CButton, _rdWorldwide, GetDlgItem(IDC_RD_WW) )
			AERO_CONTROL( CButton, _rdOrg, GetDlgItem(IDC_RD_ORG) )
			AERO_CONTROL( CButton, _rdUnit, GetDlgItem(IDC_RD_UNIT) )
			AERO_CONTROL( CButton, _grpLimits, GetDlgItem(IDC_GROUP_LIMITS) )
			AERO_CONTROL( CStatic, _lblShared, GetDlgItem(IDC_SHARED_OPTIONS) )
			AERO_CONTROL( CStatic, _lblSecurity, GetDlgItem(IDC_SECURITY_OPTIONS) )
		}
		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		return TRUE;
	}
};

class CGingkoUserComboBox :
	public CWindowImpl< CGingkoUserComboBox, CComboBoxEx >,
	public CMessageFilter, public CIdleHandler
{
	typedef CWindowImpl< CGingkoUserComboBox, CComboBox > baseClass;
		virtual BOOL PreTranslateMessage(MSG* pMsg)
		{
			return CWindow::IsDialogMessage (pMsg);
		}
		
		virtual BOOL OnIdle()
		{
			return FALSE;
		}

	public:

		BEGIN_MSG_MAP_EX(CGingkoUserComboBox)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_KEYDOWN,	OnKeyPress)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

		DECLARE_WND_SUPERCLASS(NULL, GetWndClassName())

		LRESULT OnKeyPress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )
		{
			
			return 0;
		}

		//==============================================================================
		/**
		 *		OnCreate:	Called in response to the WM_CREATE message
		 *
		 *		@return	0
		 */
		//==============================================================================
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )
		{
			CMessageLoop* pLoop = _Module.GetMessageLoop();
			ATLASSERT(pLoop != NULL);
			pLoop->AddMessageFilter(this);
			pLoop->AddIdleHandler(this);
			return 0;
		}

		LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
		{
			bHandled = FALSE;

			CMessageLoop* pLoop = _Module.GetMessageLoop();
			ATLASSERT(pLoop != NULL);
			pLoop->RemoveMessageFilter(this);
			pLoop->RemoveIdleHandler(this);

			return 0;
		}

};



class CGingkoPermission
{
public:
	CGingkoPermission()
	{
		m_holder = FALSE;
		m_owner = FALSE;
		m_holder = FALSE;
		m_deleted = FALSE;
		m_deletable = FALSE;
		m_readable = FALSE;
		m_printable = FALSE;
		m_writable = FALSE;
		m_actived = FALSE;
		m_dirty = TRUE;
	}
	
	CGingkoPermission(LPCTSTR lpszName, LPCTSTR lpszLoginId, LPCTSTR lpGingkoId )
	{
		m_displayName = lpszName;
		m_loginId = lpszLoginId;
		m_gingkoId = lpGingkoId;
		m_holder = FALSE;
		m_owner = FALSE;
		m_holder = FALSE;
		m_deleted = FALSE;
		m_deletable = FALSE;
		m_readable = FALSE;
		m_printable = FALSE;
		m_writable = FALSE;
		m_actived = FALSE;
		m_dirty = TRUE;
	}

	CGingkoPermission(const GingkoUserPack *pUser)
	{
		m_displayName.Format( _T("%s"), pUser->displayName );
		m_loginId.Format( _T("%s"), pUser->loginId );
		m_gingkoId.Format( _T("%s"), pUser->gingkoId );
		m_holder = FALSE;
		m_owner = FALSE;
		m_holder = FALSE;
		m_deleted = FALSE;
		m_deletable = FALSE;
		m_readable = FALSE;
		m_printable = FALSE;
		m_writable = FALSE;
		m_actived = FALSE;
		m_dirty = TRUE;
	}

	CGingkoPermission(const GingkoPermissionPack *pPerms)
	{
		m_displayName.Format( _T("%s"), pPerms->displayName );
		m_loginId.Format( _T("%s"), pPerms->loginId );
		m_gingkoId.Format( _T("%s"), pPerms->gingkoId );
		m_digitalId.Format( _T("%s"), pPerms->digitalId );
		m_unitId.Format( _T("%s"), pPerms->unitId );
		m_assignedBy.Format( _T("%s"), pPerms->assignedBy );
		m_assignedByName.Format( _T("%s"), pPerms->assignedByName );
		m_holder = pPerms->holder;
		m_owner = pPerms->owner;
		m_holder = pPerms->holder;
		m_deleted = pPerms->deleted;
		m_deletable = pPerms->deletable;
		m_readable = pPerms->readable;
		m_printable = pPerms->printable;
		m_writable = pPerms->writable;
		m_actived = TRUE;
		m_dirty = FALSE;
	}

	CString m_Id;
	CString m_loginId;
	CString m_gingkoId;
	CString m_displayName;
	CString m_status;
	CString m_email;
	CString m_unitId;
	CString m_digitalId;
	CString m_assignedBy;
	CString m_assignedByName;

	BOOL   m_holder;
	BOOL   m_owner;
	BOOL   m_deleted;
	BOOL   m_deletable;
	BOOL   m_readable;
	BOOL   m_printable;
	BOOL   m_writable;
	BOOL   m_actived;
	BOOL   m_dirty;

	BOOL   IsAbstractOwner()
	{
		if( m_assignedBy.Trim().Compare( _T("") ) != 0 && m_gingkoId.Trim().Compare(_T("") ) != 0 )
		{
			return m_assignedBy.Trim().Compare( m_gingkoId.Trim() ) == 0;
		}
		return FALSE;
	}
};

class CListGingkoUser : public CListImpl< CListGingkoUser >
{
public:
	DECLARE_WND_CLASS( _T( "AxGingkoUser" ) )

protected:
	//CImageList m_ilItemImages;
	CListArray< CGingkoPermission >		m_aProps;
	CListArray<CGingkoPermission>		m_DelProps;
	COLORREF							m_DisableBackground;
	COLORREF							m_DisableText;
	COLORREF							m_DisableIdColor;
	GingkoDigitalPack*					m_pDigitalPack;
	GingkoPermissionPack*				m_PermPack;
	BOOL								m_HasEdited;
	DWORD								m_dwThreadId;

protected:
	CString PreProcessString(LPCTSTR pszText)
	{
		return pszText;
	}

public:
	BOOL Initialise()
	{
		BOOL inited = CListImpl< CListGingkoUser >::Initialise();
		if( inited )
		{
			AddColumn(  _T( "" ), 20, ITEM_IMAGE_CHECKBOX, TRUE, ITEM_FORMAT_CHECKBOX, ITEM_FLAGS_READ_ONLY );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_EMAIL, _T("Email")), 100, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_NONE );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_USERNAME, _T("Username")), 100); ///Hide this field
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_OWNER, _T("Owner")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX);
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_HOLDER, _T("Holder")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_READABLE, _T("Readable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_PRINTABLE, _T("Printable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_WRITABLE, _T("Writable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
			AddColumn( GetGlobalCaption(CAPTION_USER_LIST_DELETABLE, _T("Deletable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
			m_HasEdited = FALSE;
		}
		
		return inited;
	}

	static DWORD WINAPI FetchDigitalPermissionThread( LPVOID pCaller )
	{
		if( pCaller != NULL )
		{
			CListGingkoUser *pListGingkoUser = static_cast<CListGingkoUser*>(pCaller);
			if( pListGingkoUser->m_pDigitalPack != NULL )
			{
				GingkoDigitalPack *gdp = pListGingkoUser->m_pDigitalPack;
				findAssignedPermission( gdp->unitId, gdp->digitalId, pCaller,
					CListGingkoUser::FetchDigitalPermission );
				
			}
			pListGingkoUser->m_dwThreadId = 0;
		}
		return 0;
	}

	static BOOL WINAPI FetchDigitalPermission( LPVOID pCaller, const GingkoPermissionPack* Pack )
	{
		if( pCaller != NULL )
		{
			CListGingkoUser *pListGingkoUser = static_cast<CListGingkoUser*>(pCaller);		
			pListGingkoUser->AddGingkoUser( Pack );
			return TRUE;
		}
		return FALSE;
	}

	void SetDigitalPack( GingkoDigitalPack *pack, GingkoPermissionPack *Perms = NULL)
	{
		m_HasEdited = FALSE;
		m_pDigitalPack = pack;
		m_PermPack = Perms;
		DeleteAllItems();
		HANDLE hThread = NULL;
		
		if( m_dwThreadId != 0 )
		{
			hThread = OpenThread( THREAD_ALL_ACCESS, TRUE, m_dwThreadId );
			if( hThread != INVALID_HANDLE_VALUE )
			{
				TerminateThread( hThread, 0 );
			}
		}

		hThread = CreateThread( NULL, 0, CListGingkoUser::FetchDigitalPermissionThread, (LPVOID)this, CREATE_SUSPENDED, &m_dwThreadId );
		if( hThread != INVALID_HANDLE_VALUE )
		{
			ResumeThread( hThread );
		}
	}

	const GingkoDigitalPack* GetDigitalPack()
	{
		return m_pDigitalPack;
	}

	
	int GetItemCount() // required by CListImpl
	{
		return m_aProps.GetSize();
	}
		
	CString GetItemText( int nItem, int nSubItem ) // required by CListImpl
	{
		//CListArray < CGingkoUser >::operator []
		if( nItem < 0 || nItem >= m_aProps.GetSize() )
		{
			return _T("");
		}

		CGingkoPermission gu = m_aProps[nItem];
		
		CString ret;
		switch( nSubItem )
		{
		case 0:
			ret.Format( _T("%d"), gu.m_actived );
			return ret;
		case 1:
			return gu.m_loginId;
		case 2:
			return gu.m_displayName;
		case 3:
			ret.Format( _T("%d"), gu.m_owner );
			return ret;
		case 4:
			ret.Format( _T("%d"), gu.m_holder );
			return ret;
		case 5:
			ret.Format( _T("%d"),  gu.m_readable );
			return ret;
		case 6:
			ret.Format( _T("%d"), gu.m_writable);
			return ret;
		case 7:
			ret.Format( _T("%d"), gu.m_printable);
			return ret;
		case 8:
			ret.Format( _T("%d"), gu.m_deletable);
			return ret;
		}
		return _T( "" );
	}

	int AddGingkoUser( CGingkoPermission gu )
	{
		m_aProps.Add( gu );
		m_HasEdited = TRUE;
		return CListImpl< CListGingkoUser >::AddItem() ? GetItemCount() - 1 : -1;
	}

	int AddGingkoUser( CString szLoginId, CString szName, CString szGingkoId )
	{
		return AddGingkoUser( CGingkoPermission( szLoginId, szName, szGingkoId ) );
	}

	int AddGingkoUser( GingkoUserPack* pUser)
	{
		return AddGingkoUser( CGingkoPermission( pUser ) );
	}
	
	int AddGingkoUser( const GingkoPermissionPack *pPerm )
	{
		return AddGingkoUser( CGingkoPermission( pPerm ) );
	}

	CListArray<CGingkoPermission>& GetDeletePermissions()
	{
		return m_DelProps;
	}

	CListArray<CGingkoPermission>& GetUpdatedPermissions()
	{
		return m_aProps;
	}
	
	int GetItemImage( int nItem, int nSubItem ) // overrides CListImpl::GetItemImage
	{
		return 0;
	}

	/****
	 * Change the ItemColours to identify how is the cell of item.
	 * Color for READ-ONLY
	 * Color for ABSTRACTOWNER etc.
	 ****/
	BOOL GetItemColours( int nItem, int nSubItem, COLORREF& rgbBackground, COLORREF& rgbText )
	{
			//rgbBackground = m_DisableBackground;
			//rgbText       = m_DisableIdColor;
		return CListImpl< CListGingkoUser >::GetItemColours( nItem, nSubItem, rgbBackground, rgbText );
	}

	UINT GetItemFormat( int nItem, int nSubItem )
	{
		if( nSubItem == 0 )
		{
			return ITEM_FORMAT_CHECKBOX;
		}else if( nSubItem == 1 || nSubItem == 2 )
		{
			return ITEM_FORMAT_NONE;
		}else
		{
			return ITEM_FORMAT_CHECKBOX;
		}

		//return listSubItem.m_nFormat == ITEM_FORMAT_NONE ? GetColumnFormat( IndexToOrder( nSubItem ) ) : listSubItem.m_nFormat;
	}
	
	UINT GetItemFlags( int nItem, int nSubItem )
	{
		return  GetColumnFlags( IndexToOrder( nSubItem ) ); // : listSubItem.m_nFlags;
	}
	
	BOOL GetItemComboList( int nItem, int nSubItem, CListArray < CString >& aComboList )
	{
		//if( nItem < 2 && nSubItem != colType )
		return FALSE;
		//aComboList=m_aComboList;
		//return !aComboList.IsEmpty(); // ? GetColumnComboList( IndexToOrder( nSubItem ), aComboList ) : !aComboList.IsEmpty();
	}

	BOOL DeleteItem( int nItem )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;

		if( m_aProps[nItem].IsAbstractOwner() )
		{
			return FALSE;  ///THIS RECORD CAN NOT BE DELETED.
		}

		if( m_aProps[nItem].m_actived )
		{
			CGingkoPermission gp = m_aProps[nItem];
			m_DelProps.Add( gp );
		}

		m_HasEdited = TRUE;

		return m_aProps.RemoveAt( nItem ) ? CListImpl<CListGingkoUser>::DeleteItem( nItem ) : FALSE;
	}
	
	BOOL DeleteAllItems()
	{
		m_aProps.RemoveAll();
		m_DelProps.RemoveAll();
		m_HasEdited = FALSE;
		return CListImpl<CListGingkoUser>::DeleteAllItems();
	}

	void ClearDirty()
	{
		for( int i = 0; i < m_aProps.GetSize(); i ++ )
		{
			m_aProps[i].m_dirty = FALSE;
		}
	}
	

	/***
	 * Permission who has rights to change other user's the permission while be handle here.
	 ***/
	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		
		///NOT ALLOWED TO CHANGED THE ABSTRACT OWNER, THIS RECORD WILL BE CREATED BY UPLOAD THE DEFAULT INFORMATION.
		if( m_aProps[nItem].IsAbstractOwner() ){
			return FALSE;
		}

		BOOL bPermAllow = m_PermPack == NULL && !IsValidGingkoDigital( m_pDigitalPack );
		
		if( m_PermPack != NULL )
		{
			if( m_pDigitalPack != NULL && m_pDigitalPack->permissionType != NULL )
			{
				if( _tcscmp( _T("OWN"), m_pDigitalPack->permissionType ) == 0 )
				{
					bPermAllow = m_PermPack->deleted == FALSE && m_PermPack->owner;
				}else if( _tcscmp( _T("HAE"), m_pDigitalPack->permissionType ) == 0 ){
					if( nSubItem == 3 || nSubItem == 4 )
					{
						bPermAllow =  m_PermPack->deleted == FALSE && ( m_PermPack->owner );
					}
					else
					{
						bPermAllow = m_PermPack->deleted == FALSE && ( m_PermPack->owner || m_PermPack->holder );
					}
				}else if( _tcscmp( _T("HAA"), m_pDigitalPack->permissionType ) == 0 ){
					if( nSubItem == 3 )
					{
						bPermAllow = m_PermPack->deleted == FALSE && ( m_PermPack->owner );
					}else{
						bPermAllow = m_PermPack->deleted == FALSE && ( m_PermPack->owner || m_PermPack->holder );
					}
				}else if( _tcscmp( _T("SAA"), m_pDigitalPack->permissionType ) == 0 || _tcscmp( _T("SAE"), m_pDigitalPack->permissionType ) == 0){
					bPermAllow = m_PermPack->deleted == FALSE && ( m_PermPack->owner || m_PermPack->holder );
				}
			}else{
				bPermAllow = FALSE;
			}

			if( bPermAllow )
			{
				/// If the current record is a the user's record
				if( m_aProps[nItem].m_gingkoId.Compare( m_PermPack->gingkoId ) == 0 )
				{
					bPermAllow = FALSE;
				}
			}
		}

		if( !bPermAllow )
		{
			return FALSE;
		}

		//CGingkoUser gu = m_aProps[nItem];
		switch(nSubItem)
		{
		case 3:
			m_aProps[nItem].m_owner = _ttol( lpszText ) != 0;
			break;
		case 4:
			m_aProps[nItem].m_holder = _ttol( lpszText ) != 0;
			break;
		case 5:
			m_aProps[nItem].m_readable = _ttol( lpszText ) != 0;
			break;
		case 6:
			m_aProps[nItem].m_writable = _ttol( lpszText ) != 0;
			break;
		case 7:
			m_aProps[nItem].m_printable = _ttol( lpszText ) != 0;
			break;
		case 8:
			m_aProps[nItem].m_deletable = _ttol( lpszText ) != 0;
			break;
		default:
			return FALSE;
		}

		m_aProps[nItem].m_dirty = TRUE;
		m_HasEdited = TRUE;
		
		return TRUE;
	}
	
	void ReverseItems() // overrides CListImpl::ReverseItems
	{
		m_aProps.Reverse();
	}

	BOOL HasEdited()
	{
		return m_HasEdited;
	}
	
	class CompareItem
	{
	public:
		CompareItem( int cu ) : m_colColumn( cu ) {}
		inline bool operator() ( const CGingkoPermission& prop, const CGingkoPermission& prs )
		{
			switch( m_colColumn )
			{
				case 0:
					return prop.m_actived < prs.m_actived;
				case 1:
					return prop.m_loginId.Compare( prs.m_loginId ) < 0;
				case 2:
					return prop.m_displayName.Compare( prs.m_displayName ) < 0;
				case 3:
					return prop.m_holder < prs.m_holder;
				case 4:
					return prop.m_owner < prs.m_owner;
				case 5:
					return prop.m_readable < prs.m_readable;
				case 6:
					return prop.m_writable < prs.m_writable;
				case 7:
					return prop.m_printable < prs.m_printable;
				case 8:
					return prop.m_deletable < prs.m_deletable;
				default:
					return false;
			}
			return false;
		}
		
	protected:
		int m_colColumn;
	};
	
	void SortItems( int nColumn, BOOL bAscending ) // overrides CListImpl::SortItems
	{
		m_aProps.Sort( CompareItem( nColumn ) );
	}
};


class CDigitalShareForm : 
		public CResizableDialogImplT<CDigitalShareForm, CWindow, aero::CDialogImpl<CDigitalShareForm> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CDigitalShareForm, CWindow, aero::CDialogImpl<CDigitalShareForm> > baseClass;
private:
	CListGingkoUser		m_Users;
	CFont				m_fntCustomFont1;
	CFont				m_fntCustomFont2;
	GingkoDigitalPack*	m_DigitalPack;
	GingkoPermissionPack*	m_PermissionPack;
	aero::CComboBoxEx		m_Email;
	aero::CButton			m_btnCheckAdd;
	aero::CButton			m_btnShareMap;
	aero::CButton			m_btnRemove;
	aero::CButton			m_btnRequest;
	CString			m_szOldEmail;
public:
	enum { IDD = IDD_SHARE };
	CDigitalShareForm()
	{
		m_DigitalPack = NULL;
		m_PermissionPack = NULL;
	}

	~CDigitalShareForm()
	{
		m_DigitalPack = NULL;
		m_Users.Detach();
		//m_Email.Detach();
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	virtual void SetDigitalPack( GingkoDigitalPack *pack , GingkoPermissionPack *Perms = NULL)
	{
		m_DigitalPack = pack;
		m_Users.SetDigitalPack( pack, Perms );
		m_PermissionPack = Perms;
		if( Perms == NULL )
		{
			//m_Users.EnableWindow( TRUE );
			BOOL EnabledWnd = !IsValidGingkoDigital( pack );
			m_Email.EnableWindow( EnabledWnd );
			m_btnCheckAdd.EnableWindow( EnabledWnd );
			m_btnRemove.EnableWindow( EnabledWnd );
			m_btnShareMap.EnableWindow( EnabledWnd );
		}else if( Perms->deleted == FALSE )
		{
			if( pack->permissionType != NULL )
			{
				if( _tcscmp( _T("OWN"), pack->permissionType ) == 0 )
				{
					//m_Users.EnableWindow( Perms->owner );
					m_Email.EnableWindow( Perms->owner );
					m_btnCheckAdd.EnableWindow( Perms->owner );
					m_btnRemove.EnableWindow( Perms->owner );
					m_btnShareMap.EnableWindow( Perms->owner );
				}else{
					///m_Users.EnableWindow( Perms->owner || Perms->holder );
					m_Email.EnableWindow( Perms->owner || Perms->holder );
					m_btnCheckAdd.EnableWindow( Perms->owner || Perms->holder );
					m_btnRemove.EnableWindow( Perms->owner || Perms->holder );
					m_btnShareMap.EnableWindow( Perms->owner || Perms->holder );
				}
			}
		}else{
			m_Email.EnableWindow( FALSE );
			m_btnCheckAdd.EnableWindow( FALSE );
			m_btnRemove.EnableWindow( FALSE );
			m_btnShareMap.EnableWindow( FALSE );
		}

		if( m_DigitalPack == NULL )
		{
			m_btnRequest.EnableWindow(FALSE);
		}else{
			m_btnRequest.EnableWindow(TRUE);
		}

		if( pack != NULL && pack->securityType != NULL )
		{
			if( _tcscmp( _T("NN"), pack->securityType ) == 0 || _tcscmp( _T("PR"), pack->securityType ) == 0)
			{
				m_btnRequest.EnableWindow(FALSE);
			}
		}
	}

	virtual void GetDigitalPack( GingkoDigitalPack *pack )
	{
		//Nothing to do
	}

	BEGIN_MSG_MAP(CDigitalShareForm)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MSG_WM_TIMER(OnTimer)
		COMMAND_ID_HANDLER(IDC_BTN_CHECK_ADD, OnCheckAndAdd)
		COMMAND_ID_HANDLER(IDC_EDIT_EMAIL, OnChangedValue)
		COMMAND_ID_HANDLER(IDC_BTN_REMOVE_USER, OnRemoveUser)
		COMMAND_ID_HANDLER(IDC_BTN_REQUEST, OnRequestPermission)
		COMMAND_ID_HANDLER(IDC_BTN_SHARE_ROUTE, OnShowShareMap)
		//COMMAND_HANDLER( IDC_EDIT_EMAIL, WM, OnEmailKeydown )
		//CHAIN_MSG_MAP_MEMBER(m_Email)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}

	void InitUserListCtrl()
	{
		//m_Users.DeleteAllItems();
		//m_Users.RemoveAllColumns();
		//m_Users.AddColumn(  _T( "" ), 0, ITEM_IMAGE_CHECKBOX, TRUE, ITEM_FORMAT_CHECKBOX, ITEM_FLAGS_READ_ONLY );
		//m_Users.AddColumn( GetGlobalCaption(14, _T("Email")), 100, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_NONE );
		//m_Users.AddColumn( GetGlobalCaption(15, _T("Username")), 100); ///Hide this field
		//m_Users.AddColumn( GetGlobalCaption(16, _T("Owner")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX);
		//m_Users.AddColumn( GetGlobalCaption(17, _T("Holder")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
		//m_Users.AddColumn( GetGlobalCaption(18, _T("Readable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
		//m_Users.AddColumn( GetGlobalCaption(19, _T("Printable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
		//m_Users.AddColumn( GetGlobalCaption(20, _T("Writable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
		//m_Users.AddColumn( GetGlobalCaption(21, _T("Deletable")), 80, ITEM_IMAGE_NONE, FALSE, ITEM_FORMAT_CHECKBOX );
	}


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		
		UpdateDialogUI( *this, IDD_SHARE );
		//AddAnchor( IDC_SHARE_GROUP, TOP_LEFT, BOTTOM_RIGHT );
		AddAnchor( IDC_USER_EMAIL, TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_EMAIL, TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_BTN_CHECK_ADD, TOP_RIGHT, TOP_RIGHT );
		AddAnchor( IDC_USER_LIST, TOP_LEFT, BOTTOM_RIGHT );
		AddAnchor( IDC_BTN_SHARE_ROUTE, BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDC_BTN_REMOVE_USER, BOTTOM_RIGHT, BOTTOM_RIGHT );
		AddAnchor( IDC_BTN_REQUEST, BOTTOM_CENTER, BOTTOM_CENTER );
		m_Users.SubclassWindow( GetDlgItem(IDC_USER_LIST) );

		//LOGFONT logFont = { 0 };
		//logFont.lfCharSet = DEFAULT_CHARSET;
		//logFont.lfHeight = 90;
		//lstrcpy( logFont.lfFaceName, _T( "New Times Roman" ) );
		//logFont.lfWeight = FW_BOLD;
		//logFont.lfItalic = (BYTE)TRUE;
		//
		//m_fntCustomFont1.CreatePointFontIndirect( &logFont );
		//
		//logFont.lfHeight = 100;
		//lstrcpy( logFont.lfFaceName, _T( "Arial" ) );
		//logFont.lfUnderline = (BYTE)TRUE;
		//m_fntCustomFont2.CreatePointFontIndirect( &logFont );

		m_Users.SetFocusSubItem( TRUE );

		//InitUserListCtrl();

		//m_Email.Attach( GetDlgItem( IDC_EDIT_EMAIL ) );
		//CComboBoxEx::Attach
		m_Email.SubclassWindow( GetDlgItem( IDC_EDIT_EMAIL ) );
		m_btnCheckAdd.SubclassWindow( GetDlgItem(IDC_BTN_CHECK_ADD) );
		m_btnShareMap.SubclassWindow( GetDlgItem(IDC_BTN_SHARE_ROUTE) );
		m_btnRemove.SubclassWindow( GetDlgItem( IDC_BTN_REMOVE_USER ) );
		m_btnRequest.SubclassWindow( GetDlgItem( IDC_BTN_REQUEST ) );

		SetDigitalPack( NULL, NULL );


		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}

		//AERO_CONTROL( CButton, _grpShare, GetDlgItem( IDC_SHARE_GROUP) )
		AERO_CONTROL( CStatic, _lblUser, GetDlgItem( IDC_USER_EMAIL) )
		//AERO_CONTROL( CComboBoxEx,   _editEmail, GetDlgItem( IDC_EDIT_EMAIL) )
		//AERO_CONTROL( CButton, _btnCheckAdd, GetDlgItem( IDC_BTN_CHECK_ADD) )
		AERO_CONTROL( CListBox,_lstUser, GetDlgItem( IDC_USER_LIST) )
		//AERO_CONTROL( CButton, _btnRouteMap, GetDlgItem( IDC_BTN_SHARE_ROUTE) )
		//AERO_CONTROL( CButton, _btnRemoveUser, GetDlgItem( IDC_BTN_REMOVE_USER) )
		//AERO_CONTROL( CButton, _btnRequest, GetDlgItem( IDC_BTN_REQUEST) )

		SetOpaqueUnder(IDC_USER_LIST);
		SetOpaqueUnder(IDC_EDIT_EMAIL);

		CenterWindow();
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		return TRUE;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsCompositionEnabled() )
		{
			CRect rcGlassArea;
			CPaintDC dc(m_hWnd);
			GetClientRect( rcGlassArea );
			//rcGlassArea.bottom = 150;
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		bHandled = FALSE;
		return 0;
	}


	void OnTimer(UINT nIDEvent)
	{
		if( nIDEvent == 1 )
		{
			CString szEmail;
			m_Email.GetWindowText(szEmail);
			if( szEmail.GetLength() > 3 )
			{
				if( szEmail.Compare( m_szOldEmail ) != 0 )
				{
					while ( m_Email.GetCount() > 0 )
					{
						COMBOBOXEXITEM cItem = {0};
						cItem.mask = CBEIF_LPARAM;
						cItem.iItem = 0;
						m_Email.GetItem( &cItem ); 
						if( cItem.lParam != NULL )
						{
							GingkoUserPack *pUser = (GingkoUserPack*) cItem.lParam;
							FreeUserPack( pUser, FALSE );
							cItem.lParam = NULL;
						}
						m_Email.DeleteItem(0);
					}
					_g_pDigitalSharedDialog = this;
					if( !searchUsers( szEmail, CDigitalShareForm::FetchUserPack ) )
					{
						//error process
					}
					KillTimer( nIDEvent );
					m_Email.ShowDropDown(1);
					m_szOldEmail.Empty();
					m_szOldEmail.Append(szEmail);
				}
			}else{
					while ( m_Email.GetCount() > 0 )
					{
						COMBOBOXEXITEM cItem = {0};
						cItem.mask = CBEIF_LPARAM;
						cItem.iItem = 0;
						m_Email.GetItem( &cItem ); 
						if( cItem.lParam != NULL )
						{
							GingkoUserPack *pUser = (GingkoUserPack*) cItem.lParam;
							FreeUserPack( pUser, FALSE );
							cItem.lParam = NULL;
						}
						m_Email.DeleteItem(0);
					}				
			}
		}
	}

	static BOOL GINGKO_API FetchUserPack( GingkoUserPack *pUser )
	{
		CDigitalShareForm *form;
		form = (CDigitalShareForm *)_g_pDigitalSharedDialog;
		form->AddFindUser( pUser );
		return TRUE;
	}

	void AddFindUser( GingkoUserPack *pUser )
	{
		CString userTitle;
		userTitle.Format( _T("%s(%s)"), pUser->displayName, pUser->loginId );
		m_Email.AddItem( userTitle, 0, 0, 0, (LPARAM) pUser );
	}

	
	LRESULT OnChangedValue(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//m_szOldEmail.Empty();
		//m_Email.GetWindowText( m_szOldEmail );
		
		KillTimer( 1 );
		CString szEmail;
		CString szCurText;
		m_Email.GetWindowText( szEmail );
		if( m_Email.GetCurSel() != -1 )
		{
			m_Email.GetItemText( m_Email.GetCurSel(), szCurText );
		}
		if( szEmail.Compare( szCurText ) != 0 )
		{
			SetTimer( 1, 500, NULL );
		}
		return 0;
	}

	LRESULT OnShowShareMap(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( m_DigitalPack != NULL )
		{
			CGingkoAssociateDialog assocDialog(m_DigitalPack);
			assocDialog.DoModal();
		}else
		{
			MessageBox( GetGlobalCaption(CAPTION_NOT_SELECT_DIGITAL, _T("No digital information be specialized.")), MESSAGE_WARNING_TITLE );
		}
		return 0;
	}

	/****
	 * Check the input user's default permission by calling 
	 * findDigitalPermission();
	 * The default permission will be used to setting up this digital. If the current digital's Id is not present,
	 * this call will not be raised.
	 ****/
	LRESULT OnCheckAndAdd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		COMBOBOXEXITEM cItem = {0};
		cItem.mask = CBEIF_TEXT | CBEIF_LPARAM;
		cItem.iItem = m_Email.GetCurSel();
		m_Email.GetItem( &cItem );
		CString szEmail;
		m_Email.GetWindowText( szEmail );
		if( cItem.lParam == NULL && !IsValidEmail( szEmail ))
		{
			CString szMessage;
			szMessage.Format( GetGlobalCaption( CAPTION_NOT_REGISTER_USER, _T("%s is not registered in system. And it's not a valid email form. System can not add it.") ), szEmail );
			MessageBox( szMessage, MESSAGE_WARNING_TITLE );
			return 0;
		}

		
		GingkoUserPack *pUser = (GingkoUserPack*) cItem.lParam;

		if( pUser )
		{
			m_Users.AddGingkoUser( pUser );
		}else{
			m_Users.AddGingkoUser( szEmail, szEmail, _T("") );
		}
		//m_Users.SetItemText( item, 1,  pUser->loginId );
		//m_Users.SetItemText( item, 2,  pUser->displayName );
		
		return 0;
	}

	LRESULT OnRemoveUser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListArray<int> SelectedItems;
		m_Users.GetSelectedItems(SelectedItems);
		SelectedItems.Sort();
		for( int i = SelectedItems.GetSize() - 1; i >= 0; i -- )
		{
			m_Users.DeleteItem(SelectedItems[i]);
		}

		return 0;
	}


	LRESULT OnRequestPermission(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CRequestPermissionDialog RequestDialog( m_DigitalPack, m_PermissionPack );
		RequestDialog.DoModal( );
		return 0;
	}
	
	BOOL UpdateAssignedPermission()
	{
		///This method will update the assigned permission by the m_Users' table
		if( m_DigitalPack != NULL )
		{
			if( m_DigitalPack->unitId == NULL || m_DigitalPack->digitalId == NULL )
			{
				return FALSE;
			}
		}
		///Here, I will remove all item in the delted items.
		CListArray<CGingkoPermission> DelPerms = m_Users.GetDeletePermissions();
		for( int i = 0; i < DelPerms.GetSize(); i ++ )
		{
			if( assignPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, 
				DelPerms[i].m_loginId, DelPerms[i].m_gingkoId, DelPerms[i].m_owner, 
				DelPerms[i].m_holder, DelPerms[i].m_readable, DelPerms[i].m_writable, 
				DelPerms[i].m_printable, DelPerms[i].m_deletable, FALSE ) )
			{
				DelPerms[i].m_dirty = FALSE; ///use color to identify it
			}
		}

		CListArray<CGingkoPermission> Perms = m_Users.GetUpdatedPermissions();
		for( int i = 0; i < Perms.GetSize(); i ++ )
		{
			if( Perms[i].m_dirty )
			{
				if( assignPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, 
					Perms[i].m_loginId, Perms[i].m_gingkoId, Perms[i].m_owner, 
					Perms[i].m_holder, Perms[i].m_readable, Perms[i].m_writable, 
					Perms[i].m_printable, Perms[i].m_deletable, TRUE ) )
				{
					Perms[i].m_dirty = FALSE; ///use color to identify it
				}
			}
		}
		m_Users.ClearDirty();
		///UPDATE THE BELOW ERROR
		return TRUE;
	}
	
};

#endif
