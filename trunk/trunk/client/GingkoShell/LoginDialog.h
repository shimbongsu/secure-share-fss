#ifndef __LOGIN_DIALOG_H__
#define __LOGIN_DIALOG_H__
#include "resource.h"
#include "Extends\PictureBox.h"
#include "ListCtrl.h"
#include <psapi.h>
#include "AeroSupports.h"
#include "BalloonHelp.h"

#define SetEditValue(__edit, __val)    if( __val ){ __edit.SetWindowText( __val ); } 
#define GetEditValue(__edit, __options, __field)  	\
		if( __options != NULL ){					\
			CString __temp;							\
			__edit.GetWindowText( __temp );	\
			if( __options->__field ){			\
				GingkoFree( __options->__field );		\
			}										\
			__options->__field = DuplicateFiledValue( __temp );	\
		}											\


DWORD WINAPI DetectGingkoServerThread( LPVOID lpCaller );

#define DIRECT_CONNECTION "Direct"
#define SAMEIE_CONNECTION "SameIE"
#define CUSTOM_CONNECTION "Custom"

class COptionsDialog :
	public CDialogImpl<COptionsDialog>,
	public CMessageFilter, public CIdleHandler
{
	typedef CDialogImpl<COptionsDialog> BaseCOptionsDialog;
private:
	CEdit		m_MasterServer;
	CEdit		m_UnitServer;
	CEdit		m_ProxyServer;
	CEdit		m_ProxyPort;
	CEdit		m_ProxyUser;
	CEdit		m_ProxyPassword;
	CEdit		m_Timeout;
	CComboBox	m_CboLanguage;
	CButton		m_RdNoProxy;
	CButton		m_RdSameIe;
	CButton		m_RdCustom;
	

public:
	enum { IDD = IDD_OPTIONS_DIALOG };

	~COptionsDialog()
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

	BEGIN_MSG_MAP(COptionsDialog)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		///CHAIN_MSG_MAP(BaseCOptionsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		//MSG_WM_NCPAINT( OnNcPaint )
		COMMAND_ID_HANDLER(IDC_NO_PROXY, OnCheckProxy)
		COMMAND_ID_HANDLER(IDC_SAME_AS_IE, OnCheckProxy)
		COMMAND_ID_HANDLER(IDC_RD_CUSTOM, OnCheckProxy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		UpdateDialogUI( *this, IDD_OPTIONS_DIALOG );
		GingkoOptions *options = NULL;
		GetGingkoOptions( &options );


		m_MasterServer.Attach( GetDlgItem(IDC_MASTER_SERVER) );
		m_UnitServer.Attach( GetDlgItem(IDC_EDIT_UNIT_SERVER) );
		
		m_ProxyServer.Attach( GetDlgItem(IDC_EDIT_PROXYSERVER) );
		m_ProxyPort.Attach( GetDlgItem(IDC_EDIT_PROXYPORT) );
		m_ProxyUser.Attach( GetDlgItem(IDC_EDIT_PROXY_USERNAME) );
		m_ProxyPassword.Attach( GetDlgItem(IDC_EDIT_PROXY_PASSWORD) );
		
		m_Timeout.Attach( GetDlgItem(IDC_EDIT_CALL_TIMEOUT) );
		
		m_CboLanguage.Attach( GetDlgItem(IDC_CBO_LANGUAGE) );
		InitLanguageCombo();
		
		m_RdNoProxy.Attach( GetDlgItem(IDC_NO_PROXY) );
		m_RdSameIe.Attach( GetDlgItem(IDC_SAME_AS_IE) );
		m_RdCustom.Attach( GetDlgItem(IDC_RD_CUSTOM) );
		

		if( options != NULL )
		{
			SetEditValue( m_MasterServer, options->MasterServer );
			SetEditValue( m_UnitServer, options->UnitServer );
			SetEditValue( m_ProxyServer, options->ProxyServer );
			SetEditValue( m_ProxyPort, options->ProxyPort );
			SetEditValue( m_ProxyUser, options->ProxyUser );
			SetEditValue( m_ProxyPassword, options->ProxyPassword );
			SetEditValue( m_Timeout, options->MicsTimeout );
			//SetEditValue( m_CboLanguage, options->Language );
			if( options->Language )
			{
				m_CboLanguage.SelectString(0, options->Language );
			}

			if( options->Connection != NULL )
			{
				if( _tcscmp( options->Connection, _T(DIRECT_CONNECTION) ) == 0 )
				{
					m_RdNoProxy.SetCheck(1);
				}else if( _tcscmp( options->Connection, _T(SAMEIE_CONNECTION) ) == 0 ){
					m_RdSameIe.SetCheck(1);
				}else if( _tcscmp( options->Connection, _T(CUSTOM_CONNECTION) ) == 0 ){
					m_RdCustom.SetCheck(1);
				}
			}else{
				m_RdNoProxy.SetCheck(1);
			}
		}
		BOOL bHandled = FALSE;

		OnCheckProxy( 0, 0, NULL, bHandled);

		//HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
		//									  MAKEINTRESOURCE(IDI_GINGKOSHELL),
		//									  IMAGE_ICON, 
		//									  ::GetSystemMetrics(SM_CXSMICON), 
		//									  ::GetSystemMetrics(SM_CYSMICON), 
		//									  LR_DEFAULTCOLOR);

		//SetIcon(hIconSmall, FALSE );

		//HICON hIconBig = (HICON)::LoadImage(_Module.GetResourceInstance(), 
		//									  MAKEINTRESOURCE(IDI_GINGKOSHELL),
		//									  IMAGE_ICON, 
		//									  ::GetSystemMetrics(SM_CXICON), 
		//									  ::GetSystemMetrics(SM_CYICON), 
		//									  LR_DEFAULTCOLOR);

		//SetIcon(hIconBig, TRUE );
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		return 0;
	}

	BOOL InitLanguageCombo()
	{
		WIN32_FIND_DATA FindData;
		CString szFileLanguage(_T("\\Language_*.xml"));
		CString szFullPath;
		GetRelatedPath( szFileLanguage, szFullPath );
		HANDLE hFind = FindFirstFile( szFullPath, &FindData );
		if( hFind != INVALID_HANDLE_VALUE )
		{
			do{
				CString file( FindData.cFileName );
				file.Delete( 0, (int) _tcslen( _T("Language_") ) );
				file.Delete( file.GetLength() - 4, 4 );
				m_CboLanguage.AddString( file );
				m_CboLanguage.SelectString(0, file );
			}while( FindNextFile( hFind, &FindData ) );
			FindClose( hFind );
		}
		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		return 0;
	}

	LRESULT OnCheckProxy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		m_ProxyServer.EnableWindow( m_RdCustom.GetCheck() );
		m_ProxyPort.EnableWindow( m_RdCustom.GetCheck() );
		m_ProxyUser.EnableWindow( m_RdCustom.GetCheck() );
		m_ProxyPassword.EnableWindow( m_RdCustom.GetCheck() );
		return 0;
	}
	
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		USES_CONVERSION;
		//bHandled = TRUE;
		GingkoOptions *options = NULL;
		GetGingkoOptions( &options );

		GetEditValue( m_MasterServer, options, MasterServer );
		GetEditValue( m_UnitServer, options, UnitServer );
		GetEditValue( m_ProxyServer, options, ProxyServer );
		GetEditValue( m_ProxyPort, options, ProxyPort );
		GetEditValue( m_ProxyUser, options, ProxyUser );
		GetEditValue( m_ProxyPassword, options, ProxyPassword );
		GetEditValue( m_Timeout, options, MicsTimeout );
		GetEditValue( m_CboLanguage, options, Language );
		if( options->Connection )
		{
			GingkoFree( options->Connection );
			options->Connection = NULL;
		}
		if( m_RdNoProxy.GetCheck() ){
			options->Connection = DuplicateFiledValue(_T(DIRECT_CONNECTION));
		}else if( m_RdSameIe.GetCheck() ){
			options->Connection = DuplicateFiledValue(_T(SAMEIE_CONNECTION));
		}else if( m_RdCustom.GetCheck() ){
			options->Connection = DuplicateFiledValue(_T(CUSTOM_CONNECTION));
		}

		SetSoapConnectionOptions();

		StartThread( DetectGingkoServerThread, NULL );
		//DetectGingkoServer();

		CString szConfigFile;
		
		GetRelatedPath( CString(GINGKO_CONFIG_FILE), szConfigFile );

		if( SaveConfigFile( szConfigFile ) )
		{

			NotifyReloadConfig();
			EndDialog( IDOK );
		}
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		//bHandled = FALSE;
		EndDialog( IDCANCEL );
		return 0;
	}


	void CloseDialog(int nVal)
	{
		EndDialog(nVal);
	}


};

class CLoginDialog : 
	public CResizableDialogImplT<CLoginDialog, CWindow, aero::CDialogImpl<CLoginDialog> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CLoginDialog, CWindow, aero::CDialogImpl<CLoginDialog> > BaseCLoginDialog;

private:
	WTL::aero::CEdit					m_UserIdCtrl;
	WTL::aero::CEdit					m_PasswordCtrl;
	WTL::aero::CAeroHyperLink			m_NewRegister;
	WTL::aero::CAeroHyperLink			m_ForgetPasswd;
	CPictureBox							m_PictureBox;
	WTL::aero::CStatic					m_lblUserId;
	WTL::aero::CStatic					m_lblPassword;
	WTL::aero::CButton					m_btnOption;
	WTL::aero::CButton					m_btnLogin;

	//CWebBrowserCtrl m_WebBrowser;
	//CAxWindow m_Container;
public:
	enum { IDD = IDD_LOGIN_DIALOG };
	CLoginDialog()
	{
		m_EnabledSizeGrip = FALSE;
	}

	~CLoginDialog()
	{
		if( ::IsCompositionEnabled() )
		{
			//m_UserIdCtrl.UnsubclassWindow();
			//m_PasswordCtrl.UnsubclassWindow();
			//m_lblUserId.UnsubclassWindow();
			//m_lblPassword.UnsubclassWindow();
			//m_btnOption.UnsubclassWindow();
			//m_btnLogin.UnsubclassWindow();
			//m_NewRegister.UnsubclassWindow();
			//m_ForgetPasswd.UnsubclassWindow();
		}else
		{
			((ATL::CWindow)m_UserIdCtrl).Detach();
			((ATL::CWindow)m_PasswordCtrl).Detach();
		}
	}
 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}




	BEGIN_MSG_MAP(CLoginDialog)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(BaseCLoginDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		//MSG_WM_NCPAINT( OnNcPaint )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_OPTIONS, OnOptions)
		//CHAIN_MSG_MAP_MEMBER( m_WebBrowser );
		//MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
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

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		
		UpdateDialogUI( *this, IDD_LOGIN_DIALOG );
		m_UserIdCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_USERID) );
		m_PasswordCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_PASSWORD) );
		m_lblUserId.SubclassWindow( GetDlgItem(IDC_LBL_USERID) );
		m_lblPassword.SubclassWindow( GetDlgItem(IDC_LBL_PASSWORD) );
		m_btnOption.SubclassWindow( GetDlgItem(IDC_BTN_OPTIONS) );
		m_btnLogin.SubclassWindow( GetDlgItem(IDOK) );
		//m_WebBrowser.SubclassWindow( GetDlgItem( IDC_LOGIN_TOPBAR ) );
		//m_WebBrowser.Create(  );
		
		//m_Container.Attach( GetDlgItem(IDC_LOGIN_TOPBAR) );
		//CRect rc;
		//m_Container.GetClientRect( &rc );
		//HWND m_ClientHwnd = m_WebBrowser.Create( m_Container, rc , _T("http://www.google.cn"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
		//m_WebBrowser.Navigate2( _T("http://www.google.cn") );
		//AddAnchor( m_ClientHwnd, TOP_LEFT, TOP_RIGHT );

		CWindow picBox = GetDlgItem( IDC_LOGIN_TOPBAR );
		RECT rc;
		picBox.GetClientRect( &rc );
		picBox.ShowWindow( SW_HIDE );
		///m_PictureBox.SubclassWindow( GetDlgItem(IDC_MAIN_CONTAINER) );
		
		m_PictureBox.Create( *this, rc, NULL,
				WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0);
		m_PictureBox.ClippedPicture(true);
		CString szLogoFile;
		::GetRelatedPath( CString(LOGO_FILENAME), szLogoFile);
		m_PictureBox.LoadBitmapFromFile( szLogoFile );
		m_PictureBox.UseMenu(false);
		m_PictureBox.CenterPicture(true);
		//m_PictureBox.ClippedPicture(true);
		m_PictureBox.ClippedPicture(true);
		m_PictureBox.UseMenu(false);

		AddAnchor( m_PictureBox,TOP_LEFT, TOP_RIGHT );

		AddAnchor( IDC_LBL_USERID,	TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_USERID,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_NEW_REGISTER,	TOP_RIGHT, TOP_RIGHT );

		AddAnchor( IDC_LBL_PASSWORD,	TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_PASSWORD,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_LINK_GETPASSWORD,	TOP_RIGHT, TOP_RIGHT );

		AddAnchor( IDC_BTN_OPTIONS,	BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDOK,		BOTTOM_RIGHT, BOTTOM_RIGHT );		
		//m_WebBrowser
		//GetClientRect( &rc );
		//HWND m_ClientHwnd = m_Browser.Create( *this, rc );
		//HWND m_ClientHwnd = m_Browser.Create(m_hWnd, rc , _T("http://www.google.cn"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);

		

		m_NewRegister.SubclassWindow( GetDlgItem( IDC_NEW_REGISTER ) );
		m_ForgetPasswd.SubclassWindow( GetDlgItem( IDC_LINK_GETPASSWORD ) );

		m_NewRegister.SetHyperLink(_T("http://wwww.venusinfo.cn/gingkosystem/profile?action=register"));
		m_ForgetPasswd.SetHyperLink(_T("http://wwww.venusinfo.cn/gingkosystem/profile?action=forget"));
		
		
		//SetOpaqueUnder(m_NewRegister);
		//SetOpaqueUnder(m_ForgetPasswd);

		CenterWindow();

		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}

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
		//CloseDialog(wID);
		CString userId;
		CString password;
		m_UserIdCtrl.GetWindowText( userId );
		m_PasswordCtrl.GetWindowText( password );
		//m_UserIdCtrl
		//m_PasswordCtrl
		if( userId.Trim().GetLength() == 0 )
		{
			MessageBox( GetGlobalCaption(CAPTION_INPUT_USER_ID, _T("Please input userId.")), MESSAGE_WARNING_TITLE );
			return 0;
		}
		if( password.Trim().GetLength() == 0 )
		{
			MessageBox( GetGlobalCaption(CAPTION_INPUT_PASSWORD,_T("Please input password.")), MESSAGE_WARNING_TITLE );
			return 0;
		}
		
		if( WaitForDetectGingkoServer() )
		{
			if( gingkoLogin( userId, password ) )
			{
				PostClientProcessId();
				EndDialog( wID );
			}
			else
			{
				const TCHAR* pszMsg = NULL;
				GetLastServiceStatus( &pszMsg ); 
				if( pszMsg )
				{
					MessageBox( pszMsg, MESSAGE_WARNING_TITLE );
				}
			}
		}

		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog( wID );
		return 0;
	}

	
	LRESULT OnOptions(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowWindow( SW_HIDE );
		COptionsDialog optDlg;
		optDlg.DoModal();
		ShowWindow( SW_SHOW );
		return 0;
	}


	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	void OnNcPaint(CRgn rgn)
	{
		if( !rgn.IsNull() )
		{
			CDC cdc = GetWindowDC();
			cdc.FillRgn( rgn, CreateSolidBrush( 0xFFCCEE00 ) );
		}
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

/*	LRESULT OnWindowPosChanged(UINT, WPARAM wp, LPARAM lp, BOOL& bHandled )
	{
		RECT rc;
		bHandled = TRUE;
		SendMessage( m_WebBrowser, WM_WINDOWPOSCHANGED, wp, lp );
		return 0;
	}*/	
};


class CPasswordDialog : 
	public CResizableDialogImplT<CPasswordDialog, CWindow, aero::CDialogImpl<CPasswordDialog> >,
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CPasswordDialog, CWindow, aero::CDialogImpl<CPasswordDialog> > BaseCPasswordDialog;

private:
	WTL::aero::CEdit	m_RetypePasswdCtrl;
	WTL::aero::CEdit	m_PasswordCtrl;
	WTL::aero::CStatic	m_lblPassword;
	WTL::aero::CStatic	m_lblRetype;
	WTL::aero::CButton	m_btnOK;
	WTL::aero::CButton	m_btnCancel;
	CPictureBox		 m_PictureBox;
	//CWebBrowserCtrl m_WebBrowser;
	CString m_Password;
	BOOL	m_OnlyOne;

public:
	enum { IDD = IDD_PASSWORD_DIALOG };

	CPasswordDialog(BOOL OnlyOne = FALSE)
	{
		m_OnlyOne = OnlyOne;
		m_EnabledSizeGrip = FALSE;
	}

	~CPasswordDialog()
	{
		//((ATL::CWindow)m_RetypePasswdCtrl).Detach();
		//((ATL::CWindow)m_PasswordCtrl).Detach();
	}
 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_MSG_MAP(CPasswordDialog)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(BaseCPasswordDialog)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		//CHAIN_MSG_MAP_MEMBER( m_WebBrowser );
	END_MSG_MAP()

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		if( pLoop )
		{
			pLoop->RemoveMessageFilter(this);
			pLoop->RemoveIdleHandler(this);
		}
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		
		UpdateDialogUI( *this, IDD_PASSWORD_DIALOG );
		//CAxWindow m_Container = GetDlgItem( IDC_PASSWORD_TOPBAR );
		//CRect rc;
		//m_Container.GetClientRect( &rc );
		//HWND m_ClientHwnd = m_WebBrowser.Create( m_Container, rc , _T("http://www.google.cn"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
		//m_WebBrowser.Navigate2( _T("http://www.google.cn") );
		//AddAnchor( m_ClientHwnd, TOP_LEFT, TOP_RIGHT );
		CWindow picBox = GetDlgItem( IDC_PASSWORD_TOPBAR );
		RECT rcPictureBox;
		picBox.GetClientRect( &rcPictureBox );
		picBox.ShowWindow( SW_HIDE );
		///m_PictureBox.SubclassWindow( GetDlgItem(IDC_MAIN_CONTAINER) );
		
		m_PictureBox.Create( *this, rcPictureBox, NULL,
				WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0);
		m_PictureBox.UseMenu(false);
		CString logoFile;
		
		GetRelatedPath( CString(LOGO_FILENAME), logoFile );
		m_PictureBox.LoadBitmapFromFile(logoFile);
		m_PictureBox.UseMenu(false);
		m_PictureBox.ClippedPicture(true);
		m_PictureBox.CenterPicture(true);
		

		AddAnchor( m_PictureBox,TOP_LEFT, TOP_RIGHT );

		AddAnchor( IDC_LBL_PASSWORD,	TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_PASSWORD,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_LBL_PASSWORDRET,	TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_PASSWORD_RET,	TOP_LEFT, TOP_RIGHT );

		AddAnchor( IDOK,	BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDCANCEL,	BOTTOM_RIGHT, BOTTOM_RIGHT );
		
		CenterWindow();

		m_RetypePasswdCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_PASSWORD_RET) );
		m_PasswordCtrl.SubclassWindow( GetDlgItem(IDC_EDIT_PASSWORD) );

		if( m_OnlyOne )
		{
			m_RetypePasswdCtrl.EnableWindow( FALSE );
		}

		m_lblPassword.SubclassWindow( GetDlgItem(IDC_LBL_PASSWORD) );
		m_lblRetype.SubclassWindow( GetDlgItem(IDC_LBL_PASSWORDRET) );
		m_btnOK.SubclassWindow( GetDlgItem(IDOK) );
		m_btnCancel.SubclassWindow( GetDlgItem(IDCANCEL) );

		if( IsCompositionEnabled() )
		{
			MARGINS mar = {-1};
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}
		
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
		//ATLASSERT(pLoop != NULL);
		if( pLoop )
		{
			pLoop->AddMessageFilter(this);
			pLoop->AddIdleHandler(this);
		}

		BringWindowToTop();
		SetActiveWindow();
		///UIAddChildWindowContainer(m_hWnd);

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

	const TCHAR* GetPassword(INT* Length)
	{
		if( Length != NULL )
		{
			*Length = m_Password.GetLength();
		}
		return m_Password;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		//bHandled = TRUE;
		CString pass;
		CString retPass;
		if( !m_OnlyOne )
		{
			m_PasswordCtrl.GetWindowText( pass );
			m_RetypePasswdCtrl.GetWindowText( retPass );

			if( pass.Compare( retPass ) == 0 )
			{
				m_Password = pass;
				EndDialog( IDOK );
			}else
			{
				MessageBox( GetGlobalCaption( CAPTION_PASSWORD_NOT_EQUALS, _T("Password and Retype Password is not match")), MESSAGE_WARNING_TITLE );
			}
		}else
		{
			m_PasswordCtrl.GetWindowText( m_Password );
			EndDialog( IDOK );
		}
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		//bHandled = FALSE;
		EndDialog( IDCANCEL );
		return 0;
	}


	void CloseDialog(int nVal)
	{
		EndDialog(nVal);
	}
};





#endif
