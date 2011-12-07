// GingkoShell.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GingkoShell.h"
#include "MainDialog.h"
#include "LoginDialog.h"
#include "XmlConfigParse.h"
#include "dwmapi_delegate.h"

#define _GINGKO_INSTANCE_MUTEX  _T("{B15B2814-5344-41f7-902B-735BE755FA03}")
#define _GINGKO_SYNC_EVENT  _T("Global\\SyncEvent-{6909DB0C-2564-4431-B18A-B46ECC1E01A8}")

CAppModule _Module;
BOOL	__gGingkoSyncClientRunning = FALSE;
HWND	_g_MainDialogHWND = NULL;
HANDLE	_gSingleInstanceMutex = NULL;
HANDLE	_gClientSyncEvent = NULL;
UINT	WM_WALKUP = RegisterWindowMessage( _T("{33B96FE9-68EA-47b4-BC19-F5071639CBE2}") );
UINT	WM_NOTIFY_EVENT_MESSAGE = RegisterWindowMessage( _T("{A049D36B-5EB0-4b36-83EA-BF24453E971C}") );
DWORD WINAPI DetectGingkoServerThread( LPVOID lpCaller );
BOOL PASCAL InputDecryptPassword(const TCHAR* szTitle, int *retLength, unsigned char* retPassword, int maxLength);
DWORD WINAPI ThreadTest( LPVOID lpWnd );
FILE* hLogFile = NULL;
HANDLE hLogMutex = NULL;
DWORD WINAPI ThreadGingkoSyncEvent( LPVOID lpWnd );

static BOOL ShowDigitalSharingMap( LPCTSTR lpFileName )
{
	GingkoDigitalPack pack = {0};
	
	HANDLE hFile = CreateFile( lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile != INVALID_HANDLE_VALUE )
	{
		BOOL bRet = GingkoGetDigitalPack( hFile, NULL );

		if( bRet )
		{
			WaitForDetectGingkoServer();
			if( !::IsSessionValid( NULL ) )
			{
				CLoginDialog loginDialog;
				if( IDCANCEL == loginDialog.DoModal() )
				{
					CloseHandle( hFile );
					return FALSE;
				}
			}

			PostClientProcessId();

			bRet = GingkoGetDigitalPack( hFile, &pack );
			
		}
		
		CloseHandle( hFile );

		if( !bRet )
		{
			MessageBox( NULL, GetGlobalCaption( CAPTION_NOT_INCLUDE_DIGITALINFO, _T("Can not find the digital information.") ), MESSAGE_ERROR_TITLE, 0);
			return FALSE;
		}

		CGingkoAssociateDialog gad( &pack );
		gad.DoModal( GetDesktopWindow() );
	}
	
	return TRUE;
}

DWORD WINAPI StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, LPVOID pCaller)
{
	DWORD ThreadId = -1;
	HANDLE hThread = CreateThread( NULL,0, ThreadRoutine, pCaller, CREATE_SUSPENDED, &ThreadId );
	
	if( hThread != NULL )
	{
		ResumeThread( hThread );
		CloseHandle( hThread );
	}
	
	return ThreadId;
}

BOOL GetExecutablePath(CString &szFilepath)
{
	HANDLE hProcess = GetCurrentProcess();

	if( hProcess == NULL )
	{
		return FALSE;
	}

	TCHAR filePath[MAX_PATH];
	
	memset( filePath, 0, sizeof(filePath) );

	DWORD dwSize = GetModuleFileNameEx( hProcess, NULL, filePath, MAX_PATH );

	if( dwSize <= 0 )
	{
		return FALSE;
	}


	CString file( filePath );

	int idxSpash = file.ReverseFind( _T('\\') );
	
	if( idxSpash > 0 )
	{
		szFilepath.Format( _T("%s\\"), file.Mid( 0, idxSpash ) );
	}else{
		szFilepath.Append( file );
		szFilepath.Append( _T("\\") );
	}

	return TRUE;
}

BOOL GetRelatedPath( CString &szFile, CString& szFullPath )
{
	BOOL sz = GetExecutablePath( szFullPath );
	if( sz )
	{
		szFullPath.Append( szFile );
	}
	return sz;
}


int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	int nArgs = 0;
	int nRet = 0;
	ULONG dwProcessId = 0L;
	ULONG dwPermission = 0L;
	BOOL ShowLogin = FALSE;
	BOOL ShowOption = FALSE;
	BOOL ShowMain = TRUE;
	LPWSTR szEncryptFile = NULL;
	USES_CONVERSION;
	CString szCmdLine( lpstrCmdLine );
	CString szOpenedFile;

	LOG( L"EXECUTE WITH: %s\n", lpstrCmdLine );
	
	//szArglist = CommandLineToArgvW( lpstrCmdLine, &nArgs);
	//if( NULL == szArglist )
	//{
	//  wprintf(L"CommandLineToArgvW failed\n");
	//  return 0;
	//}
	szCmdLine = szCmdLine.Trim();

	if( szCmdLine.GetLength() > 2 )
	{
		

		CString cmd = szCmdLine.Left(2);
		if( cmd.CompareNoCase( CString(_T("/o") ) )== 0){
			///show options
			ShowOption = TRUE;
			ShowMain = FALSE;
		}else if( cmd.CompareNoCase(CString(_T("/l"))) == 0 ){
			///show login dialog
			if( IsSessionValid( NULL ) )
			{
				ShowLogin = FALSE;
				ShowMain = FALSE;
				PostClientProcessId();
			}else{
				ShowLogin = TRUE;
			}
		}else if( cmd.CompareNoCase(CString(_T("/s"))) == 0 ){
			///show login dialog
			szEncryptFile = _tcsdup( szCmdLine.Right( szCmdLine.GetLength() - 2 ).Trim() );
			ShowMain = TRUE; 
		}else if( cmd.CompareNoCase(CString(_T("/ul"))) == 0 ){
			///logout before
			logout();
			ShowMain = FALSE;
		}else if( cmd.CompareNoCase(CString(_T("/m"))) == 0 ){
			///logout before
			szEncryptFile = _tcsdup( szCmdLine.Right( szCmdLine.GetLength() - 2 ).Trim() );
			nRet = ShowDigitalSharingMap( szEncryptFile );
			ShowMain = FALSE;
		}else if( cmd.CompareNoCase(CString(_T("-p"))) == 0 )
		{
			int retLength = 0;
			unsigned char password[65] = {0}; 
			CString cmdLine = szCmdLine.Right( szCmdLine.Trim().GetLength() - 2 ).Trim();
			CString szFileHash = cmdLine.Left( 32 );
			
			cmdLine = cmdLine.Right( cmdLine.GetLength() - 32 ).Trim();

			CString szProcessId = cmdLine.Left( cmdLine.Find( _T(' ') ) );
			CString szFileName = cmdLine.Right( cmdLine.GetLength() - szProcessId.GetLength() );

			if( InputDecryptPassword( szFileName.Trim(), &retLength, password, 64) )
			{
				if( GingkoSendDecryptPassword((unsigned char*) T2A(szFileHash), password, (ULONG) _ttol( szProcessId ), retLength ) )
				{
					return 0xFF;
				}
			}

			return 0;
		}
	}

	//LocalFree(szArglist);

	if( ShowOption )
	{
		COptionsDialog OptionDlg;
		OptionDlg.DoModal();
	}

	if( ShowLogin )
	{
		CLoginDialog loginDlg;
		//loginDlg.ShowWindow(nCmdShow);
		 ShowMain = (loginDlg.DoModal() == IDOK );
		 
	}

	

	if( ShowMain )
	{
		HANDLE hInstMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, _GINGKO_INSTANCE_MUTEX );
		if( hInstMutex == NULL )
		{
			hInstMutex = CreateMutex( NULL, FALSE, _GINGKO_INSTANCE_MUTEX );
			if( hInstMutex == NULL )
			{
				SendMessage( HWND_BROADCAST, WM_WALKUP, (WPARAM) szEncryptFile, 0 );
				//CreateThread( NULL, 0, TestThread, szEncryptFile, 0, NULL );
			}
		}else
		{
			
			HGLOBAL hData = NULL;
			DWORD dwSize = 0;

			if( OpenClipboard( GetDesktopWindow() ) )
			{
				if( szEncryptFile != NULL )
				{
					dwSize = (DWORD) (sizeof(TCHAR) * (_tcslen( szEncryptFile ) + 1));
					EmptyClipboard();
					hData = GlobalAlloc( GPTR, dwSize );
					if( hData )
					{
						LPVOID lpData = GlobalLock( hData );
						if( lpData )
						{
							memcpy( lpData, szEncryptFile, dwSize );
						}
						GlobalUnlock( hData );
						SetClipboardData( CF_TEXT, hData );
					}
				}
				CloseClipboard();
			}

			SendMessage( HWND_BROADCAST, WM_WALKUP, (WPARAM) 0, (LPARAM) dwSize );
			
			Sleep( 2000 );
			
			if( hData != NULL )
			{
				GlobalFree( hData );
			}

			return 0;
		}

		_gSingleInstanceMutex = hInstMutex;

		_gClientSyncEvent = OpenEvent( EVENT_ALL_ACCESS, FALSE, _GINGKO_SYNC_EVENT );
		
		CMainDialog dlgMain;

		if(dlgMain.Create(NULL) == NULL)
		{
			ATLTRACE(_T("Main dialog creation failed!\n"));
			return 0;
		}
		
		//CWindow::GetWndClassName()
		if( szEncryptFile != NULL )
		{
			dlgMain.SetDigitalName( szEncryptFile, szEncryptFile );
		}

		dlgMain.ShowWindow(nCmdShow);

		CreateThread( NULL, 0, ThreadGingkoSyncEvent, dlgMain.m_hWnd, 0, NULL ); 

		nRet = theLoop.Run();

		_Module.RemoveMessageLoop();
	}


	return nRet;
}

void SetSoapConnectionOptions()
{
	USES_CONVERSION;
	bool _enableProxy = false;
	GingkoOptions *pOptions = NULL;
	GetGingkoOptions( &pOptions );
	
	if( pOptions != NULL )
	{
		addGingkoServiceUrl( T2A( pOptions->MasterServer ) );
		addGingkoServiceUrl( T2A( pOptions->UnitServer ));

		if( pOptions->Connection != NULL )
		{
			if( _tcscmp( pOptions->Connection, _T(DIRECT_CONNECTION) ) == 0 )
			{
				_enableProxy = false;
			}else if( _tcscmp( pOptions->Connection, _T(SAMEIE_CONNECTION) ) == 0 ){
				///Find the Proxy information by system call
				_enableProxy = true;
			}else if( _tcscmp( pOptions->Connection, _T(CUSTOM_CONNECTION) ) == 0 ){
				_enableProxy = true; 
			}

			int timeout = _ttoi( pOptions->MicsTimeout );
			SetConnectionOption( _enableProxy, T2A( pOptions->ProxyServer), T2A(pOptions->ProxyPort), 
						T2A( pOptions->ProxyUser ), T2A( pOptions->ProxyPassword ), timeout, timeout, 0 );

		}

		StartThread( DetectGingkoServerThread, NULL ); 
	}
}


DWORD WINAPI DetectGingkoServerThread( LPVOID lpCaller )
{
	return ::DetectGingkoServer();
}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	USES_CONVERSION;
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	if( hPrevInstance )
	{
		MessageBox( NULL, _T("Application was running."), _T("PrevInstance"), 0 );
	}

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	INITCOMMONCONTROLSEX CommonCtrlEx;
	CommonCtrlEx.dwICC = ICC_BAR_CLASSES | ICC_USEREX_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
	CommonCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	if( InitCommonControlsEx( &CommonCtrlEx ) )
	{
		hRes = S_OK;
	}

	LOG_START();

	AtlAxWinInit();

	InitDWMTheme();

	CString szConfigFile;
	CString szLanguage;
	GetRelatedPath( CString(GINGKO_CONFIG_FILE), szConfigFile ); 
	ParseConfigFile( szConfigFile );

	GetRelatedPath( CString(_T("Language")), szLanguage ); 

	GingkoOptions *pOptions = NULL;
	
	GetGingkoOptions( &pOptions );

	if( pOptions->Language == NULL )
	{
		szLanguage.Append( _T(".xml"));
	}else
	{
		szLanguage.AppendFormat( _T("_%s.xml"), pOptions->Language );
	}

	ParseLanguageFile( szLanguage );

	SetSoapConnectionOptions();

	hRes = _Module.Init(NULL, hInstance);

	ATLASSERT(SUCCEEDED(hRes));

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	int nRet = 0;

	nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	
	if( _gSingleInstanceMutex != NULL )
	{
		ReleaseMutex( _gSingleInstanceMutex );
		CloseHandle( _gSingleInstanceMutex );
	}

	FreeConfig();
	FreeDWMTheme();

	GdiplusShutdown(gdiplusToken);
	::CoUninitialize();

	LOG_SHUTDOWN();
	//GkoFreeLicense();

	return nRet;
}


BOOL PASCAL AcceptUserPassword(int *retLength, unsigned char* retPassword, int maxLength)
{
	USES_CONVERSION;
	CPasswordDialog PwdDlg;
	if( IDOK == PwdDlg.DoModal(_g_MainDialogHWND) )
	{
		int length = 0;
		const TCHAR* pwd = PwdDlg.GetPassword( &length );
		if( length >= maxLength )
		{
			MessageBox( NULL, GetGlobalCaption( CAPTION_PASSWORD_TOO_LONG, _T("You input password too long.")), MESSAGE_WARNING_TITLE, 0 );
			return FALSE;
		}

		char* pwdstr = W2A( pwd );
		memcpy( (retPassword), pwdstr, length );
		//strcpy_s( (char*)(*retPassword), length, pwdstr );
		if( retLength != NULL )
		{
			*retLength = length;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL PASCAL AcceptDecryptPassword(int *retLength, unsigned char* retPassword, int maxLength)
{
	USES_CONVERSION;
	CPasswordDialog PwdDlg(TRUE);
	if( IDOK == PwdDlg.DoModal(_g_MainDialogHWND) )
	{
		int length = 0;
		const TCHAR* pwd = PwdDlg.GetPassword( &length );
		if( length >= maxLength )
		{
			MessageBox( NULL, GetGlobalCaption( CAPTION_PASSWORD_TOO_LONG, _T("You input password too long.")), MESSAGE_WARNING_TITLE, 0 );
			return FALSE;
		}

		char* pwdstr = W2A( pwd );
		memcpy( (retPassword), pwdstr, length );
		//strcpy_s( (char*)(*retPassword), length, pwdstr );
		if( retLength != NULL )
		{
			*retLength = length;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL PASCAL InputDecryptPassword(const TCHAR* szTitle, int *retLength, unsigned char* retPassword, int maxLength)
{
	USES_CONVERSION;
	CPasswordDialog PwdDlg(TRUE);
	if( IDOK == PwdDlg.DoModal(_g_MainDialogHWND) )
	{
		int length = 0;
		const TCHAR* pwd = PwdDlg.GetPassword( &length );
		if( length >= maxLength )
		{
			MessageBox( NULL, GetGlobalCaption( CAPTION_PASSWORD_TOO_LONG, _T("You input password too long.")), MESSAGE_WARNING_TITLE, 0 );
			return FALSE;
		}

		char* pwdstr = W2A( pwd );
		memcpy( (retPassword), pwdstr, length );
		//strcpy_s( (char*)(*retPassword), length, pwdstr );
		if( retLength != NULL )
		{
			*retLength = length;
		}
		return TRUE;
	}
	return FALSE;
}

void ShowBalloonMessage(LPCTSTR lpszTitle, LPCTSTR lpszText, DWORD dwPermission)
{
	CBalloonHelp *Balloon = new CBalloonHelp(dwPermission);
	Balloon->SetIcon( IDI_INFORMATION );

	if( (dwPermission & 0x80000000) == 0x80000000)
	{
		HBITMAP hYesPassBitmap = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_PASS));
		HBITMAP hNoPassBitmap  = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_NO));
		Balloon->SetYesNoIcon( hYesPassBitmap, hNoPassBitmap );
	}

	DWORD dwOptions = CBalloonHelp::BOShowCloseButton  | CBalloonHelp::BOShowTopMost |
		CBalloonHelp::BOCloseOnButtonDown | CBalloonHelp::BOCloseOnButtonUp | 
		CBalloonHelp::BODeleteThisOnClose;
	
	CPoint ptAnchor(0,0);
	
	if( !(Balloon->Create( NULL, lpszTitle, lpszText, &ptAnchor, 
		dwOptions, NULL, 15000 ) ) )
	{
		delete Balloon;
	}
}


LRESULT NotifyEventMessage( HWND hWnd,GingkoNotifyMessage *lpNotify )
{
	DWORD dxSize = sizeof(GingkoNotifyMessage) + 50 * sizeof(TCHAR);
	TCHAR *pszMsg = (TCHAR*) GlobalAlloc( GPTR, dxSize );

	if( pszMsg != NULL )
	{
		memset( pszMsg, 0, dxSize );
		_stprintf_s( pszMsg, dxSize, 
			GetGlobalCaption( 60, _T("Application %s opened a share file (%s). The follow functions will be limited by this application:") ), 
			lpNotify->ProcessName, lpNotify->szFileName );

		ShowBalloonMessage( _T("Test"), pszMsg, lpNotify->dwPermission );
	}

	
	//LRESULT lr = ::SendNotifyMessage( hWnd, WM_NOTIFY_EVENT_MESSAGE, (WPARAM)(lpNotify->dwPermission), (LPARAM) pszMsg );
	
	return 0;
}


DWORD WINAPI ThreadGingkoSyncEvent( LPVOID lpWnd )
{
	HWND hWnd = (HWND) lpWnd;

	if( _gClientSyncEvent == NULL )
	{
		return 1;
	}

	__gGingkoSyncClientRunning = TRUE;

	while( __gGingkoSyncClientRunning )
	{
		if( WAIT_OBJECT_0 == WaitForSingleObject( _gClientSyncEvent, 2000 ) )
		{
			static MSG msg;			
			while( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
			{
				DispatchMessage( &msg );
			}

			SendMessage( hWnd, WM_NOTIFY_EVENT_MESSAGE, 0, 0 ); 
			ResetEvent( _gClientSyncEvent );
		}
	}
	return 0;
}


