// Fishshell.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GingkoRemote.h"

FILE* hLogFile = NULL;
HANDLE hLogMutex = NULL;
void PrintUsage(TCHAR* argv);

int InjectRemoteProcess(ULONG dwProcessId);
int EjectRemoteProcess(ULONG dwProcessId, ULONG hMod);

BOOL GetImagePath( TCHAR* path, DWORD nSize, const TCHAR* lpName )
{
	TCHAR ImagePath[MAX_PATH + 1] = {0};
	DWORD dwPathLength = GetModuleFileName( NULL, ImagePath, MAX_PATH );
	if( dwPathLength <= 0 )
	{
		return FALSE;
	}

	for( DWORD End = dwPathLength; End > 0 ; End -- )
	{
		if( ImagePath[End] == _T('\\') )
		{
			ImagePath[End] = 0;
			break;
		}else
		{
			ImagePath[End] = 0;
		}
	}

	_stprintf_s( path, nSize, _T("%s\\%s"), ImagePath, lpName );

	return TRUE;
}


///int _tmain(int argc, _TCHAR* argv[])
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	int argc = 0;
	DWORD dwRet = 0;
	LPTSTR* argv = CommandLineToArgvW( lpstrCmdLine, &argc);
	if( NULL == argv )
	{
	  wprintf(L"CommandLineToArgvW failed\n");
	  return 0;
	}


	
	if( argc >=  2 )
	{
		
		if( _tcscmp( argv[0], _T("-Hook") ) == 0 )
		{
			dwRet = InjectRemoteProcess( _ttol( argv[1] ) );
		}else if( _tcscmp( argv[2], _T("-UnHook") ) == 0 ){
			dwRet = EjectRemoteProcess( _ttol( argv[1] ), _ttol( argv[2] ));
		}else{
			PrintUsage(argv[0]);
		}
	}else
	{
		PrintUsage(argv[0]);
	}
	LocalFree( argv );
	return dwRet;
}

void PrintUsage( TCHAR* argv )
{
	_tprintf_s( _T("The FishShell Console inject/eject a Fish Security Dll to an application.\n") );
	_tprintf_s( _T("Copyrights 2009 JPoet Secruity System Co.Ltd.\n") );
	_tprintf_s( _T("To use this tool, follow is the syntax.\n") );
	_tprintf_s( _T("\tInject a Dll to   a process: %s -Hook <ProcessId>.\n"), argv );
	_tprintf_s( _T("\tEject  a Dll from a process: %s -UnHook <ProcessId> <HMODULE HANDLE>.\n"), argv );
}

int InjectRemoteProcess(ULONG dwProcessId)
{

	HANDLE hProcess = NULL;	
	HINSTANCE hRemoteDll = NULL;
	DWORD dwProcessFlags = 0L;
	
	TCHAR ImagePath[MAX_PATH + 1] = {0};
	
	if( !Initialization() )
		return 0;

	hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );


	if( hProcess != NULL && hProcess != INVALID_HANDLE_VALUE )
	{
		BOOL bImagePathOK = FALSE;
#ifdef _M_AMD64
		bImagePathOK = GetImagePath( ImagePath, MAX_PATH, _T("Jackfish64.dll") );
#else
		bImagePathOK = GetImagePath( ImagePath, MAX_PATH, _T("Jackfish32.dll") );
#endif
		if( !bImagePathOK )
			return 0;

		dwProcessFlags = GetProcessInfo( hProcess );
		
		return Inject( hProcess, dwProcessFlags, ImagePath, 10000, &hRemoteDll );
		
	}

	return 0;
}



int EjectRemoteProcess(ULONG dwProcessId, ULONG hMod)
{
	return 0xDD;
}