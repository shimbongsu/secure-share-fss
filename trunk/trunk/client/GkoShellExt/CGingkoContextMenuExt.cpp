// CGingkoContextMenuExt.cpp : Implementation of CCGingkoContextMenuExt

#include "stdafx.h"
#include "GkoShellExt_i.h"
#include "CGingkoContextMenuExt.h"
#include <Psapi.h>
#include "GingkoLibrary.h"

typedef BOOL (WINAPI *FunIsSessionValid)( const GingkoUserPack **pack );
typedef BOOL (WINAPI *FunGingkoGetDigitalPack)( HANDLE hFile, GingkoDigitalPack *pack );

static HMODULE hGingkoLibrary = NULL;
static FunGingkoGetDigitalPack		__GingkoGetDigitalPack = NULL;
static FunIsSessionValid			__IsSessionValid = NULL;

static BOOL GetModulePath(CString &szFilepath)
{
	HINSTANCE hModule = _AtlModule.GetModuleInstance();

	if( hModule == NULL )
	{
		return FALSE;
	}

	HANDLE hProcess = GetCurrentProcess();

	if( hProcess == NULL )
	{
		return FALSE;
	}


	TCHAR filePath[MAX_PATH];
	
	memset( filePath, 0, sizeof(filePath) );

	DWORD dwSize = GetModuleFileNameEx( hProcess, hModule, filePath, MAX_PATH );

	if( dwSize <= 0 )
	{
		CloseHandle( hProcess );
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
	CloseHandle( hProcess );
	return TRUE;
}

static BOOL GetRelativePath( LPCTSTR lpszName, CString& szFile )
{
	GetModulePath( szFile );
	szFile.Append( lpszName );
	return TRUE;
}


BOOL InitGingkoLibrary()
{
	CString szFile;
	GetRelativePath( GINGKO_LIBRARY_NAME, szFile );

	hGingkoLibrary = LoadLibrary( szFile );
	
	if( hGingkoLibrary != NULL )
	{
		__IsSessionValid = (FunIsSessionValid)GetProcAddress( hGingkoLibrary, "IsSessionValid" );
		__GingkoGetDigitalPack = (FunGingkoGetDigitalPack)GetProcAddress( hGingkoLibrary, "GingkoGetDigitalPack" );
		return __IsSessionValid != NULL && __GingkoGetDigitalPack != NULL;
	}

	return FALSE;
}

BOOL FreeGingkoLibrary()
{

	if( hGingkoLibrary )
	{
		FreeLibrary( hGingkoLibrary );
	}
	return TRUE;
}

BOOL IsCurrentSessionValid()
{
	if( __IsSessionValid )
	{
		return __IsSessionValid( NULL );
	}
	return FALSE;
}

BOOL IsGingkoFile( CString& szFile )
{
	BOOL bRet = FALSE;
	HANDLE hFile = CreateFile( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( __GingkoGetDigitalPack && hFile != INVALID_HANDLE_VALUE )
	{
		bRet = __GingkoGetDigitalPack( hFile, NULL );
	}
	if( INVALID_HANDLE_VALUE != hFile )
	{
		CloseHandle( hFile );
	}
	return bRet;
}
// CCGingkoContextMenuExt

/////////////////////////////////////////////////////////////////////////////
// CRenameCtxMenuExt
STDMETHODIMP CCGingkoContextMenuExt::Initialize ( LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID )
{
	FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg = { TYMED_HGLOBAL };
	HDROP hDrop;
	HRESULT hr = S_OK;

	try {
		// Look for CF_HDROP data in the data object.
		if ( FAILED( pDataObj->GetData ( &fmt, &stg ))) return E_INVALIDARG;

		// Get a pointer to the actual data.// Make sure it worked.
		hDrop = (HDROP) GlobalLock ( stg.hGlobal );
		if ( NULL == hDrop )
			return E_INVALIDARG;

		// check number of selected files (0xFFFFFFFF flag)
		int fcount = DragQueryFile ( hDrop, 0xFFFFFFFF, NULL, 0 );
		if ( fcount == 0 )
		{ 
			GlobalUnlock ( stg.hGlobal ); 
			ReleaseStgMedium ( &stg ); 
			return S_OK;
		}else
		{
			CString szFileBuff;
			UINT cchBuff = 1024;
			LPTSTR pszBuff = NULL;
			do
			{
				pszBuff = szFileBuff.GetBuffer( cchBuff );
				if( DragQueryFile( hDrop, 0, pszBuff, cchBuff ) > 0 )
				{
					break;
				}
				szFileBuff.ReleaseBuffer();
				cchBuff += 1024;
			}while( cchBuff < 65535 );
			szFileBuff.ReleaseBuffer();
			m_szSelectedFilename.Format( szFileBuff );

		}

	}
	catch (...)
	{
		MSGBOX0("exception raised!");
		hr=E_INVALIDARG;
	}
	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );
	return hr;
}
//
STDMETHODIMP CCGingkoContextMenuExt::QueryContextMenu (HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd, UINT uidLastCmd, UINT uFlags)
{
	if ( uFlags & CMF_DEFAULTONLY ) return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );
	ATL::CString szGingkoShare( MAKEINTRESOURCE(IDS_MENU_SHARE) );
	ATL::CString szGingkoMore(_T("Advance Options"));
	ATL::CString szGingkoMap(MAKEINTRESOURCE(IDS_MENU_MAP));
	

	ATL::CString szSetting(_T("Settings"));
	ATL::CString szLogin(_T("Login"));
	ATL::CString szLogout(_T("Logout"));
	UINT uCmdID = uidFirstCmd;
	try {
		BOOL bCurrentSessionValid = IsCurrentSessionValid();

		InsertMenu ( hmenu, uMenuIndex, MF_SEPARATOR | MF_BYPOSITION, uCmdID ++, NULL );
		InsertMenu ( hmenu, uMenuIndex + 1, MF_STRING | MF_BYPOSITION, uCmdID ++, szGingkoShare );
		InsertMenu ( hmenu, uMenuIndex + 2, MF_STRING | MF_BYPOSITION, uCmdID ++, szGingkoMap );
		InsertMenu ( hmenu, uMenuIndex + 3, MF_SEPARATOR | MF_BYPOSITION, uCmdID ++, NULL );
		
		SetMenuItemBitmaps( hmenu, uMenuIndex + 1, MF_BYPOSITION, m_hBitmap, NULL );

		//SetMenuItemBitmaps( hmenu, uMenuIndex + 2, MF_BYPOSITION, m_hBitmap, NULL );
		
		if( !IsGingkoFile( m_szSelectedFilename ) )
		{
			EnableMenuItem( hmenu, uMenuIndex + 2, MF_BYPOSITION |  MF_GRAYED | MF_DISABLED);
		}

		//HMENU hSubMenu = ::CreateMenu();		
		//MENUITEMINFO mf = {0};
		//mf.cbSize = sizeof(MENUITEMINFO);
		//mf.hSubMenu = hSubMenu;
		//mf.fMask = MIIM_STRING | MIIM_SUBMENU;
		//mf.dwTypeData = _tcsdup( szGingkoMore );
		//mf.wID = uCmdID ++;
		//InsertMenuItem( hmenu, uMenuIndex, TRUE, &mf );
		//InsertMenu ( hmenu, uMenuIndex, MF_STRING | MF_BYPOSITION, uCmdID++, szGingkoShare );		
		//InsertMenu ( hSubMenu, 0, MF_STRING | MF_BYPOSITION, uCmdID++, szGingkoShare );
		//InsertMenu ( hSubMenu, 1, MF_STRING | MF_BYPOSITION, uCmdID++, szGingkoMap );

		//if( !IsGingkoFile( m_szSelectedFilename ) || ! bCurrentSessionValid )
		//{
		//	EnableMenuItem( hSubMenu, 1, MF_BYPOSITION |  MF_GRAYED | MF_DISABLED);
		//}

		//InsertMenu ( hSubMenu, 2, MF_SEPARATOR | MF_BYPOSITION, uCmdID++, NULL );
		//InsertMenu ( hSubMenu, 3, MF_STRING | MF_BYPOSITION, uCmdID++, szLogin );
		//InsertMenu ( hSubMenu, 4, MF_STRING | MF_BYPOSITION, uCmdID++, szLogout );
		//
		//if( bCurrentSessionValid )
		//{
		//	EnableMenuItem( hSubMenu, 3, MF_BYPOSITION |  MF_GRAYED | MF_DISABLED);
		//}else
		//{
		//	EnableMenuItem( hSubMenu, 4, MF_BYPOSITION |  MF_GRAYED | MF_DISABLED);
		//}

		//InsertMenu ( hSubMenu, 5, MF_STRING | MF_BYPOSITION, uCmdID++, szSetting );

		
	}
	catch (...)
	{
		MSGBOX0("exception in QueryContextMenu");
		return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );
	}
	return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, uCmdID - uidFirstCmd );//1 entry added
}
//
STDMETHODIMP CCGingkoContextMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
	USES_CONVERSION;

	// Check idCmd, it must be 0 or 1 since we have two menu items.
	if ( idCmd > 8 ) return E_INVALIDARG;

    //If Explorer is asking for a help string, copy our string into the supplied buffer
    if ( uFlags & GCS_HELPTEXT )
	{

		CString pszLoginText(_T("Login to Gingko Security System"));
		CString pszLogoutText(_T("Logout current user from Gingko Security System"));
		CString pszSharingText(MAKEINTRESOURCE(IDS_DESC_SHARE));
		CString pszMapText(MAKEINTRESOURCE(IDS_DESC_MAP));
		CString pszOptionText(_T("Gingko Security Client Settings."));
		CString *pszText = NULL;
		switch(idCmd)
		{
		case 1:
			pszText = &pszSharingText;
			break;
		case 2:  ///Gingko Sharing
			pszText = &pszMapText;
			break;
		//case 4:	 ///Gingko Map
		//	pszText = &pszMapText;
		//	break;
		//case 6:
		//	pszText = &pszLoginText;
		//	break;
		//case 7:
		//	pszText = &pszLogoutText;
		//	break;
		//case 8:
		//	pszText = &pszOptionText;
		//	break;
		default:
			pszText = NULL;
		}

		if( pszText != NULL )
		{
			if (uFlags & GCS_UNICODE) 
			{
				lstrcpynW ( (LPWSTR) pszName, T2CW(*pszText), cchMax);
			}
			else 
			{
				lstrcpynA ( pszName, T2CA(*pszText), cchMax );
			}
		}

		return S_OK;
	}
    return E_INVALIDARG;
}

STDMETHODIMP CCGingkoContextMenuExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo )
{
    // If lpVerb really points to a string, ignore this function call and bail out.
	USES_CONVERSION;
    if ( 0 != HIWORD( pCmdInfo->lpVerb )) return E_INVALIDARG;
	try {
		// Check that lpVerb is our command (index 0)
		UINT uCmdId = LOWORD( pCmdInfo->lpVerb );
		CString szCmdText;
		CString szExeFile;
		GetRelativePath( _T("GingkoFish.exe"), szExeFile );

		switch( uCmdId )
		{
		case 1:
			szCmdText.Append(_T(" /s "));
			szCmdText.Append( m_szSelectedFilename );
			break;
		case 2:
			//szCmdText.Append(_T(" /s "));
			//szCmdText.Append( m_szSelectedFilename );
			//break;
			szCmdText.Append(_T(" /m "));
			szCmdText.Append( m_szSelectedFilename );
			break;

		//case 3:  ///Gingko Sharing
		//	szCmdText.Append(_T(" /s "));
		//	szCmdText.Append( m_szSelectedFilename );
		//	break;
		//case 4:	 ///Gingko Map
		//	szCmdText.Append(_T(" /m "));
		//	szCmdText.Append( m_szSelectedFilename );
		//	break;
		//case 6:
		//	szCmdText.Append(_T(" /l "));
		//	break;
		//case 7:
		//	szCmdText.Append(_T(" /ul "));
		//	break;
		//case 8:
		//	szCmdText.Append(_T(" /o "));
		//	break;
		default:
			return E_INVALIDARG;
		}
		
		ShellExecute( NULL, NULL, szExeFile, szCmdText, A2T(pCmdInfo->lpDirectory), SW_SHOW );
		
		return S_OK;
	}
	catch(...) 
	{
		return E_INVALIDARG;
	}

	
	return S_OK;
}
//