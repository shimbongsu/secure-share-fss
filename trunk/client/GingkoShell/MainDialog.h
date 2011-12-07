// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "resource.h"
#include "ListCtrl.h"
#include <psapi.h>
#include "FreakTabView.h"
#include "LoginDialog.h"
#include "AeroSupports.h"
#include "TrayIconImpl.h"

//#define WM_NOTIFY_EVENT_MESSAGE WM_USER + 1
#define SEND_LOGIN_EVENT		0x0010

extern HWND		_g_MainDialogHWND;
extern UINT		WM_WALKUP;
extern HANDLE	_gClientSyncEvent;
extern BOOL	__gGingkoSyncClientRunning;

DWORD WINAPI ThreadGingkoSyncEvent( LPVOID lpWnd );
DWORD FormatLastError(CString& szErrorText);
BOOL PASCAL AcceptUserPassword(int *, unsigned char*, int); 
BOOL PASCAL AcceptDecryptPassword(int *retLength, unsigned char* retPassword, int maxLength);
BOOL PASCAL EncrypFileProgress(LPVOID __pack, int percent, const TCHAR* pszMsg);
BOOL GetProcessExeFile(DWORD dwProcessId, TCHAR* szFileName, INT Length );
DWORD FormatNotifyMessage(GingkoNotifyMessage* pNotify, TCHAR* lpszMessage, DWORD MaxSize);
extern UINT WM_NOTIFY_EVENT_MESSAGE;
class CMainDialog : 
		public CUpdateUI<CMainDialog>,
		public CTrayIconImpl<CMainDialog>,
		public CResizableDialogImplT<CMainDialog, CWindow, aero::CDialogImpl<CMainDialog> >, 
		public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImplT<CMainDialog, CWindow, aero::CDialogImpl<CMainDialog> > baseClass;

private:
	CFreakTabView				m_TabViews;
	WTL::aero::CEdit			m_FilenameCtrl;
	CImageList					m_ilItemImages;
	CFont						m_fntCustomFont1;
	CFont						m_fntCustomFont2;
	CFont						m_font;
	WTL::aero::CButton			m_EncryptFileOnly;
	WTL::aero::CButton			m_btnDoOK;
	WTL::aero::CButton			m_btnDelete;
	WTL::aero::CButton			m_btnBrowse;
	WTL::aero::CButton			m_btnOption;
	WTL::aero::CButton			m_btnClose;
	WTL::aero::CStatic			m_lblFilename;
	WTL::aero::CButton			m_grpFilename;
	CPictureBox					m_PictureBox;
	GingkoDigitalPack*			m_DigitalPack;
	GingkoPermissionPack*		m_PermissionPack;
	const GingkoUserPack*		m_CurrentUser;
	CProgressBarCtrl			m_FileProgressCtrl;
	CString						m_DigitalFileName;
	DWORD						m_ThreadId;
	BOOL						m_WasEnabled;
	DWORD						m_dwGingkoSyncThreadId;
	//UINT						WM_NOTIFY_EVENT_MESSAGE;
public:
	enum { IDD = IDD_MAINDLG };

	CMainDialog()
	{
		//WM_NOTIFY_EVENT_MESSAGE = RegisterWindowMessage( _T("{743722BB-DD1C-4602-A939-60468FD1A132}") );

		m_DigitalPack = (GingkoDigitalPack*) malloc( sizeof(GingkoDigitalPack) );
		if( m_DigitalPack )
		{
			memset( m_DigitalPack, 0, sizeof(GingkoDigitalPack) );
		}
		m_PermissionPack = (GingkoPermissionPack*) malloc( sizeof(GingkoPermissionPack) );
		if( m_PermissionPack )
		{
			memset( m_PermissionPack, 0, sizeof(GingkoPermissionPack) );
		}
		m_CurrentUser = NULL; // (GingkoUserPack*) malloc( sizeof( GingkoUserPack ) );
		m_EnabledSizeGrip = FALSE;
		//if( m_CurrentUser )
		//{
		//	memset( m_CurrentUser, 0, sizeof(GingkoUserPack));
		//}
		//SetThemeClassList ( L"globals" );
	}

	~CMainDialog()
	{
		((ATL::CWindow)m_FilenameCtrl).Detach();
		
		((ATL::CWindow)m_TabViews).Detach();
		FreeDidgitalPack( m_DigitalPack, FALSE );
		if( m_DigitalPack )
		{
			free( m_DigitalPack );
			m_DigitalPack = NULL;
		}

		FreePermissionPack( m_PermissionPack, FALSE );
		
		if( m_PermissionPack )
		{
			free( m_PermissionPack );
			m_PermissionPack = NULL;
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

	BEGIN_UPDATE_UI_MAP(CMainDialog)
	END_UPDATE_UI_MAP()

	//DECLARE_WND_SUPERCLASS( _T( "{F669956B-B323-4245-91D4-0C3DF2DDF5DB}" ), NULL )
	

	BEGIN_MSG_MAP(CMainDialog)
		//COMMAND_HANDLER(IDC_BTN_ADDFAXNO, BN_CLICKED, OnBnClickedBtnAddfaxno)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint )
		MESSAGE_HANDLER(WM_NOTIFY_EVENT_MESSAGE, OnEventMessage )
		MESSAGE_HANDLER(WM_WALKUP, OnInstanceWalkup)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_BROWSER, OnBrowseFile)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancelWindows)
		COMMAND_ID_HANDLER(IDC_BTN_OPTIONS, OnOptions)
		COMMAND_ID_HANDLER(IDC_BTN_DELETE, OnDelete)
		COMMAND_ID_HANDLER(IDC_CHK_ENCRYPT_ONLY, OnCheckEncryptOnly)
		COMMAND_ID_HANDLER(ID_POPUP_GINGKOSHARING, OnPopupGingkoSharing)
		COMMAND_ID_HANDLER(ID_POPUP_GINGKOMAP, OnPopupGingkoMap)
		COMMAND_ID_HANDLER(ID_POPUP_OPTIONS, OnOptions)
		COMMAND_ID_HANDLER(ID_POPUP_LOGIN, OnLogin)
		COMMAND_ID_HANDLER(ID_POPUP_DECRYPTFILE, OnDecrypFile)
		COMMAND_ID_HANDLER(ID_POPUP_LOGOUT, OnLogout)
		COMMAND_ID_HANDLER(ID_POPUP_EXIT, OnPopupExit)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP( CTrayIconImpl<CMainDialog> )
		REFLECT_NOTIFICATIONS()
		//COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
	END_MSG_MAP()

	LRESULT OnEventMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{		
		GingkoNotifyMessage* gnm = NULL;
		bHandled = TRUE;
		gnm = (GingkoNotifyMessage*) LocalAlloc( GPTR, sizeof(GingkoNotifyMessage) );
		//gnm = (GingkoNotifyMessage*) malloc( sizeof(GingkoNotifyMessage) + 20 );
		if( gnm != NULL )
		{
			CImageList cImg;
			if( ReadNotifyMessage( gnm, sizeof(GingkoNotifyMessage) ) )
			{
				//pThis->ModifyIcon( szMsg, FALSE );
				LOG( L"ThreadGingkoSyncEvent at thread: %d.\n", GetCurrentThreadId() );
				DWORD dxSize = sizeof(GingkoNotifyMessage) + 50 * sizeof(TCHAR);
				TCHAR *pszMsg = (TCHAR*) malloc( dxSize );

				cImg.Create( 16, 16, 0, 0, 0 );

				if( pszMsg != NULL )
				{
					memset( pszMsg, 0, dxSize );
					_stprintf_s( pszMsg, dxSize / sizeof(TCHAR), 
						GetGlobalCaption( 60, _T("Application %s opened a share file (%s). The follow functions will be limited by this application:") ), 
						gnm->ProcessName, gnm->szFileName );

					ModifyIcon( (LPCTSTR) (pszMsg), FALSE, (ULONG) gnm->dwPermission );
					
					//LocalUnlock( pszMsg );
					free(pszMsg);
					//LocalFree( pszMsg );
				}
			}
			LocalFree( gnm );
			//free( gnm );
		}		
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		UpdateDialogUI( *this, IDD_MAINDLG );
		CWindow picBox = GetDlgItem( IDC_TOP_BAR );
		RECT rcPictureBox;
		RECT rcWnd;
		picBox.GetClientRect( &rcPictureBox );
		GetWindowRect( &rcWnd );
		rcPictureBox.left = 0;
		rcPictureBox.right = rcWnd.right - rcWnd.left + 1;
		picBox.ShowWindow( SW_HIDE );
		///m_PictureBox.SubclassWindow( GetDlgItem(IDC_MAIN_CONTAINER) );
		m_PictureBox.Create( *this, rcPictureBox, NULL,
				WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0);
		CString szLogoFile;
		GetRelatedPath( CString( LOGO_FILENAME ), szLogoFile );
		m_PictureBox.LoadBitmapFromFile( szLogoFile );
		m_PictureBox.ClippedPicture(true);
		m_PictureBox.UseMenu(false);
		m_PictureBox.CenterPicture(true);

		AddAnchor( m_PictureBox,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_FILE_INFO_GROUP,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_FILE_NAME,	TOP_LEFT, TOP_LEFT );
		AddAnchor( IDC_EDIT_FILENAME,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_PRG_FILEPROCESS,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_BROWSER,	TOP_RIGHT, TOP_RIGHT );
		AddAnchor( IDC_BTN_DELETE,	TOP_RIGHT, TOP_RIGHT );
		AddAnchor( IDC_CHK_ENCRYPT_ONLY,	TOP_LEFT, TOP_RIGHT );
		AddAnchor( IDC_TAB_PANEL,	TOP_LEFT, BOTTOM_RIGHT );
		AddAnchor( IDC_BTN_OPTIONS,	BOTTOM_LEFT, BOTTOM_LEFT );
		AddAnchor( IDOK,	BOTTOM_RIGHT, BOTTOM_RIGHT );
		AddAnchor( IDCANCEL,	BOTTOM_RIGHT, BOTTOM_RIGHT );

		m_TabViews.SubclassWindow( GetDlgItem( IDC_TAB_PANEL ) );
		m_TabViews.SetMinTabWidth( 120 );
		m_TabViews.InitializeTabs();
		//m_TabViews.SetTabStops();
		m_FilenameCtrl.SubclassWindow( GetDlgItem( IDC_EDIT_FILENAME ) );
		m_FilenameCtrl.SetTabStops();

		m_FileProgressCtrl.Attach( GetDlgItem(IDC_PRG_FILEPROCESS));
		m_FileProgressCtrl.SetRange( 0, 100 );

		m_EncryptFileOnly.SubclassWindow( GetDlgItem(IDC_CHK_ENCRYPT_ONLY) );
		
		m_btnBrowse.SubclassWindow( GetDlgItem(IDC_BROWSER) );
		m_btnOption.SubclassWindow( GetDlgItem(IDC_BTN_OPTIONS) );
		m_btnClose.SubclassWindow( GetDlgItem(IDCANCEL) );
		///m_grpFilename.SubclassWindow( GetDlgItem(IDC_FILE_INFO_GROUP) );
		//m_lblFilename.SubclassWindow( GetDlgItem(IDC_FILE_NAME) );
		
		AERO_CONTROL(CButton, _grpFilename, GetDlgItem(IDC_FILE_INFO_GROUP))
		AERO_CONTROL(CStatic, _lblFilename, GetDlgItem(IDC_FILE_NAME))

		m_btnDoOK.SubclassWindow( GetDlgItem(IDOK) );
		m_btnDelete.SubclassWindow( GetDlgItem( IDC_BTN_DELETE ) );
		m_btnDelete.EnableWindow( FALSE );
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		SetTimer( SEND_LOGIN_EVENT, 500, NULL );

		//StartThread( ThreadDetectServer, (LPVOID)this );

		if(IsCompositionEnabled())
		{
			MARGINS mar = {-1};
			//mar.cyTopHeight = 150;
			_DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
		}

		SetOpaqueUnder(m_PictureBox);
		//SetOpaqueUnder(IDC_TAB_PANEL);
		LOGFONT lf = {0};
		OpenThemeData(_T("globals"));

		if( !IsThemeNull() )
			GetThemeSysFont(TMT_MSGBOXFONT, &lf);
		else
		{
			NONCLIENTMETRICS ncm ={sizeof(NONCLIENTMETRICS)};
			SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),&ncm, false );
			lf = ncm.lfMessageFont;
		}

		lf.lfHeight *= 3;
		m_font.CreateFontIndirect ( &lf );
		EnableThemeDialogTexture(ETDT_ENABLE);
		
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXSMICON), 
											  ::GetSystemMetrics(SM_CYSMICON), 
											  LR_DEFAULTCOLOR);

		InstallIcon( GetGlobalCaption( CAPTION_GINGKO_SYSTEM, _T("Gingko Security Sharing System")), 
			hIconSmall, IDR_POPUP);

		SetIcon(hIconSmall, FALSE );

		HICON hIconBig = (HICON)::LoadImage(_Module.GetResourceInstance(), 
											  MAKEINTRESOURCE(IDI_GINGKOSHELL),
											  IMAGE_ICON, 
											  ::GetSystemMetrics(SM_CXICON), 
											  ::GetSystemMetrics(SM_CYICON), 
											  LR_DEFAULTCOLOR);

		SetIcon(hIconBig, TRUE );


		///SendMessage( WM_NOTIFY_EVENT_MESSAGE, 0 ,0 ); 

		//m_dwGingkoSyncThreadId = StartThread( ThreadGingkoSyncEvent, (PVOID)this->m_hWnd );

		//LOG( L"InitDialog at thread: %d.\n", GetCurrentThreadId() );

		return TRUE;
	}

	LRESULT OnInstanceWalkup(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DWORD dwSize = (DWORD) lParam;

		ShowWindow( SW_SHOW );
		SetActiveWindow();

		if( dwSize > 0 )
		{
			if( OpenClipboard() )
			{
				LPTSTR szText = (LPTSTR) malloc( dwSize * sizeof(TCHAR) );
				HANDLE hData = GetClipboardData( CF_TEXT );
				if( szText != NULL )
				{
					LPVOID lpSrc = GlobalLock( hData );
					memset( szText, 0, dwSize * sizeof(TCHAR) );
					if( lpSrc != NULL )
					{
						memcpy( szText, lpSrc, dwSize );
					}
					GlobalUnlock( hData );
					SetDigitalName( szText, szText );
					ModifyIcon( szText, TRUE );
					free( szText );
				}
				CloseClipboard();
			}
		}

		LOG( L"OnInstanceWalkup at thread: %d.\n", GetCurrentThreadId() );
		//TCHAR szMsg[1024];
		//_stprintf_s( szMsg, 1024, GetGlobalCaption(60, _T("Application was started. \nYou don't need start it again.")) , _T("MsWORD.exe"), _T("C:\\Users\\Long\\Desktop\\Test.doc") );
		//ModifyIcon( szMsg, FALSE, 0x8FFFF000 );
		
		//HANDLE hThread = CreateThread( NULL, 0, ThreadTest, this, 0, NULL );
		//SetEvent( g_TestEvent );
		
		//WaitForSingleObject( hThread, INFINITE );

		return TRUE;
	}

	void OnTimer(UINT nIDEvent)
	{
		if( nIDEvent == SEND_LOGIN_EVENT )
		{
			KillTimer( SEND_LOGIN_EVENT );
			
			WaitForDetectGingkoServer();

			if( !IsSessionValid( NULL ) )
			{
				UpdateMenu( FALSE );
				CLoginDialog loginDlg;
				if( IDOK != loginDlg.DoModal( *this ) )
				{
					CloseDialog(IDCANCEL);
					return;
				}

				///UpdateMenu( TRUE );

				if( m_DigitalFileName.GetLength() > 0 )
				{
					int last = m_DigitalFileName.ReverseFind( _T('\\') );
					CString fileTitle = m_DigitalFileName.Right( m_DigitalFileName.GetLength() - last );
					SetDigitalName( fileTitle, m_DigitalFileName );
				}

				UpdateMenu( TRUE );
			}else
			{
				UpdateMenu( TRUE );
			}

			PostClientProcessId();
		}
	}

	BOOL UpdateMenu( BOOL bEnable )
	{
		BOOL old = m_WasEnabled;
		m_WasEnabled = bEnable;
		return old;
	}


	BOOL SetProgress( int perc )
	{
		if( !m_FileProgressCtrl.IsWindowVisible() )
		{
			return FALSE;
		}
		
		m_FileProgressCtrl.SetPos( perc );

		if( perc >= 100 )
		{
			m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
			m_FilenameCtrl.ShowWindow( SW_SHOW );
			m_FileProgressCtrl.SetPos( 0 );
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

	LRESULT OnCheckEncryptOnly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//m_EncryptFileOnly.EnableWindow( m_EncryptFileOnly.GetCheck() );
		m_TabViews.EnableWindow( m_EncryptFileOnly.GetCheck() == 0 );
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		CRect rcGlassArea;
		if ( IsCompositionEnabled() )
		{
			GetClientRect ( rcGlassArea );	
			dc.FillSolidRect(rcGlassArea, RGB(0,0,0));
		}
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		//CAboutDlg dlg;
		//dlg.DoModal();
		return 0;
	}

	LRESULT OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		COptionsDialog optDlg;
		optDlg.DoModal();
		//CAboutDlg dlg;
		//dlg.DoModal();
		return 0;
	}
	
	LRESULT OnPopupExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(IDOK);
		return 0;
	}

	LRESULT OnPopupGingkoSharing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowWindow( SW_SHOW );
		return 0;
	}

	LRESULT OnPopupGingkoMap(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog ofndlg( TRUE );
		if( IDOK == ofndlg.DoModal() )
		{
			GingkoDigitalPack pack = {0};
			
			HANDLE hFile = CreateFile( ofndlg.m_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( hFile == INVALID_HANDLE_VALUE )
			{
				CString szFileMsg;
				szFileMsg.Format( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open the file(%s).") ),ofndlg.m_szFileName );
				MessageBox( szFileMsg, MESSAGE_WARNING_TITLE );
				return 0;
			}

			if( GingkoGetDigitalPack( hFile, &pack ) )
			{
				CGingkoAssociateDialog cadlg(&pack);
				cadlg.DoModal();
			}else
			{
				MessageBox( GetGlobalCaption( CAPTION_SELECT_DIGITAL_AGAIN, _T("The file is not a valid Gingko Digital format. Please select again.") ), MESSAGE_WARNING_TITLE );
			}
		}
		return 0;
	}

	LRESULT OnDecrypFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog ofndlg( TRUE );
		if( IDOK == ofndlg.DoModal() )
		{
			GingkoDigitalPack pack = {0};
			HANDLE hFile = CreateFile( ofndlg.m_szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			if( hFile == INVALID_HANDLE_VALUE )
			{
				CString szFileMsg;
				szFileMsg.Format( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open the file(%s).") ),ofndlg.m_szFileName );

				MessageBox( szFileMsg, MESSAGE_WARNING_TITLE );
				return 0;
			}


			if( GingkoGetDigitalPack( hFile, &pack ) )
			{
				CString szFileOutput(ofndlg.m_szFileName);
				int last = szFileOutput.ReverseFind( _T('.') );
				szFileOutput.Insert( last, _T(".decrypt") );
				HANDLE hOutputFile = CreateFile( szFileOutput, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				if( hOutputFile == INVALID_HANDLE_VALUE )
				{
					CString szFileMsg;
					szFileMsg.Format( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open the file(%s).") ),ofndlg.m_szFileName );
					MessageBox( szFileMsg, MESSAGE_WARNING_TITLE );
					CloseHandle( hFile );
					return 0;
				}
				ShowWindow( SW_SHOW );
				m_FileProgressCtrl.ShowWindow( SW_SHOW );  //HIDE IT TO CANCEL THE PROGRESS
				m_FilenameCtrl.ShowWindow( SW_HIDE );

				pack.ProgressCallback = EncrypFileProgress;
				pack.CallObject = this;

				if( !GingkoDecryptFile( hFile, hOutputFile, &pack, AcceptDecryptPassword ) )
				{
					const TCHAR* pMsg = NULL;
					GetLastServiceStatus( &pMsg );
					MessageBox( pMsg, MESSAGE_WARNING_TITLE );
				}

				m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
				m_FilenameCtrl.ShowWindow( SW_SHOW );

				CloseHandle( hFile );
				CloseHandle( hOutputFile );

			}else
			{
				MessageBox( GetGlobalCaption( CAPTION_SELECT_DIGITAL_AGAIN, _T("The file is not a valid Gingko Digital format.") ), MESSAGE_WARNING_TITLE );
			}
		}
		return 0;
	}


	

	LRESULT OnLogin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CLoginDialog loginDlg;		
		if( IDOK != loginDlg.DoModal( *this ) )
		{
			UpdateMenu( FALSE );
			//CloseDialog(IDCANCEL);
		}else
		{
			UpdateMenu( TRUE );
		}
		return 0;
	}

	LRESULT OnLogout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( MessageBox( GetGlobalCaption( CAPTION_WANT_LOGOUT, _T("Do yant want to logout?")), MESSAGE_CONFIRM_TITLE, MB_YESNO ) == IDYES )
		{
			logout();
			UpdateMenu( FALSE );
		}
		return 0;
	}

	virtual void PrepareMenu(HMENU hMenu)
	{
		CMenu menu;
		MENUITEMINFO mii={0};
		menu.Attach( hMenu );
		for( int i = 0; i < menu.GetMenuItemCount(); i ++ )
		{
			//menu.GetMenuItemInfo(i, TRUE, &mii);
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask  = MIIM_STRING;
			const TCHAR* caption = ::GetGlobalCaption( CAPTION_SYSTEM_MENU_START + i, NULL );
			if( caption != NULL )
			{
				mii.dwTypeData = (LPTSTR)_tcsdup(caption);
				menu.SetMenuItemInfo(i, TRUE, &mii );
			}
			
			UINT dwMenuFlag = 0;

			switch( i )
			{
			case 4:
				dwMenuFlag = MF_ENABLED | MF_BYPOSITION;
				break;
			case 9:
				dwMenuFlag = MF_ENABLED | MF_BYPOSITION;
				break;
			case 6:
				dwMenuFlag = (!m_WasEnabled) ? MF_ENABLED | MF_BYPOSITION : MF_DISABLED | MF_GRAYED | MF_BYPOSITION;
				break;
			case 7:
				dwMenuFlag = m_WasEnabled ? MF_ENABLED | MF_BYPOSITION : MF_DISABLED | MF_GRAYED | MF_BYPOSITION;
				break;
			default:
				if( m_WasEnabled )
				{
					dwMenuFlag = MF_ENABLED | MF_BYPOSITION;
				}else
				{
					dwMenuFlag = MF_DISABLED | MF_GRAYED | MF_BYPOSITION;
				}
			}
			menu.EnableMenuItem( i, dwMenuFlag );
		}
		menu.Detach();
	}

	LRESULT OnBrowseFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog fileDialog( TRUE, _T("*.*"), NULL, 4 | 2, _T("All Files (*.*)\0*.*\0"), *this );
		if( IDOK == fileDialog.DoModal() )
		{
			PostClientProcessId();
			m_FilenameCtrl.SetWindowText( fileDialog.m_szFileName );
			SetDigitalName( fileDialog.m_szFileTitle, fileDialog.m_szFileName );
		}
		return 0;
	}

	static DWORD WINAPI ThreadDetectServer( LPVOID lpWnd )
	{
		DetectGingkoServer();
		return 0;
	}


	DWORD StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, LPVOID pCaller)
	{
		DWORD ThreadId = -1;
		HANDLE hThread = CreateThread( NULL,0, ThreadRoutine, pCaller, CREATE_SUSPENDED, &ThreadId );
		if( hThread != NULL )
		{
			ResumeThread( hThread );
		}
		return ThreadId;
	}

	static DWORD WINAPI ThreadEncryptFile( LPVOID lpWnd)
	{
		CMainDialog* mdlg = static_cast<CMainDialog*>(lpWnd);
		CString szMessage;
		if( mdlg->m_DigitalPack != NULL && mdlg->m_DigitalPack->IsGingkoFile )
		{
			if( !mdlg->m_EncryptFileOnly.GetCheck() ) ///Don't update anything if this checkbox is checked.
			{
				///Update it
				GingkoDigitalPack *pDigitalPack = mdlg->m_DigitalPack;
				
				if( updateDigitalInfo( pDigitalPack->digitalId, pDigitalPack->author, 
					pDigitalPack->description, pDigitalPack->title, pDigitalPack->keyword, 
					pDigitalPack->status, pDigitalPack->securityType, pDigitalPack->permissionType, 
					pDigitalPack->limitationType ) )
				{
					mdlg->m_TabViews.UpdateAssignedPermission();
					szMessage.Format( GetGlobalCaption( CAPTION_FILE_ENCRYPTION_UPDATED, _T("%s 's information was updated successfully.") ), pDigitalPack->title );
					mdlg->MessageBox( szMessage, MESSAGE_CONFIRM_TITLE );
				}
			}

		}else{
			CString szNewFile;
			if( mdlg->CreateEncryptFile(szNewFile) )
			{
				mdlg->m_TabViews.UpdateAssignedPermission();
				GingkoDigitalPack *pDigitalPack = mdlg->m_DigitalPack;
				if( pDigitalPack != NULL ){
					szMessage.Format( GetGlobalCaption( CAPTION_FILE_ENCRYPTED_OK, _T("Gingko sharing file %s was created and saved to a new file with suffix \".gingko\" successfully.") ), pDigitalPack->title );
					mdlg->MessageBox( szMessage, MESSAGE_CONFIRM_TITLE );
					mdlg->SetDigitalName( szNewFile, szNewFile );
				}
			}
		}

		return 0;
	}

	BOOL CreateEncryptFile(CString& szNewFile)
	{
		const TCHAR *pMsg = NULL;
		SoapStatusEnum status = PACK_SUCCESS;
		CString szMessage;
		CString fileName;
		m_FilenameCtrl.GetWindowText( fileName );

		HANDLE hFile = CreateFile( fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == NULL || hFile == INVALID_HANDLE_VALUE )
		{
			szMessage.Format( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open file (%s).") ), fileName );
			MessageBox( szMessage, MESSAGE_ERROR_TITLE );
			return 0;
		}
		
		int last = fileName.ReverseFind(_T('.'));
		if( last > 0 )
		{
			fileName.Insert( last, _T(".gingko") );
		}else
		{
			fileName.Append( _T(".gingko") );
		}

		HANDLE hOutputFile = CreateFile( fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hOutputFile == NULL || hOutputFile == INVALID_HANDLE_VALUE )
		{
			szMessage.Format( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open file (%s).") ), fileName );
			MessageBox( szMessage, MESSAGE_ERROR_TITLE );
			return 0;
		}

		m_DigitalPack->ProgressCallback = EncrypFileProgress;
		m_DigitalPack->CallObject = this;
		m_DigitalPack->unitId = DuplicateFiledValue(_T("000000") );

		m_FileProgressCtrl.ShowWindow( SW_SHOW );  //HIDE IT TO CANCEL THE PROGRESS
		m_FilenameCtrl.ShowWindow( SW_HIDE );
		 
		if( m_EncryptFileOnly.GetCheck() )
		{
			if( !GingkoEncryptFile( hFile, hOutputFile, m_DigitalPack, AcceptUserPassword ) )
			{
				status = GetLastServiceStatus( &pMsg );
				CloseHandle( hFile );
				CloseHandle( hOutputFile );
				m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
				m_FilenameCtrl.ShowWindow( SW_SHOW );

				MessageBox( pMsg, MESSAGE_ERROR_TITLE );
				return 0;
			}
		}
		else
		{
			///Create the file and or check it.
			if( !GingkoEncryptFile( hFile, hOutputFile, m_DigitalPack, NULL ) )
			{
				status = GetLastServiceStatus( &pMsg );
				CloseHandle( hFile );
				CloseHandle( hOutputFile );	
				m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
				m_FilenameCtrl.ShowWindow( SW_SHOW );

				MessageBox( pMsg, MESSAGE_ERROR_TITLE );
				return 0;
			}
		}

		CloseHandle( hFile );
		CloseHandle( hOutputFile );

		m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
		m_FilenameCtrl.ShowWindow( SW_SHOW );

		if( !m_EncryptFileOnly.GetCheck() )
		{
			
			///Try to upload the DigitalContent information.
			if( !createDigitalInfo( m_DigitalPack->author, m_DigitalPack->description, m_DigitalPack->digitalId,
				m_DigitalPack->fileHash, m_DigitalPack->title, m_DigitalPack->keyword, m_DigitalPack->owner, 
				_T("Enabled"), 
				m_DigitalPack->securityType, m_DigitalPack->permissionType, m_DigitalPack->limitationType, 
				m_DigitalPack->PublicKey,m_DigitalPack->PrivateKey ) )
			{
				status = GetLastServiceStatus( &pMsg );
				MessageBox( pMsg, MESSAGE_ERROR_TITLE );
				return 0;
			}
		}

		szNewFile.Format( _T("%s"), fileName );
		
		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 

		CString szFilename;
		m_FilenameCtrl.GetWindowText( szFilename );

		if( szFilename.Trim().GetLength() == 0 )
		{
			MessageBox( GetGlobalCaption( CAPTION_SELECT_DIGITAL_AGAIN, _T("Please input/select a digital file and try again.") ), MESSAGE_WARNING_TITLE );
			return 0;
		}
		
		m_TabViews.GetDigitalPack( m_DigitalPack );

		while( !IsSessionValid( &m_CurrentUser ) )
		{
			CLoginDialog loginDlg;
			if( IDCANCEL == loginDlg.DoModal() )
			{
				return 0;
			}
		}

		PostClientProcessId();

		
		_g_MainDialogHWND = *this;

		HANDLE hThread = CreateThread( NULL, 0, ThreadEncryptFile, this, CREATE_SUSPENDED, &m_ThreadId );
		
		if( m_ThreadId != NULL )
		{
			ResumeThread( hThread );
		}
		

		return 0;
	}

	LRESULT OnCancelWindows(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if( m_FileProgressCtrl.IsWindowVisible() )
		{
			m_FileProgressCtrl.ShowWindow( SW_HIDE );  //HIDE IT TO CANCEL THE PROGRESS
			m_FilenameCtrl.ShowWindow( SW_SHOW );
		}else{
			//CloseDialog(wID);
			ShowWindow( SW_HIDE );
		}
		return 0;
	}


	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString fileName;
		CString szMessage;
		m_FilenameCtrl.GetWindowText( fileName );

		szMessage.Format( GetGlobalCaption( CAPTION_DELETE_FILE, _T("Do you want to delete %s?") ), fileName );
		
		if( MessageBox( szMessage , MESSAGE_CONFIRM_TITLE, MB_YESNO) == IDYES )
		{
			if( DeleteDigital( fileName ) )
			{
				///clear all
				FreeDidgitalPack( m_DigitalPack, FALSE );
				FreePermissionPack( m_PermissionPack, FALSE );
				m_FilenameCtrl.SetWindowText( _T("") );
				return 0;
			}
			const TCHAR *pszMessage = NULL;
			GetLastServiceStatus( &pszMessage );
			if( pszMessage != NULL )
			{
				MessageBox( pszMessage, MESSAGE_WARNING_TITLE );
			}
		}
		return 0;
	}


	BOOL DeleteDigital( const TCHAR* pszFilename )
	{
		BOOL RetValue = FALSE;
		HANDLE hFile = CreateFile( pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		
		if( hFile == NULL || hFile == INVALID_HANDLE_VALUE )
		{
			///CAPTION_FILE_OPEN_FAILD
			ThreadSetLastMessage( GetGlobalCaption( CAPTION_FILE_OPEN_FAILD, _T("Can not open file.") ), GetLastError() );
			return FALSE;
		}
		
		FreeDidgitalPack( m_DigitalPack, FALSE );

		FreePermissionPack( m_PermissionPack, FALSE );

		if( GingkoGetDigitalPack( hFile, m_DigitalPack ) )
		{
			if( m_DigitalPack != NULL )
			{
				
				if(  COMPARE_SELF_DIGITAL(m_DigitalPack->digitalId, m_DigitalPack->unitId) )
				{
					////Local encrypted file by password
					//m_EncryptFileOnly.EnableWindow( FALSE );
					//m_TabViews.EnableWindow( FALSE );
					///If it's a local file
					///SHOULD I DELETE IT BY THIS METHOD??
					ThreadSetLastMessage( GetGlobalCaption( CAPTION_DELETE_FILE_FIALD, _T("Don't delete local encryption file by this way.") ), 0 );
				}else{
					m_PermissionPack->deleted = FALSE;
					m_PermissionPack->holder = FALSE;
					m_PermissionPack->owner = FALSE;
					FreePackValue( m_PermissionPack, privateKey );
					FreePackValue( m_PermissionPack, publicKey );
					if( findDigitalPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, m_PermissionPack ) )
					{
						if( m_PermissionPack != NULL )
						{
							if( m_PermissionPack->deleted == FALSE && m_PermissionPack->deletable && (m_PermissionPack->holder || m_PermissionPack->owner ) )
							{
								if( updateDigitalInfo( m_DigitalPack->digitalId, m_DigitalPack->author, 
									m_DigitalPack->description, m_DigitalPack->title,
									m_DigitalPack->keyword, _T("Deleted"), m_DigitalPack->securityType,
									m_DigitalPack->permissionType, m_DigitalPack->limitationType ) )
								{
									RetValue = TRUE;
								}
							}else
							{
								ThreadSetLastMessage( GetGlobalCaption( CAPTION_NO_DELETE_RIGHT, _T("No rights to delete this file.") ), 0 );
							}
						}
					}
				}
			}
		}

		CloseHandle( hFile );

		return RetValue;
	}


	void SetDigitalName(const TCHAR* Filename, const TCHAR* Fullpath)
	{
		m_DigitalFileName = Fullpath;

		if( m_FilenameCtrl.IsWindow() )
		{
			m_FilenameCtrl.SetWindowText( m_DigitalFileName );
		}else
		{
			return;
		}

		WaitForDetectGingkoServer();

		while( !IsSessionValid( &m_CurrentUser ) )
		{
			if( IsWindow() && IsWindowVisible() )
			{
				CLoginDialog loginDlg;
				if( IDCANCEL == loginDlg.DoModal() )
				{
					return;
				}
				PostClientProcessId();
			}else
			{
				return;
			}
		}

		

		HANDLE hFile = CreateFile( Fullpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		
		if( hFile == NULL || hFile == INVALID_HANDLE_VALUE )
		{
			return;
		}



		m_EncryptFileOnly.EnableWindow( TRUE );
		
		m_btnDelete.EnableWindow( FALSE );

		FreeDidgitalPack( m_DigitalPack, FALSE );
		
		m_DigitalPack->IsGingkoFile = FALSE;

		FreePermissionPack( m_PermissionPack, FALSE );
		
		if( GingkoGetDigitalPack( hFile, m_DigitalPack ) )
		{
			if( m_DigitalPack != NULL )
			{
				if( COMPARE_SELF_DIGITAL(m_DigitalPack->digitalId, m_DigitalPack->unitId) )
				{
					////Local encrypted file by password
					m_EncryptFileOnly.EnableWindow( FALSE );
					m_TabViews.EnableWindow( FALSE );
					m_EncryptFileOnly.SetCheck(1);
				}else{
					m_PermissionPack->deleted = FALSE;
					m_PermissionPack->holder = FALSE;
					m_PermissionPack->owner = FALSE;
					FreePackValue( m_PermissionPack, privateKey );
					FreePackValue( m_PermissionPack, publicKey );
					m_EncryptFileOnly.SetCheck(0);
					if( findDigitalPermission( m_DigitalPack->unitId, m_DigitalPack->digitalId, m_PermissionPack ) )
					{
						m_btnDoOK.EnableWindow( m_PermissionPack->deleted == FALSE && (m_PermissionPack->owner || m_PermissionPack->holder) );
						m_btnDelete.EnableWindow(  m_PermissionPack->deleted == FALSE && m_PermissionPack->deletable == TRUE && (m_PermissionPack->owner || m_PermissionPack->holder));
					}else{
						m_btnDoOK.EnableWindow( FALSE );
					}

					m_EncryptFileOnly.EnableWindow( FALSE );
					m_TabViews.SetDigitalPack( m_DigitalPack, m_PermissionPack );
				}
			}
		}else
		{
			if( m_DigitalPack != NULL )
			{
				SetPackValue( m_DigitalPack, title, Filename );
			}

			if( m_CurrentUser != NULL )
			{
				if( m_CurrentUser->displayName != NULL )
				{
					SetPackValue( m_DigitalPack, author, m_CurrentUser->displayName );
				}
			}
			m_TabViews.SetDigitalPack( m_DigitalPack );
			m_btnDoOK.EnableWindow( TRUE );
		}

		m_TabViews.EnableWindow( m_EncryptFileOnly.GetCheck() == 0  );

		CloseHandle( hFile );
		m_btnDelete.Invalidate();
		m_btnBrowse.Invalidate();
		m_EncryptFileOnly.Invalidate();
	}
};

