// Bootstrap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winerror.h>
#include "FindFiles.h"
#include "bootstrap.h"
#include "cheapman.h"
#include "random.h"

HANDLE hDriver = INVALID_HANDLE_VALUE;

int DoDriverInstall();
static int DriverLoad ();
static int StartDriverService();
void TestDecryptProcess(LPCWSTR fileName);
void TestEncryptProcess(LPCWSTR fileName,LPCWSTR OutputFileName);
void EncryptionControl(BOOL Action);
void TestEncryptFileMapping( LPCWSTR fileName );
BOOL DriverUnload ();
BOOL CopyFileDest( TCHAR* pszSourceFile, TCHAR *pszDestFile, BOOL bFailExist );

int _tmain(int argc, _TCHAR* argv[])
{
	if( argc > 1 )
	{
		if( wcscmp( argv[1], L"-Install" ) == 0 ){
			DriverLoad ();
			DatabaseInstall();
		}
		else if( wcscmp( argv[1], L"-Remove" ) == 0 ){
			DriverUnload();
		}
		else if( wcscmp( argv[1], L"-Map" ) == 0 )
		{
			if( argc > 2 )
			{
				TestEncryptFileMapping( argv[2] );
			}
		}
		else if( wcscmp( argv[1], L"-Run" ) == 0 ){
			StartDriverService();
			OpenDatabase( FALSE );
			HANDLE	RequestReadQueueEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
			ThreadBag* bag;

			//Randinit();
			StartThread( DeviceIoThreadProc, NULL, &bag );

			if( bag != NULL )
			{

			}
				//WaitForSingleObject( bag->hThread, INFINITE );
			getchar();

			CloseDatabase();
		}
		else if( wcscmp( argv[1], L"-Write" ) == 0 ){
			StartDriverService();

			if( argc > 3 )
			{
				EncryptionControl(TRUE);
				TestEncryptProcess( argv[2], argv[3]  );
				EncryptionControl(FALSE);
			}else{
				wprintf(L"%s: -Write <Input File> <Output File>\n", argv[0]);
				return 1;
			}
		}else if( wcscmp( argv[1], L"-DB" ) == 0 )
		{
			//Randinit();
			DatabaseInstall();
			wprintf(L"Install database is OK.\n");
			OpenDatabase( FALSE );
			wprintf(L"Try to open database is OK.\n");
			PrivateKeyRequest* request = (PrivateKeyRequest*) malloc( sizeof(PrivateKeyRequest) + sizeof(WCHAR)* MAX_PATH );
			PrivateKeyResponse response;
			if( request != NULL )
			{
				memset( request, 0, sizeof(PrivateKeyRequest) + sizeof(WCHAR)* MAX_PATH );
				HexToBytes( (const unsigned char*)"585a59303031", request->Company, 12 );
				HexToBytes( (const unsigned char*)"6fe244d8d619064a8abfe23608d27d15", request->Md5, 32 );
				memcpy( request->Filename, L"E:\\testing\\test.doc", 19 * 2 );
				request->FilenameLength = 19 * 2;

				QueryPrimaryKey( request, &response );
				RenewPrimaryKey( request, &response );

				free( request );
			}
		}
		else if( wcscmp( argv[1], L"-Combine" ) == 0 ){
			WCHAR* Path = NULL;
			if( argc <= 2 )
			{
				wprintf(L"%s: -Combine <File Pattern> [<OutputFileName>] [<Changed Directory>]\n", argv[0]);
				return 1;
			}

			if( argc < 4 )
			{
				DWORD nSize = GetCurrentDirectory( 0, NULL );
				if( nSize > 0 )
				{
					Path = (WCHAR*) malloc( sizeof(WCHAR) * (nSize + 1) );
					if( Path != NULL )
					{
						if( GetCurrentDirectory( nSize, Path ) == 0 )
						{
							free( Path );
							Path = NULL;
						}
					}
				}
			}

			CombineDirectory( argv[2], argc > 3 ? argv[3] : L"combined.out", Path ); 
			
			if( Path != NULL )
			{
				free( Path );
			}
		}
		else if(  wcscmp( argv[1], L"-Copy" ) == 0 ) {
			if( argc >= 4 )
			{
				CopyFileDest( argv[2], argv[3], FALSE );
			}else{
				wprintf(L"%s: -Copy <Source File> <Dest File>\n", argv[0]);
			}
		}else if( argv[1][0] == L'-' ){
			wprintf( L"%s: -Install\n%s: -Remove\n", argv[0], argv[0] ); 
		}else{
			StartDriverService();
			TestDecryptProcess( L"c:\\Test\\test_out--gko-512.pdf" );
		}
	
	}
	return 0;
}

BOOL CopyFileDest( TCHAR* pszSourceFile, TCHAR *pszDestFile, BOOL bFailExist )
{
	HANDLE hSourceFile = CreateFile( pszSourceFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	HANDLE hDestFile = NULL;
	if( hSourceFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	hDestFile = CreateFile( pszDestFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if(hDestFile == INVALID_HANDLE_VALUE )
	{
		CloseHandle( hSourceFile );
		return FALSE;
	}
	
	unsigned char temp[4096];
	DWORD ReadBytes = 0;
	DWORD WriteBytes  = 0;
	do{
		ReadFile( hSourceFile, temp, 4096, &ReadBytes, NULL );
		if( ReadBytes > 0 )
		{
			WriteFile( hDestFile, temp, ReadBytes, &WriteBytes, NULL );
		}
	}while(ReadBytes > 0 );
	CloseHandle( hSourceFile );
	CloseHandle( hDestFile );
	return TRUE;
}

BOOL Is64BitOs ()
{
    static BOOL isWow64 = FALSE;
	static BOOL valid = FALSE;
	typedef BOOL (__stdcall *LPFN_ISWOW64PROCESS ) (HANDLE hProcess,PBOOL Wow64Process);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	if (valid)
		return isWow64;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");

    if (fnIsWow64Process != NULL)
        if (!fnIsWow64Process (GetCurrentProcess(), &isWow64))
			isWow64 = FALSE;

	valid = TRUE;
    return isWow64;
}

int DoDriverInstall()
{
	SC_HANDLE hManager, hService = NULL;
	BOOL bOK = FALSE, bRet;

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		goto error;

	hService = CreateService (hManager, L"Bluefisher", L"Bluefisher",
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_SYSTEM_START, SERVICE_ERROR_NORMAL,
		!Is64BitOs () ? L"System32\\drivers\\bluefish.sys" : L"SysWOW64\\drivers\\bluefish-x64.sys",
		NULL, NULL, NULL, NULL, NULL);

	if (hService == NULL)
		goto error;
	else
		CloseServiceHandle (hService);

	hService = OpenService (hManager, L"Bluefisher", SERVICE_ALL_ACCESS);
	if (hService == NULL)
		goto error;

	bRet = StartService (hService, 0, NULL);
	if (bRet == FALSE)
		goto error;

	bOK = TRUE;

error:
	if (bOK == FALSE && GetLastError () != ERROR_SERVICE_ALREADY_RUNNING)
	{
		MessageBoxW (NULL, L"DRIVER_INSTALL_FAILED", L"Failed to install the driver.", MB_ICONHAND);
	}
	else
		bOK = TRUE;

	if (hService != NULL)
		CloseServiceHandle (hService);

	if (hManager != NULL)
		CloseServiceHandle (hManager);

	return 0;
}

int GetDriverRefCount ()
{
	int refCount = 0;
	return refCount;
}



// Install and start driver service and mark it for removal (non-install mode)
static int DriverLoad ()
{
	HANDLE file = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find = {0};
	SC_HANDLE hManager = NULL, hService = NULL;
	WCHAR driverPath[MAX_PATH*2] = {0};
	BOOL res = FALSE;
	WCHAR *tmp;

	GetModuleFileName (NULL, driverPath, sizeof (driverPath));
	tmp = wcsrchr (driverPath, '\\');
	if (!tmp)
	{
		wcscpy_s(driverPath, MAX_PATH * 2, L".");
		tmp = driverPath + 1;
	}

	wcscpy_s (tmp, MAX_PATH, !Is64BitOs () ? L"\\Bluefish.sys" : L"\\Bluefish-x64.sys");

	file = FindFirstFile (driverPath, &find);

	if (file == INVALID_HANDLE_VALUE)
	{
		wprintf (L"DRIVER_NOT_FOUND, was the driver stored in %s?\n", driverPath );
		return ERR_DONT_REPORT;
	}

	FindClose (file);

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
	{
		if (GetLastError () == ERROR_ACCESS_DENIED)
		{
			
			wprintf (L"ADMIN_PRIVILEGES_DRIVER No admin privileges of this driver.\n", 0);
			return ERR_DONT_REPORT;
		}

		return ERR_OS_ERROR;
	}

	hService = OpenService (hManager, L"Bluefisher", SERVICE_ALL_ACCESS);
	if (hService != NULL)
	{
		// Remove stale service (driver is not loaded but service exists)
		wprintf(L"Driver is not loaded but the service exits.\n");
		DeleteService (hService);
		CloseServiceHandle (hService);
		Sleep (500);
	}
	//Clear the Error
	SetLastError(0);

	wprintf( L"Installing Bluefisher driver...\n");
	wprintf( L"Driver Path: %s\n", driverPath );

	hService = CreateService (hManager, L"Bluefisher", L"Bluefisher",
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		driverPath, NULL, NULL, NULL, NULL, NULL);

	if (hService == NULL)
	{
		wprintf(L"Bluefisher was not created. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
		return ERR_OS_ERROR;
	}

	CloseServiceHandle (hService);

	hService = OpenService (hManager, L"Bluefisher", SERVICE_ALL_ACCESS);

	if (hService == NULL)
	{
		wprintf(L"Bluefisher was not opened. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
		return ERR_OS_ERROR;
	}

	if( !StartService (hService, 0, NULL) )
	{
		wprintf(L"Bluefisher was not started. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
	}

	//DeleteService (hService);

	CloseServiceHandle (hManager);
	CloseServiceHandle (hService);

	if( GetLastError() != 0 )
	{
		wprintf(L"Error: %d.\n", GetLastError() );
	}else
	{
		wprintf(L"Bluefisher driver installed successfully.\n" );
	}

	return !res ? ERR_OS_ERROR : ERROR_SUCCESS;
}

static int StartDriverService()
{
	HANDLE file = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find = {0};
	SC_HANDLE hManager = NULL, hService = NULL;
	WCHAR driverPath[MAX_PATH*2] = {0};
	BOOL res = FALSE;

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
	{
		if (GetLastError () == ERROR_ACCESS_DENIED)
		{
			
			wprintf (L"ADMIN_PRIVILEGES_DRIVER No admin privileges of this driver.\n", 0);
			return ERR_DONT_REPORT;
		}

		return ERR_OS_ERROR;
	}


	hService = OpenService (hManager, L"Bluefisher", SERVICE_ALL_ACCESS);

	if (hService == NULL)
	{
		wprintf(L"Bluefisher was not opened. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
		return ERR_OS_ERROR;
	}

	if( !StartService (hService, 0, NULL) )
	{
		wprintf(L"Bluefisher was not started. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
	}

	//DeleteService (hService);

	CloseServiceHandle (hManager);
	CloseServiceHandle (hService);

	if( GetLastError() != 0 )
	{
		wprintf(L"Error: %d.\n", GetLastError() );
	}else
	{
		wprintf(L"Bluefisher driver was started successfully.\n" );
	}

	return !res ? ERR_OS_ERROR : ERROR_SUCCESS;
}

BOOL DriverUnload ()
{
	SC_HANDLE hManager = NULL, hService = NULL;
	BOOL bRet = FALSE;
	SERVICE_STATUS status = {0};
	int x = 0;

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		goto error;

	hService = OpenService (hManager, L"Bluefisher", SERVICE_ALL_ACCESS);
	if (hService == NULL)
	{
		wprintf( L"Open Service for Bluefisher was failed. Error: %d.\n", GetLastError() );
		SetLastError(0);
		goto error;
	}

	bRet = QueryServiceStatus (hService, &status);
	if (bRet != TRUE)
	{
		wprintf( L"Query Service status failed. Error: %d.\n", GetLastError() );
		SetLastError(0);
		goto error;
	}

	if (status.dwCurrentState != SERVICE_STOPPED)
	{
		ControlService (hService, SERVICE_CONTROL_STOP, &status);

		for (x = 0; x < 5; x++)
		{
			bRet = QueryServiceStatus (hService, &status);
			if (bRet != TRUE)
			{
				wprintf( L"Query Service status failed. Loop %d. Error: %d.\n", x, GetLastError() );
				SetLastError(0);
				goto error;
			}
			
			if (status.dwCurrentState == SERVICE_STOPPED)
				break;

			Sleep (200);
		}
	}

	DeleteService( hService );

error:
	if (hService != NULL)
		CloseServiceHandle (hService);

	if (hManager != NULL)
		CloseServiceHandle (hManager);

	if (status.dwCurrentState == SERVICE_STOPPED)
	{
		//hDriver = INVALID_HANDLE_VALUE;
		return TRUE;
	}

	return FALSE;
}


void TestDecryptProcess(LPCWSTR fileName)
{
	HANDLE hFile = CreateFile( fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		char buf[4096];
		DWORD nSize = 0;
		do{
			ReadFile( hFile, buf, 4096, &nSize, NULL );
		}while( nSize > 0 );
		CloseHandle( hFile );
	}
	return;
}

void TestEncryptProcess(LPCWSTR fileName,LPCWSTR OutputFileName)
{
	HANDLE hOutputFile = CreateFile( OutputFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hOutputFile != INVALID_HANDLE_VALUE )
	{
		HANDLE hFile = CreateFile( fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile != INVALID_HANDLE_VALUE )
		{
			char buf[512];
			DWORD nSize = 0;
			do{
				memset( buf, 0, sizeof(buf) );
				ReadFile( hFile, buf, sizeof(buf), &nSize, NULL );
				WriteFile( hOutputFile, buf, nSize, &nSize, NULL );
			}while( nSize > 0 );
			CloseHandle( hFile );
		}
		CloseHandle(hOutputFile);
	}

	return;
}

void TestEncryptFileMapping( LPCWSTR fileName )
{
	HANDLE hOutputFile = CreateFile( fileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hOutputFile != INVALID_HANDLE_VALUE )
	{
		DWORD FileSizeHigh = 0L;
		DWORD FileSizeLow = GetFileSize( hOutputFile, &FileSizeHigh );
		GINGKO_HEADER *Header = NULL;
		SetFilePointer( hOutputFile, 0, 0, FILE_END );

		SetEndOfFile( hOutputFile );

		HANDLE hMap = CreateFileMapping( hOutputFile, NULL, PAGE_READWRITE | SEC_NOCACHE | SEC_RESERVE,
			0, FileSizeLow, NULL );

		if( hMap == NULL )
		{
			wprintf( L"Last Error To Create FileMapping: %d.\n", GetLastError());
		}

		Header =(GINGKO_HEADER*) MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GINGKO_HEADER) );
		if( Header == NULL )
		{
			wprintf(L"MapViewOfFile: %d\n", GetLastError());
		}else
		{
			wprintf( L"Header Identity: %c%c%c%c\n", Header->Identifier[0], Header->Identifier[1], Header->Identifier[2], Header->Identifier[3]);
		}


		LPVOID BaseAddress = VirtualAlloc( NULL, 4096*1024, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE );

		if( BaseAddress == NULL )
		{
			wprintf(L"VirtualAlloc Error: %d\n", GetLastError());
		}

		char *pbuf = (char*)MapViewOfFileEx( hMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, FileSizeLow, NULL );

		if( pbuf == NULL )
		{
			wprintf(L"MapViewOfFile2: %d\n", GetLastError());
		}

		for( DWORD dwLoop = 0; dwLoop < FileSizeLow; dwLoop ++ )
		{
			wprintf( L"%C", pbuf[dwLoop] );
		}

		if( BaseAddress != NULL )
		{
			VirtualFree( BaseAddress, FileSizeLow, MEM_RELEASE );
		}

		UnmapViewOfFile( Header );

		CloseHandle( hMap );
		CloseHandle( hOutputFile );
	}
}

void EncryptionControl(BOOL Action)
{
	DWORD dwSize = 0;

	if( hDriver == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( hDriver == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Open driver failed with the error: %d\n", GetLastError());
			return;
		}
	}

	if( !DeviceIoControl( hDriver, Action ? GINGKO_SET_ENCRYPT_START : GINGKO_SET_ENCRYPT_END, NULL, 0, NULL, 0, &dwSize, NULL ) )
	{
		wprintf(L"Control to hDriver was failed. with the error code: %d.\n", GetLastError());
		return;
	}
}
