#include "stdafx.h"
#include <windows.h> 
#include "GingkoServiceDispatcher.h"
#include "GingkoLibrary.h"
#include <strsafe.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <shellapi.h>
#include <userenv.h>
#include <winerror.h>
#include <sqlite3.h>
#include "cheapman.h"

#define _GINGKO_SYNC_EVENT  _T("Global\\SyncEvent-{6909DB0C-2564-4431-B18A-B46ECC1E01A8}")

sqlite3 *g_gingkodb = NULL; //Database connect

//CAppModule  _Module;
extern HANDLE				__gClientSyncEventHandle;
extern HANDLE				_gNotifyMessageMutex;
extern DWORD __gClientProcessId;
BOOL	  __gApplicationExit = FALSE;

static TCHAR* __gExe32Path = NULL;
static TCHAR* __gExe64Path = NULL;
static TCHAR* __gGingkoPath = NULL;

GINGKO_PROGRAM_PASS*  _ProgramPassFirst = NULL;

GingkoPasswordTable __gPasswordTable = {0};
HANDLE __gPasswordEventHandle = NULL;
DWORD ExecuteProcessAsSession(const TCHAR* szExePath, TCHAR* szCommandLine, DWORD dwSessionId );
LPWSTR GetModulePath(HMODULE hModule /* = NULL */);

BOOL LoadProgramPassThroughtTable()
{
	return TRUE;
}

BOOL FreeProgramPassThroghtTable()
{
	return TRUE;
}

BOOL ProgramIsPassthrought(const TCHAR* szFileName)
{
	//INT cp = 0;
	GINGKO_PROGRAM_PASS* pPass = _ProgramPassFirst;
	TCHAR szPath[MAX_PATH + 1];
	memset(szPath, 0, sizeof(szPath));
	//::_tccpy( szPath, szFileName );
	
	_tcscpy_s( szPath, MAX_PATH, szFileName );

	while( pPass != NULL ){
		if( _tcsnicmp(szFileName, pPass->szPathName, MAX_PATH) == 0 ){
			return TRUE;
		}
		pPass = pPass->Next;
	}
	return FALSE;
}

BOOL IsProcessWow32( HANDLE hProcess )
{
	BOOL IsWow32 = TRUE;
	if( IsWow64Process != NULL )
	{
		IsWow64Process( hProcess, &IsWow32 );
	}
	return IsWow32;
}

LPCTSTR GetFishshellPath(BOOL bX32)
{
	if( __gExe32Path == NULL )
	{
		LPTSTR pModulePath = GetModulePath( NULL );
		if( pModulePath != NULL )
		{
			SIZE_T szPathLength = sizeof(TCHAR) * (_tcslen( pModulePath ) + 20);
			__gExe32Path = (TCHAR*) malloc( szPathLength );
			__gExe64Path = (TCHAR*) malloc( szPathLength );
			__gGingkoPath = (TCHAR*) malloc( szPathLength );
			if( __gExe32Path )
			{
				memset( __gExe32Path, 0, szPathLength );
				_stprintf_s( __gExe32Path, szPathLength / sizeof(TCHAR) - 1, _T("%s\\Fishshell32.exe"), pModulePath );
			}
			if( __gExe64Path )
			{
				memset( __gExe64Path, 0, szPathLength );
				_stprintf_s( __gExe64Path, szPathLength / sizeof(TCHAR) - 1, _T("%s\\Fishshell64.exe"), pModulePath );
			}
			if( __gGingkoPath )
			{
				memset( __gGingkoPath, 0, szPathLength );
				_stprintf_s( __gGingkoPath, szPathLength / sizeof(TCHAR) - 1, _T("%s\\GingkoFish.exe"), pModulePath );
			}
			free( pModulePath );
		}
	}

	if( bX32 == TRUE )
	{
		return __gExe32Path;
	}else {
		return __gExe64Path;
	}

	return __gGingkoPath;
}

LPWSTR GetModulePath(HMODULE hModule /* = NULL */)
{
	TCHAR buf[MAX_PATH] = {'\0'};
	int i = 0;
	GetModuleFileName( hModule, buf, MAX_PATH);
	LOG(L"MODULE: %s\n", buf );
	for( i = MAX_PATH; i > 0; i -- )
	{
		if( buf[i] == L'\\' )
		{
			break;
		}
	}

	LOG(L"Mod %d\n", i );

	if( i > 0 )
	{
		LOG(L"MODULE: ...\n");
		TCHAR SecondBuf[MAX_PATH] = {'\0'};

		memset( SecondBuf, 0, sizeof(TCHAR) * MAX_PATH );
		memcpy( SecondBuf, buf, sizeof(TCHAR) * i );

		LOG(L"MODULE: %s \n", SecondBuf );
		return _wcsdup(SecondBuf);
	}
	return NULL;
}

BOOL GetProcessExeFile(DWORD dwProcessId, TCHAR* szFileName, INT Length )
{
	BOOL bret = FALSE;
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );
	if( hProcess != NULL )
	{
		bret = GetModuleFileNameEx( hProcess, NULL, szFileName, Length ) > 0;
	}
	CloseHandle( hProcess );
	return bret;
}

BOOL GetProcessBaseName(DWORD dwProcessId, TCHAR* szFileName, INT Length )
{
	BOOL bret = FALSE;
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );
	if( hProcess != NULL )
	{
		bret = GetModuleBaseName( hProcess, NULL, szFileName, Length ) > 0;
	}
	CloseHandle( hProcess );
	return bret;
}


int OpenDatabase(BOOL Create)
{
	int ret = sqlite3_open( "gko_authenticdb", &g_gingkodb );
	
	if( ret != SQLITE_OK )
	{
		const char *err = sqlite3_errmsg( g_gingkodb );
		printf( "Can not open the database gko_authenticdb. The error message is %s.\n", err );
		sqlite3_close( g_gingkodb );
	}

	if( Create )
	{
		sqlite3_exec( g_gingkodb, "CREATE TABLE authentic_application(exe_path VARCHAR(2000), md5_hash VARCHAR(32), file_ext VARCHAR(100), allowed INT);", NULL, NULL, NULL );	
	}

	return ret;
}

int CloseDatabase()
{
	int ret = SQLITE_OK;
	if( g_gingkodb != NULL )
	{
		ret = sqlite3_close( g_gingkodb );
		g_gingkodb = NULL;
	}
	return ret;
}

void FreeText( void* pText )
{
	if( pText )
	{
		free( pText );
	}
}

BOOL QueryAuthenticApplication( const unsigned char* szMd5, const TCHAR* szOpenFileName )
{
	USES_CONVERSION;
	BOOL retValue = FALSE;
	sqlite3_stmt *QueryAuthStmt = NULL; //The primary key query statement
	int ret = sqlite3_prepare_v2( g_gingkodb, "SELECT exe_path, md5_hash, file_ext, allowed FROM authentic_application WHERE md5_hash=$md5hash AND file_ext=$fileExt", -1, &QueryAuthStmt, NULL );

	if( ret == SQLITE_OK )
	{
		//sqlite3_reset( QueryAuthStmt );
		//sqlite3_clear_bindings( QueryAuthStmt );
		sqlite3_bind_text( QueryAuthStmt, 1, _strdup((const char*)szMd5), -1, FreeText ); //"company id"
		sqlite3_bind_text( QueryAuthStmt, 2, _strdup( T2A(szOpenFileName)), -1, FreeText ); //"company id"

		while(  SQLITE_ROW == sqlite3_step( QueryAuthStmt) )
		{
			if( sqlite3_column_int( QueryAuthStmt, 3 ) != 0 )
			{
				retValue = TRUE;
				break;
			}
		}
		sqlite3_finalize( QueryAuthStmt );
	}

	return retValue;

}

BOOL AuthenticProcess( DWORD dwProcessId, TCHAR* szOpenFileName )
{
	TCHAR* szExeFile = (TCHAR*) malloc( sizeof(TCHAR) * (MAX_PATH + 1) );
	DWORD dwLength = MAX_PATH;
	BOOL  hasError = FALSE;
	BOOL  bret = FALSE;
	if( dwProcessId == __gClientProcessId )
	{
		return FALSE;
	}

	if( IsProcessUnderService( dwProcessId ) )
	{
		return FALSE;
	}

	if( szExeFile != NULL )
	{
		memset( szExeFile, 0, sizeof(TCHAR) + dwLength );
		while( !GetProcessExeFile( dwProcessId, szExeFile, dwLength ) )
		{
			if( dwLength > MAX_PATH * 20 )
			{
				hasError = TRUE;
				break;
			}

			free( szExeFile );
			dwLength += MAX_PATH;
			szExeFile = (TCHAR*) malloc( sizeof(TCHAR) * (dwLength + 1) );
			if( szExeFile == NULL )
			{
				hasError = TRUE;
				break;
			}
			memset( szExeFile, 0, sizeof(TCHAR) + dwLength );
		}

		if ( ProgramIsPassthrought( szExeFile ) ){
			free( szExeFile );
			return TRUE;
		}

		/// IF THE APPLICATION WAS SIGNATURE BY A VALID CERTIFICATION,
		/// WE  WILL TRUST IT.
		if( VerifyEmbeddedSignature( szExeFile ) == TRUE )
		{
			free( szExeFile );
			return TRUE;
		}

		if( !hasError )
		{
			unsigned char szMd5[33] = {0};
			MD5FileObject( szExeFile, szMd5, 32 ); 
			bret = QueryAuthenticApplication( szMd5, szOpenFileName );
			if( bret == FALSE )
			{
				///NOT  EXIST IN THE DATABASE, SO ASK THE USER TO CONFIRM THAT THE APPLICATION WILL OPEN THE FILE.
				///SEND THE BROADCAST WINDOW MESSAGE TO START A WINDOW TO CONFIRM IT.
				///BUT  BY DEFAULT, THE APPLICATION WILL NOT ALLOWED TO OPEN THE FILE BEFORE IT IS ADDED TO THE DATABASE.
				free( szExeFile );
				return TRUE;
			}
		}

		if( szExeFile != NULL )
		{
			free( szExeFile );
		}

		bret = TRUE;

	}

	return bret;
}


BOOL GetDigitalPassword( const TCHAR *fileHashId, const TCHAR *szFileName, 
						unsigned char* lpvPassBuffer, int maxlength, int *retLength, ULONG dwProcessId )
{
	HDESK   hdeskCurrent;
	HDESK   hdesk;
	HWINSTA hwinstaCurrent;
	HWINSTA hwinsta;
	BOOL    bRet = FALSE;
	USES_CONVERSION;
	hwinstaCurrent = GetProcessWindowStation();
	unsigned char *pszFileHash =(unsigned char*) T2A( fileHashId );

	SetLastError( 0 );

	

	if( hwinstaCurrent == NULL )
	{
		return bRet;
	}

	hdeskCurrent = GetThreadDesktop( GetCurrentThreadId() );

	if ( hdeskCurrent == NULL )
	{
		//LOG(L"thread desk null\n");
		return bRet;
	}

	hwinsta = OpenWindowStation( L"winsta0", FALSE,                          
				WINSTA_ACCESSCLIPBOARD   |
				WINSTA_ACCESSGLOBALATOMS |
				WINSTA_CREATEDESKTOP     |
				WINSTA_ENUMDESKTOPS      |
				WINSTA_ENUMERATE         |
				WINSTA_EXITWINDOWS       |
				WINSTA_READATTRIBUTES    |
				WINSTA_READSCREEN        |
				WINSTA_WRITEATTRIBUTES);

	if (hwinsta == NULL){
		LOG(L"open winsta faild\n");
		return bRet;
	}

	if (!SetProcessWindowStation(hwinsta)){
		LOG(L"set process winstation faild\n");
		return bRet;
	}

	//´ò¿ªdesktop
	hdesk = OpenDesktop(L"default", DF_ALLOWOTHERACCOUNTHOOK, FALSE,                
				DESKTOP_CREATEMENU |
				DESKTOP_CREATEWINDOW |
				DESKTOP_ENUMERATE    |
				DESKTOP_HOOKCONTROL  |
				DESKTOP_JOURNALPLAYBACK |
				DESKTOP_JOURNALRECORD |
				DESKTOP_READOBJECTS |
				DESKTOP_SWITCHDESKTOP |
				DESKTOP_WRITEOBJECTS);

	if (hdesk == NULL){
		LOG(L"desk.\n");
		return bRet;
	}
	SetThreadDesktop(hdesk);  
	///Ready to show this Window
	HANDLE hTokenThis = NULL; 
	HANDLE hTokenDup = NULL; 
	DWORD dwSessionId = 0;
	DWORD dwCurrentSessionId = 0;
	DWORD dwReturnLength = 0;
	HANDLE hThisProcess = GetCurrentProcess(); 

	OpenProcessToken(hThisProcess, TOKEN_ALL_ACCESS, &hTokenThis); 
	
	if( FALSE == DuplicateTokenEx(hTokenThis, MAXIMUM_ALLOWED,NULL, SecurityIdentification, TokenPrimary, &hTokenDup) )
	{
		LOG(L"Error to duplicatetoken.\n");
	}
	//
	dwSessionId = WTSGetActiveConsoleSessionId(); 
	
	SetLastError(0); 

	if( FALSE == SetTokenInformation( hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)) )
	{
		LOG(L"ERROR TO SET THE TokenSessionId to current process. Error Code: %d\n", GetLastError() );
	}

	if( FindGingkoPassword( pszFileHash, dwProcessId, NULL, NULL, FALSE ) == FALSE )
	{
		InsertGingkoPassword( pszFileHash, NULL, 0, dwProcessId ); //Just make a placeholder

		TCHAR ExecutePath[MAX_PATH + sizeof(TCHAR)] = {'\0'};
		TCHAR *CommandLine = (TCHAR*) malloc( sizeof(TCHAR) * 1024 * 8 ); //8K
		if( CommandLine != NULL )
		{
			memset( CommandLine, 0, sizeof(TCHAR) * 1024 * 8 );
			_sntprintf_s( CommandLine, 1024 * 8, sizeof(TCHAR) * 1024 * 8, _T("  -p %s %d %s"), fileHashId, dwProcessId, szFileName );
		}

		const TCHAR* ExePath = GetExecutablePath();
		if( ExePath != NULL )
		{
			_snwprintf_s( ExecutePath, MAX_PATH, L"%s\\%s", ExePath, _T("GingkoFish.exe") );

			STARTUPINFO si; 
			PROCESS_INFORMATION pi; 
			ZeroMemory(&si, sizeof(STARTUPINFO)); 
			ZeroMemory(&pi, sizeof(PROCESS_INFORMATION)); 
			si.cb = sizeof(STARTUPINFO); 
			si.lpDesktop = L"WinSta0\\Default"; 
			
			LPVOID pEnv = NULL;

			DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT; 

			//if( FALSE == CreateEnvironmentBlock(&pEnv, hTokenDup, FALSE) )
			//{
			//	LOG(L"ERROR TO CreateEnvironmentBlock\n");
			//}

			//WCHAR GPF[MAX_PATH] = {'\0'};
			//_snwprintf( GPF, MAX_PATH, L"GingkoConsole" );

			LOG(L"BEGIN CreateProcessAsUser: %p.\n", hTokenDup );
			if( FALSE == CreateProcessAsUser( 
						  hTokenDup, 
						  ExecutePath, 
						  CommandLine, 
						  NULL, 
						  NULL, 
						  FALSE, 
						  dwCreationFlag, 
						  NULL, 
						  ExePath, 
						  &si, 
						  &pi) )
			{
				DWORD ErrorCode = GetLastError();
				LOG(L"ERROR TO CREATE PROCESS AS OTHER USER. %d\n", ErrorCode );
			}

			//CloseHandle( pi.hProcess );
			//CloseHandle( pi.hThread );

			//CloseHandle( hTokenDup );
			//ShellExecute(0,NULL, ExecutePath, NULL, NULL, SW_SHOWNORMAL);

			DWORD ExitCode = 0;
			if( pi.hProcess != NULL )
			{
				WaitForSingleObject( pi.hProcess, INFINITE );
				GetExitCodeProcess( pi.hProcess, &ExitCode );
			}


			if( ExitCode == 0xFF )
			{
				////Actually, it's our code.
				while( (bRet = FindGingkoPassword( pszFileHash, dwProcessId, lpvPassBuffer, retLength, TRUE )) == FALSE )
				{
					///Ready to return.
					WaitForSingleObject( __gPasswordEventHandle, 1000 );
					
					if( __gApplicationExit )
					{
						break;
					}
				}
			}else
			{
				bRet = FALSE;
			}
		}else{

			while( (bRet = FindGingkoPassword( pszFileHash, dwProcessId, lpvPassBuffer, retLength, TRUE )) == FALSE )
			{
				///Ready to return.
				WaitForSingleObject( __gPasswordEventHandle, 1000 );
				
				if( __gApplicationExit )
				{
					break;
				}
			}

		}

		if( CommandLine != NULL )
		{
			free( CommandLine );
			CommandLine = NULL;
		}
	}
	
	///WaitForApplication Exit


	CloseHandle( hTokenThis );
	CloseHandle( hTokenDup ); 
	CloseHandle( hThisProcess );

	SetProcessWindowStation(hwinstaCurrent);
	SetThreadDesktop(hdeskCurrent);
	CloseWindowStation(hwinsta);
	CloseDesktop(hdesk);
	return bRet;
}


BOOL InitCheapman()
{
	SECURITY_ATTRIBUTES Sa = {0};
	
	PSECURITY_DESCRIPTOR pSd = GkoInitializeSecurityDescriptor( &Sa );

	__gPasswordEventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );

	__gClientSyncEventHandle = CreateEvent( &Sa, FALSE, FALSE, _GINGKO_SYNC_EVENT );

	if( pSd != NULL )
	{
		LocalFree( pSd );
	}

	_gNotifyMessageMutex = CreateMutex( NULL, FALSE, NULL );

	return TRUE;
}

BOOL UninitCheapman()
{
	//FreeDWMTheme();
	//
	//_Module.Term();

	__gApplicationExit = TRUE;
	
	SetEvent( __gPasswordEventHandle );

	SetEvent( __gClientSyncEventHandle );

	Sleep( 500 );

	CloseHandle( __gClientSyncEventHandle );

	return TRUE;
}


BOOL InsertGingkoPassword( unsigned char *fileHash, unsigned char *passwd, int pwdLength, ULONG dwProcessId )
{
	GingkoPasswordTable *pTable = __gPasswordTable.Next;
	while( pTable != NULL )
	{
		if( memcmp( pTable->FileHash, fileHash, 32 ) == 0 && pTable->dwProcessId == dwProcessId )
		{
			memcpy( pTable->Password, passwd, 64 ); //PASSWD SHOULD NOT BE NULL VALUE
			pTable->PasswordGot = TRUE;
			pTable->dwProcessId = dwProcessId;
			pTable->PasswordLength = pwdLength;
			return TRUE;
		}
		pTable = pTable->Next;
	}

	GingkoPasswordTable *pNewTable = (GingkoPasswordTable *) malloc(sizeof(GingkoPasswordTable));
	if( pNewTable == NULL )
	{
		return FALSE;
	}

	memcpy( pNewTable->FileHash, fileHash, 32 );
	if( passwd == NULL )
	{
		pNewTable->PasswordGot = FALSE;
	}else{
		pNewTable->PasswordGot = TRUE;
		memcpy( pNewTable->Password, passwd, 64 );
	}
	pNewTable->dwProcessId = dwProcessId;
	pNewTable->PasswordLength = pwdLength;

	pTable = __gPasswordTable.Next;
	pNewTable->Next = pTable;
	__gPasswordTable.Next = pNewTable;
	
	return TRUE;
}

BOOL FindGingkoPassword( unsigned char *fileHash, ULONG dwProcessId, unsigned char *passwd, int* pwdLength, BOOL bRemove )
{
	BOOL bRet = FALSE;
	GingkoPasswordTable *pTable = __gPasswordTable.Next;
	GingkoPasswordTable *pBefore = NULL;
	while( pTable != NULL )
	{
		if( memcmp( pTable->FileHash, fileHash, 32 ) == 0 && pTable->dwProcessId == dwProcessId )
		{
			///Copy to return
			if( pwdLength != NULL )
			{
				*pwdLength = pTable->PasswordLength;
			}
			
			if( passwd != NULL )
			{
				memcpy( passwd, pTable->Password, 64 );
			}

			bRet = TRUE;

			if( bRemove == TRUE )
			{
				bRet = pTable->PasswordGot;
				///Free Here
				if( pBefore == NULL )
				{
					__gPasswordTable.Next = pTable->Next;
					free( pTable );
				}else{
					pBefore->Next = pTable->Next;
					free( pTable );
				}
			}

			return bRet;
		}
		pBefore = pTable;
		pTable = pTable->Next;
	}
	return FALSE;
}


/***
 * This routine try to inject a Dll to the request ProcessId
 ****/
DWORD TryRemoteInjection( DWORD dwProcessId )
{
	HANDLE hToken = NULL;
	DWORD  dwSessionId = 0;
	DWORD  dwReLength = 0; 
	HANDLE hThisProcess = NULL;
	HANDLE hThisToken = NULL;
	BOOL   IsProcess32Bit = FALSE;
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );
	
	if( !hProcess ){
		return 0L;	///Can not this Process by Id. So, I will just faild this password request.
	}


	if( !OpenProcessToken( hProcess, TOKEN_ALL_ACCESS, &hToken) )
	{
		CloseHandle( hProcess );
		return 0L;
	}

	if( !GetTokenInformation( hToken, TokenSessionId, &dwSessionId, sizeof(DWORD), &dwReLength ) )
	{
		CloseHandle( hProcess );
		CloseHandle( hToken );
		return 0L;
	}
	
	if( IsWindows64Bit() )
	{
		if( IsProcessWow32( hProcess ) )
		{
			IsProcess32Bit = TRUE;
		}else{
			IsProcess32Bit = FALSE;
		}
	}else
	{
		IsProcess32Bit = TRUE;
	}

	CloseHandle( hProcess );
	CloseHandle( hToken );

	TCHAR szCommandLine[120] = {0};

	_stprintf_s( szCommandLine, 120, _T(" -Hook %d"), dwProcessId );

	return ExecuteProcessAsSession( GetFishshellPath(IsProcess32Bit), szCommandLine, dwSessionId );
}

DWORD ExecuteProcessAsSession(const TCHAR* szExePath, TCHAR* szCommandLine, DWORD dwSessionId )
{
	HANDLE hThisToken = NULL;
	HANDLE hTokenDup = NULL;
	STARTUPINFO si = {0}; 
	PROCESS_INFORMATION pi = {0}; 
	DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT; 
	HANDLE hThisProcess = GetCurrentProcess();
	
	if( !OpenProcessToken( hThisProcess, TOKEN_ALL_ACCESS, &hThisToken ) )
	{
		CloseHandle( hThisProcess );
		return 0L;
	}

	if( FALSE == DuplicateTokenEx(hThisToken, MAXIMUM_ALLOWED,NULL, SecurityIdentification, TokenPrimary, &hTokenDup) )
	{
		LOG(L"Error to duplicatetoken.\n");
		CloseHandle( hThisProcess );
		CloseHandle( hThisToken );
		return 0L;
	}
		
	SetLastError(0); 

	if( FALSE == SetTokenInformation( hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)) )
	{
		DWORD dwLastError = GetLastError();
		LOG(L"ERROR TO SET THE TokenSessionId to current process. Error Code: %d\n", dwLastError );
		CloseHandle( hThisProcess );
		CloseHandle( hThisToken );
		CloseHandle( hTokenDup );
		return 0L;
	}

	si.cb = sizeof(STARTUPINFO); 
	si.lpDesktop = L"WinSta0\\Default"; 

	if( FALSE == CreateProcessAsUser( 
				  hTokenDup, 
				  szExePath, 
				  szCommandLine, 
				  NULL, 
				  NULL, 
				  FALSE, 
				  dwCreationFlag, 
				  NULL, 
				  NULL, 
				  &si, 
				  &pi) )
	{
		DWORD ErrorCode = GetLastError();
		LOG(L"ERROR TO CREATE PROCESS AS OTHER USER. %d\n", ErrorCode );
	}

	DWORD dwExitCode = 0L;

	if( pi.hProcess != NULL && pi.hProcess != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject( pi.hProcess, INFINITE );
		GetExitCodeProcess( pi.hProcess, &dwExitCode );
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}

	CloseHandle( hThisProcess );
	CloseHandle( hThisToken );
	CloseHandle( hTokenDup );

	return dwExitCode;
}


BOOL IsProcessUnderService( DWORD dwProcessId )
{
	HANDLE hToken = NULL;
	TOKEN_USER* TkOwner = NULL;
	DWORD dwLength = 0;
	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );

	if( hProcess == NULL )
	{
		return FALSE;
	}

	if( !OpenProcessToken( hProcess, TOKEN_ALL_ACCESS, &hToken) )
	{
		CloseHandle( hProcess );
		return FALSE;
	}

	if( !GetTokenInformation( hToken, TokenUser, NULL, 0, &dwLength ) )
	{
		if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
		{
			CloseHandle( hProcess );
			CloseHandle( hToken );
			return FALSE;
		}
	}

	TkOwner = (TOKEN_USER*) GlobalAlloc( GPTR, dwLength );
	///ptu=(PTOKEN_USER)   GlobalAlloc(GPTR,cb); 
	if( !GetTokenInformation( hToken, TokenUser, TkOwner, dwLength, &dwLength ) )
	{
		CloseHandle( hProcess );
		CloseHandle( hToken );
		GlobalFree( TkOwner );
		return FALSE;
	}
	

	TCHAR	name[MAX_PATH + 1] = {0}; 
	DWORD   cchName = MAX_PATH; 
	TCHAR	domain[MAX_PATH + 1] = {0}; 
	DWORD   cchDomain = MAX_PATH; 
	SID_NAME_USE   snu; 

	if( !LookupAccountSid( NULL, TkOwner->User.Sid, name, &cchName, domain, &cchDomain, &snu ) )
	{
		CloseHandle( hProcess );
		CloseHandle( hToken );
		GlobalFree( TkOwner );
		return FALSE;
	}

	LOG( L"The process(0x%08x) is running under %s\\%s.\n", dwProcessId, name, domain );

	CloseHandle( hProcess );
	CloseHandle( hToken );
	GlobalFree( TkOwner );

	if( _tcsicmp( name, _T("SYSTEM") ) == 0 ||
		_tcsicmp( name, _T("LOCAL SERVICE") ) == 0 ||
		_tcsicmp( name, _T("NETWORK SERVICE") ) == 0 )
	{
		return TRUE;
	}

	return FALSE;
}

