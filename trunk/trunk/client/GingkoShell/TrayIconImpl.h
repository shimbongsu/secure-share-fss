#ifndef __TRAYICONIMPL_H__
#define __TRAYICONIMPL_H__

#include <atlmisc.h>
#include "BalloonHelp.h"
// Wrapper class for the Win32 NOTIFYICONDATA structure
class CNotifyIconData : public NOTIFYICONDATA
{
public:	
	CNotifyIconData()
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
};

// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.
template <class T>
class CTrayIconImpl
{	
private:
	UINT WM_TRAYICON;
	DLLGETVERSIONPROC _DllGetVersion;
	CNotifyIconData m_nid;
	bool m_bInstalled;
	LONG ShellVersion;
	UINT m_nDefault;
	UINT m_msgTaskbarRestart;
	UINT m_ScreenWidth;
	UINT m_ScreenHeight;
	HANDLE m_TrayNotifyEvent;
	BOOL m_BalloonShown;
	HBITMAP hYesPassBitmap;
	HBITMAP hNoPassBitmap;
	CBalloonHelp* m_pBalloon;
protected:
	HWND m_hWndTooltip;

public:	
	CTrayIconImpl() : m_bInstalled(false), m_nDefault(0)
	{
		WM_TRAYICON = ::RegisterWindowMessage(_T("WM_TRAYICON"));
		_DllGetVersion = (DLLGETVERSIONPROC) GetProcAddress( LoadLibrary( _T("shell32.dll") ), "DllGetVersion" );
		
		ShellVersion = GetShellVersion();

		if( ShellVersion >= 6 ){
			//m_nid.cbSize = sizeof(NOTIFYICONDATA);
			m_nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		}else if( ShellVersion == 5 ){
			m_nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		}else{
			m_nid.cbSize = NOTIFYICONDATA_V1_SIZE;
		}

		m_msgTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

		m_ScreenWidth = GetSystemMetrics(SM_CXSCREEN);   
		m_ScreenHeight = GetSystemMetrics(SM_CYSCREEN); 
		m_pBalloon = new CBalloonHelp();
		m_TrayNotifyEvent = CreateMutex( NULL, FALSE, NULL );
		m_BalloonShown = FALSE;

		hYesPassBitmap = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_PASS));
		hNoPassBitmap  = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISP_NO));

		//Init( _Module.GetModuleInstance(), _T("Application Title") );
	}

	LONG GetShellVersion()
	{
		DLLVERSIONINFO DllInfo = {0};
		
		DllInfo.cbSize = sizeof(DLLVERSIONINFO);

		if( _DllGetVersion == NULL )
		{
			return 0L;
		}

		if(  S_OK == _DllGetVersion( &DllInfo ) )
		{

			return DllInfo.dwMajorVersion;
		}

		return 0L;
	}
	
	~CTrayIconImpl()
	{
		// Remove the icon
		RemoveIcon();
		if( hYesPassBitmap )
		{
			DeleteObject( hYesPassBitmap );
		}
		if( hNoPassBitmap )
		{
			DeleteObject( hNoPassBitmap );
		}
	}


	static int BalloonHelpCallback( PVOID pSrc )
	{
		CTrayIconImpl *pImpl = (CTrayIconImpl*) pSrc;
		if( pImpl != NULL )
		{
			WaitForSingleObject( pImpl->m_TrayNotifyEvent, 20000 );
			//pImpl->m_pBalloon = NULL;
			//pImpl->m_BalloonShown = FALSE;
			ReleaseMutex( pImpl->m_TrayNotifyEvent );
		}
		return 0;
	}

	// Install a taskbar icon
	// 	lpszToolTip 	- The tooltip to display
	//	hIcon 		- The icon to display
	// 	nID		- The resource ID of the context menu
	/// returns true on success
	bool InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID)
	{
		T* pT = static_cast<T*>(this);

		//Init( NULL, lpszToolTip );

		// Fill in the data		
		m_nid.hWnd = pT->m_hWnd;
		m_nid.uID = nID;
		m_nid.hIcon = hIcon;
		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; // | NIF_INFO;
		m_nid.uCallbackMessage = WM_TRAYICON;
		
		_tcscpy_s(m_nid.szTip, 128, lpszToolTip);
		_tcscpy_s(m_nid.szInfoTitle, 64, lpszToolTip);
		//_tcscpy_s(m_nid.szInfo, 256, _T("Application Started.") );
		// Install
		m_bInstalled = Shell_NotifyIcon(NIM_ADD, &m_nid) ? true : false;

		if( ShellVersion >= 5 )
		{
			m_nid.uVersion = NOTIFYICON_VERSION;
			Shell_NotifyIcon(NIM_SETVERSION, &m_nid) ? true : false;
		}

		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		// Done
		return m_bInstalled;
	}

	
	BOOL ModifyIcon( LPCTSTR lpszToolTip, BOOL bBalloon, ULONG dwPermission = 0)
	{
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		//m_nid.hWnd = pT->m_hWnd;
		//m_nid.uID = nID;

		if( bBalloon && ShellVersion >= 5 )
		{
			m_nid.uFlags = NIF_INFO;
			m_nid.dwInfoFlags = NIIF_INFO;
			//StringCchCopy( m_nid.szInfo, ARRAYSIZE(m_nid.szInfo), lpszToolTip );
			_tcscpy_s( m_nid.szInfo, sizeof( m_nid.szInfo ) / sizeof(TCHAR), lpszToolTip );
			m_nid.uTimeout = 15000;
		}else
		{
			return ShowCustomTips( m_nid.szInfoTitle, lpszToolTip, dwPermission );
			//_tcscpy_s(m_nid.szTip, 128, lpszToolTip);
		}

		//TrackTooltip();
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
	}

	BOOL ShowCustomTips(LPCTSTR lpszTitle, LPCTSTR lpszText, ULONG dwPermission)
	{
		CPoint ptAnchor( m_ScreenWidth - 20, m_ScreenHeight - 20 );
		T* pT = static_cast<T*>(this);
		WaitForSingleObject( m_TrayNotifyEvent, 20000 );
		//LOG( L"Thread Id for Show CustomTips: %d.\n", GetCurrentThreadId() );
		try{
			BOOL bret = FALSE;
			CBalloonHelp *Balloon = new CBalloonHelp();
			Balloon->SetIcon( IDI_INFORMATION );
			Balloon->m_Permission = dwPermission;

			if( (dwPermission & 0x80000000) == 0x80000000)
			{
				Balloon->SetYesNoIcon( hYesPassBitmap, hNoPassBitmap );
			}

			DWORD dwOptions = CBalloonHelp::BOShowCloseButton  | CBalloonHelp::BOShowTopMost |
				CBalloonHelp::BOCloseOnButtonDown | CBalloonHelp::BOCloseOnButtonUp;

			bret = Balloon->Create( NULL, lpszTitle, lpszText, &ptAnchor, 
				dwOptions, NULL, 15000 );
			
			if( bret != TRUE )
			{
				//delete Balloon;
			}
		}catch(DWORD dwExceptCode)
		{
			LOG( L"Exception with %d.\n", dwExceptCode);
		}

		ReleaseMutex( m_TrayNotifyEvent);
		
		//if( m_pBalloon != NULL )
		//{
		//	if( m_BalloonShown == FALSE )
		//	{
		//		//m_pBalloon = new CBalloonHelp( dwPermission );
		//		m_pBalloon->SetCloseCallback( (CloseWindowCallback)BalloonHelpCallback, this );
		//		m_pBalloon->SetIcon( IDI_INFORMATION );
		//		m_pBalloon->m_Permission = dwPermission;
		//		DWORD dwOptions = CBalloonHelp::BOShowCloseButton  | CBalloonHelp::BOShowTopMost |
		//			CBalloonHelp::BOCloseOnButtonDown | CBalloonHelp::BOCloseOnButtonUp;

		//		BOOL bRet=m_pBalloon->Create( NULL, lpszTitle, lpszText, &ptAnchor, 
		//			dwOptions, NULL, 15000 );

		//		if( bRet )
		//		{
		//			m_BalloonShown = TRUE;
		//		}
		//		
		//	}else
		//	{
		//		m_pBalloon->SetTitle( lpszTitle );
		//		m_pBalloon->SetContent( lpszText );
		//		m_pBalloon->SetTimeout( 15000 );
		//		m_pBalloon->m_Permission = dwPermission;
		//	}
		//}
		
		return TRUE;
	}

	// Remove taskbar icon
	// returns true on success
	bool RemoveIcon()
	{
		if (!m_bInstalled)
			return false;
		// Remove
		m_nid.uFlags = 0;
		return Shell_NotifyIcon(NIM_DELETE, &m_nid) ? true : false;
	}

	// Set the icon tooltip text
	// returns true on success
	bool SetTooltipText(LPCTSTR pszTooltipText)
	{
		if (pszTooltipText == NULL)
			return FALSE;
		// Fill the structure
		m_nid.uFlags = NIF_TIP;
		_tcscpy(m_nid.szTip, pszTooltipText);
		// Set
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
	}

	// Set the default menu item ID
	inline void SetDefaultItem(UINT nID) { m_nDefault = nID; }

	BEGIN_MSG_MAP(CTrayIcon)
		MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
		MESSAGE_HANDLER(m_msgTaskbarRestart, OnRestart)
	END_MSG_MAP()

	LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Is this the ID we want?
		if (wParam != m_nid.uID)
			return 0;
		T* pT = static_cast<T*>(this);
		// Was the right-button clicked?
		if (LOWORD(lParam) == WM_RBUTTONUP)
		{
			// Load the menu
			CMenu oMenu;
			if (!oMenu.LoadMenu(m_nid.uID))
				return 0;
			// Get the sub-menu
			CMenuHandle oPopup(oMenu.GetSubMenu(0));
			// Prepare
			pT->PrepareMenu(oPopup);
			// Get the menu position
			CPoint pos;
			GetCursorPos(&pos);
			// Make app the foreground
			SetForegroundWindow(pT->m_hWnd);
			// Set the default menu item
			if (m_nDefault == 0)
				oPopup.SetMenuDefaultItem(0, TRUE);
			else
				oPopup.SetMenuDefaultItem(m_nDefault);
			// Track
			oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);
			// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
			pT->PostMessage(WM_NULL);
			// Done
			oMenu.DestroyMenu();
		}
		else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			// Make app the foreground
			SetForegroundWindow(pT->m_hWnd);
			// Load the menu
			CMenu oMenu;
			if (!oMenu.LoadMenu(m_nid.uID))
				return 0;
			// Get the sub-menu
			CMenuHandle oPopup(oMenu.GetSubMenu(0));			
			// Get the item
			if (m_nDefault)
			{
				// Send
				pT->SendMessage(WM_COMMAND, m_nDefault, 0);
			}
			else
			{
				UINT nItem = oPopup.GetMenuItemID(0);
				// Send
				pT->SendMessage(WM_COMMAND, nItem, 0);
			}
			// Done
			oMenu.DestroyMenu();
		}
		return 0;
	}

	LRESULT OnRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		RemoveIcon();
		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIM_SETVERSION;
		m_nid.uVersion = ShellVersion;
		Shell_NotifyIcon(NIM_ADD, &m_nid);

		return 0;
	}

	// Allow the menu items to be enabled/checked/etc.
	virtual void PrepareMenu(HMENU hMenu)
	{
		// Stub
	}
};

#endif
