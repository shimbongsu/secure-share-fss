#include "StdAfx.h"
#include "MainDialog.h"
#include <atlpath.h>

BOOL PASCAL EncrypFileProgress(LPVOID __pack, int percent, const TCHAR* pszMsg)
{
	GingkoDigitalPack *pack = static_cast<GingkoDigitalPack*> (__pack);
	CMainDialog *pMainDlg =  static_cast<CMainDialog*>( pack->CallObject );
	return pMainDlg->SetProgress( percent );
}

BOOL GetProcessExeFile(DWORD dwProcessId, TCHAR* szFileName, INT Length )
{
	BOOL bret = FALSE;
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );
	if( hProcess == NULL )
	{
		DWORD dwError = GetLastError();
		LOG(L"To get the process exe file with error: %d.\n", dwError );
		return bret;
	}

	bret = GetModuleFileNameEx( hProcess, NULL, szFileName, Length ) > 0;
	CloseHandle( hProcess );
	return bret;
}

BOOL BalloonDisplayEventMessage( LPVOID Parent, GingkoNotifyMessage* pMessage )
{

	return TRUE;
}

BOOL FormatPermission(TCHAR *szPermission, DWORD MaxSize, ULONG dwPerm )
{
	const TCHAR *szFmt = GetGlobalCaption( 61, _T("%s\t%s\t%s") );
	_stprintf_s( szPermission, MaxSize, szFmt, 
		(dwPerm & 0x800F0000) == 0x800F0000 ? _T("Readable") : _T("Cannot Read"), 
		(dwPerm & 0x80000F00) == 0x80000F00 ? _T("Printable") : _T("Cannot print"),
		(dwPerm & 0x8000F000) == 0x8000F000 ? _T("Transferable") : _T("Cannot transfer"));
	return TRUE;
}

DWORD FormatNotifyMessage(GingkoNotifyMessage* pNotify, TCHAR* lpszMessage, DWORD MaxSize)
{
	const TCHAR *szFmt = GetGlobalCaption( 60, _T("%s opened the Sharing file(%s). Which is allowed as follow permission by the owner.\n%s") );
	TCHAR szFmtPerm[ MAX_PATH + 1] = {0};
	TCHAR szFmtExefile[ MAX_PATH + 1] = {0};
	GetProcessExeFile( pNotify->dwProcessId, szFmtExefile, MAX_PATH );
	FormatPermission( szFmtPerm, MAX_PATH, pNotify->dwPermission );
	_stprintf_s( lpszMessage, MaxSize, szFmt, szFmtExefile, pNotify->szFileName, szFmtPerm );
	return MaxSize;
}
