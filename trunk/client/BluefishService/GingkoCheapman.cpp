#include "stdafx.h"
#include "cheapman.h"
#include "GingkoServiceDispatcher.h"
#include "GingkoLibrary.h"
#define  BLUEFISHER_DRIVER			_T("Bluefisher")
#define  BLUEFISHER_DRIVER_DISP		_T("Bluefisher Security Filter Driver")

HANDLE __g_DriverHandler = INVALID_HANDLE_VALUE;

HANDLE RequestEvent = NULL;

BOOL InitialServiceUrl();

BOOL CommitPrimaryKey(PrivateKeyRequest* pHeader, const unsigned char* primaryKey, int KeySize);

LPWSTR ConvertToGingkoFileId(PrivateKeyRequest* pHeader, LPWSTR Suffix);

BOOL LoadPrimaryKey(PrivateKeyRequest* pHeader, unsigned char* primaryKey, int KeySize);

extern BOOL Is64BitWindows;
volatile ULONG gChildThreadCount = 0;

BOOL DeviceIoSetRequestReadEvent(SharedNotification *notification)
{
	DWORD ret = 0;
	
	if( !notification )
	{
		return FALSE;
	}

	if( __g_DriverHandler == INVALID_HANDLE_VALUE || __g_DriverHandler == NULL)
	{
		OpenDriver();
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}


	if( DeviceIoControl( __g_DriverHandler, GINGKO_DEVICE_START, notification, sizeof(SharedNotification), NULL, 0, &ret, NULL ) )
	{
		LOG(L"Notify the server process was success." );
		return TRUE;
	}
	LOG(L"Notify the server process was NOT success." );
	return FALSE;
}

BOOL DeviceIoSetRequestStop()
{
	DWORD ret = 0;
	
	if( __g_DriverHandler == INVALID_HANDLE_VALUE || __g_DriverHandler == NULL)
	{
		OpenDriver();
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	if( DeviceIoControl( __g_DriverHandler, GINGKO_DEVICE_STOP, NULL, 0, NULL, 0, &ret, NULL ) )
	{
		LOG(L"Server process was notified to be stoped successfully." );
		return TRUE;
	}
	LOG(L"Server process was notified to be stoped, but is not successful." );
	return FALSE;
}

BOOL DeviceIoQueryPermission(ULONG ProcessId, ULONG *Permission)
{
	DWORD ret = 0;
	if( Permission == NULL )
	{
		return FALSE;
	}

	if( __g_DriverHandler == INVALID_HANDLE_VALUE || __g_DriverHandler == NULL)
	{
		OpenDriver();
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	if( DeviceIoControl( __g_DriverHandler, GINGKO_QUERY_PROCESS_PERMISSION, &ProcessId, sizeof(ULONG), (LPVOID)Permission, sizeof(ULONG), &ret, NULL ) )
	{
		LOG(L"Get the process (%d) permission is %08x.", ProcessId, *Permission );
		return TRUE;
	}

	LOG(L"Server process was notified to be stoped, but is not successful." );
	return FALSE;
}


BOOL DeviceIoGetPrivateKeyRequest(PrivateKeyRequest **pkr)
{
	DWORD ret = 0;
	PrivateKeyRequest* request = (PrivateKeyRequest*)malloc( sizeof(PrivateKeyRequest) + sizeof(WCHAR) * MAX_PATH );
	if( !request )
	{
		return FALSE;
	}

	if( __g_DriverHandler == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	
	memset( request, 0 , sizeof(PrivateKeyRequest) + sizeof(WCHAR) * MAX_PATH );

	if( DeviceIoControl( __g_DriverHandler, GINGKO_GET_REQUEST, NULL, 0, request, sizeof(PrivateKeyRequest) + sizeof(WCHAR) * MAX_PATH, &ret, NULL ) )
	{
		if( ret > 0 )
		{
			if( pkr != NULL )
			{
				*pkr = request;
				LOG(L"The request file name: %s.\n", request->Filename );
				return TRUE;
			}
		}
	}

	free( request );

	return FALSE;
}

BOOL DeviceIoPutPrivateKeyResponse(PrivateKeyRequest *pkr, PrivateKeyResponse *response )
{
	DWORD ret = 0;

	if( __g_DriverHandler == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	return DeviceIoControl( __g_DriverHandler, GINGKO_PUT_RESPONSE, response, sizeof(PrivateKeyResponse), NULL, 0, &ret, NULL );
}

BOOL StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, PVOID ThreadArgs, ThreadBag **pBag )
{
	ThreadBag *bag = (ThreadBag*) malloc( sizeof(ThreadBag) );
	if( bag != NULL )
	{
		DWORD dwThreadId;
		if( pBag != NULL )
		{
			bag->hEvent  = CreateEvent( NULL, FALSE, FALSE , NULL );
			if( bag->hEvent == NULL || bag->hEvent == INVALID_HANDLE_VALUE )
			{
				LOG(L"Create Event failed with error code: %d\n", GetLastError());
				free(bag);
				return FALSE;
			}
		}

		bag->bThreadExit = FALSE;
		bag->ThreadArgs = ThreadArgs;
		
		if( pBag != NULL )
		{
			bag->hThread = CreateThread( NULL, 0, ThreadRoutine, bag, CREATE_SUSPENDED, &dwThreadId ); 
		}else
		{
			bag->hThread = CreateThread( NULL, 0, ThreadRoutine, ThreadArgs, CREATE_SUSPENDED, &dwThreadId ); 
		}

		bag->dwThreadId = dwThreadId;
		
		if( bag->hThread == NULL || bag->hThread == INVALID_HANDLE_VALUE )
		{
			LOG(L"Create Thread failed with error code: %d\n", GetLastError());
			free( bag );
			return FALSE;
		}

		HANDLE hThread = bag->hThread;

		if( pBag )
		{
			*pBag = bag;
		}else
		{
			free( bag );
		}

		ResumeThread( hThread );


		return TRUE;
	}
	return FALSE;
}

BOOL StopThread(ThreadBag *bag)
{
	bag->bThreadExit = TRUE;
	
	SetEvent( bag->hEvent );

	WaitForSingleObject( bag->hThread, INFINITE );
	
	//CloseHandle( bag->hEvent );
	
	CloseHandle( bag->hThread );
	
	free( bag );
	
	return TRUE;
}

DWORD WINAPI DeviceIoThreadProc(PVOID threadArg)
{
	ThreadBag *bags = (ThreadBag*) threadArg;
	ULONG	  dwChildThreadCount = 0;
	SharedNotification notification = {0};

	CoInitialize(NULL);

	notification.cbStruct = sizeof( SharedNotification);
	notification.ClientProcess = NULL;
	notification.ReadEvent = bags->hEvent;
	notification.WriteEvent = NULL;
	//notification.ServerProcess = GetCurrentProcessId();

	SetLastError(0);
	
	///After _loadConfig

	GingkoOptions *pOptions = NULL;

	InitialServiceUrl();

	DetectGingkoServer();

	OpenDatabase( FALSE );

	if( DeviceIoSetRequestReadEvent(&notification) )
	{
		LOG(L"The Driver was notified to started. The event object is %p\n", bags->hEvent );
		while( !bags->bThreadExit )
		{
			DWORD dw = WaitForSingleObject( bags->hEvent, INFINITE );
			LOG(L"Get a request from Kernel notification. %d==%d. Err: %d.\n", dw, WAIT_OBJECT_0, GetLastError() );
			PrivateKeyRequest *request = NULL;
			BOOL TryAgain = TRUE;
			WaitForDetectGingkoServer();
			do{
				if( DeviceIoGetPrivateKeyRequest( &request ) )
				{
					if( request != NULL )
					{
						LOG(L"Got the Key Request.\n");
						TryAgain = TRUE;
						StartThread( RequestKeyThreadProc, request, NULL );
						InterlockedIncrement( &gChildThreadCount );
					}else{
						TryAgain = FALSE;
					}
				}else{ TryAgain = FALSE; }
			}while( TryAgain );
		}
	}

	while( InterlockedExchangeAdd( &gChildThreadCount, 0 ) > 0 )
	{
		Sleep( 1000 );
	}

	DeviceIoSetRequestStop();

	CloseBluefishDriver();

	LOG(_T("Close global value of g_DriverHandler.\n"));

	CoUninitialize();

	CloseDatabase();

	LOG(_T("Close sqlite database.\n"));
	return 0L;
}

BOOL CloseBluefishDriver()
{
	if( __g_DriverHandler != INVALID_HANDLE_VALUE && __g_DriverHandler != NULL )
	{
		CloseHandle( __g_DriverHandler );
		__g_DriverHandler = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}

DWORD  WINAPI RequestKeyThreadProc(PVOID threadArg)
{
	LOG(L"Enter RequestKeyThreadproc.\n");

	PrivateKeyRequest *request = (PrivateKeyRequest*) threadArg;
	
	if( request == NULL )
	{
		InterlockedDecrement(&gChildThreadCount);
		return -1;
	}

	PrivateKeyResponse response = {0};

	__try
	{
		CopyRequestToResponse( request, &response );

		if( AuthenticProcess( request->dwProcessId, NULL ) )
		{
			if( request->Identifier[0] == 'G' &&
				request->Identifier[1] == 'K' &&
				request->Identifier[2] == 'T' &&
				request->Identifier[3] == 'F' )
			{
				
				///GET THE KEY FROM SERVER
				LOG(L"Get the Key from server.\n");
				if( TryRemoteInjection( request->dwProcessId ) != 0L )
				{
					QueryPrimaryKey( request, &response );
				}else
				{
					response.Permission = 0x80000000;
				}
			}
			else
			{
				RenewPrimaryKey( request, &response );
				response.Permission = 0x8000FFFF;
			}
			
		}else{
			response.Permission = 0x80000000;
		}
	}
	__finally
	{
		if( (response.Permission > (ULONG)0x80000000 ) && (response.Permission & 0x80000000) )
		{
			LOG(L"Calling NotifyProcessEvent which file has been opened by normal decryption.\n");
			NotifyProcessEvent( request->Filename, request->dwProcessId, response.Permission );
		}

		DeviceIoPutPrivateKeyResponse( request, &response );
		InterlockedDecrement(&gChildThreadCount);
	}
	return 0;
}

void CopyRequestToResponse( PrivateKeyRequest *request, PrivateKeyResponse *response )
{
	response->FileObject = (LONG_PTR) request->FileObject;
	response->ProcessId = request->dwProcessId;
	response->RequestKeyEvent = (LONG_PTR) request->RequestKeyEvent;
	response->SizeOfBlock = request->SizeOfBlock;
	response->SizeOfFile = request->SizeOfFile;
	response->SizeOfHeader = request->SizeOfHeader;
	
	memcpy( response->Company,		request->Company,		COMPANY_LENGTH );
	memcpy( response->Identifier,	request->Identifier,	GINGKO_IDENTIFIER_LENGTH );
	memcpy( response->Version,		request->Version,		GINGKO_VERSION_LENGTH );
	memcpy( response->Md5,			request->Md5,			MD5_DIGEST_LENGTH );
	memcpy( response->Method,		request->Method,		GINGKO_METHOD_LENGTH );
	response->SizeOfResponse = sizeof( PrivateKeyResponse );
}

void OpenDriver()
{
	if( __g_DriverHandler == INVALID_HANDLE_VALUE )
	{
		__g_DriverHandler = CreateFile( L"\\\\.\\Bluefisher", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( __g_DriverHandler == INVALID_HANDLE_VALUE )
		{
			LOG(L"Open driver failed with the error: %d\n", GetLastError());
			return;
		}
	}
}

BOOL CommitPrimaryKey(PrivateKeyRequest* pHeader, const unsigned char* primaryKey, int KeySize)
{
	LPWSTR KeyFileName = ConvertToGingkoFileId( pHeader, L".pk" );
	
	HANDLE hKeyFile = NULL;
	if( KeyFileName != NULL )
	{
		hKeyFile = CreateFile( KeyFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		free( KeyFileName );

		if( INVALID_HANDLE_VALUE != hKeyFile )
		{
			DWORD dwSizeWrittern = 0;
			WriteFile( hKeyFile, primaryKey, KeySize, &dwSizeWrittern, NULL );
			CloseHandle( hKeyFile );
			return TRUE;
		}
	}
	return FALSE;
}

LPWSTR ConvertToGingkoFileId(PrivateKeyRequest* pHeader, LPWSTR Suffix)
{
	WCHAR arrFileId[256];
	memset( arrFileId, 0, sizeof( arrFileId ) );

	_snwprintf_s( arrFileId, sizeof(arrFileId), 
		L"%02x%02x%02x%02x%02x%02x-%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%s",
		pHeader->Company[0], pHeader->Company[1],pHeader->Company[2],pHeader->Company[3],pHeader->Company[4],pHeader->Company[5],
		pHeader->Md5[0], pHeader->Md5[1],pHeader->Md5[2],pHeader->Md5[3],
		pHeader->Md5[4], pHeader->Md5[5],pHeader->Md5[6],pHeader->Md5[7],
		pHeader->Md5[8], pHeader->Md5[9],pHeader->Md5[10],pHeader->Md5[11],
		pHeader->Md5[12], pHeader->Md5[13],pHeader->Md5[14],pHeader->Md5[15],
		Suffix
		);
	return _wcsdup( arrFileId );
}

BOOL LoadPrimaryKey(PrivateKeyRequest* pHeader, unsigned char* primaryKey, int KeySize)
{
	LPWSTR KeyFileName = ConvertToGingkoFileId( pHeader, L".pk" );


	HANDLE hKeyFile = NULL;
	if( KeyFileName != NULL )
	{
		hKeyFile = CreateFile( KeyFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
		free( KeyFileName );

		if( INVALID_HANDLE_VALUE != hKeyFile )
		{
			DWORD dwSizeRead = 0;
			ReadFile( hKeyFile, primaryKey, KeySize, &dwSizeRead, NULL );
			CloseHandle( hKeyFile );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsWindows64Bit ()
{
	static BOOL IsChecked = FALSE;
	if( IsChecked == FALSE )
	{
		SYSTEM_INFO si = {0};

		GetNativeSystemInfo(&si); 
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||  si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) 
		{ 
			Is64BitWindows = TRUE;
		}else
		{
			Is64BitWindows = FALSE;
		}
		IsChecked = TRUE;
	}

	return Is64BitWindows;
}


int GetDriverRefCount ()
{
	int refCount = 0;
	return refCount;
}



// Install and start driver service and mark it for removal (non-install mode)
int DriverLoad ()
{
	HANDLE file = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find = {0};
	SC_HANDLE hManager = NULL, hService = NULL;
	WCHAR driverPath[MAX_PATH*2] = {0};
	BOOL res = FALSE;
	WCHAR *tmp;
	DWORD dwLastError = 0;

	GetModuleFileName (NULL, driverPath, sizeof (driverPath));
	tmp = wcsrchr (driverPath, '\\');
	if (!tmp)
	{
		wcscpy_s(driverPath, MAX_PATH * 2, L".");
		tmp = driverPath + 1;
	}

	wcscpy_s (tmp, MAX_PATH, !IsWindows64Bit () ? L"\\Bluefish.sys" : L"\\Bluefish-x64.sys");

	file = FindFirstFile (driverPath, &find);

	if (file == INVALID_HANDLE_VALUE)
	{
		LOG (L"DRIVER_NOT_FOUND, was the driver stored in %s?\n", driverPath );
		return ERROR_INVALID_ACCESS;
	}

	FindClose (file);

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
	{
		if (GetLastError () == ERROR_ACCESS_DENIED)
		{
			LOG (L"ADMIN_PRIVILEGES_DRIVER No admin privileges of this driver.\n");
			return ERROR_INVALID_ACCESS;
		}

		return ERROR_INVALID_ACCESS;
	}

	hService = OpenService (hManager, BLUEFISHER_DRIVER, SERVICE_ALL_ACCESS);

	if (hService != NULL)
	{
		SERVICE_STATUS ServiceStatus = {0};
		// Remove stale service (driver is not loaded but service exists)
		LOG(L"Driver is not loaded but the service exits.\n");
		
		QueryServiceStatus( hService, &ServiceStatus );
		
		if( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
		{
			return ERROR_SUCCESS;
		}
		DeleteService (hService);
		CloseServiceHandle (hService);
		Sleep (500);
	}
	//Clear the Error
	SetLastError(0);

	LOG( L"Installing Bluefisher driver...\n");
	LOG( L"Driver Path: %s\n", driverPath );

	//hService = CreateService (hManager, L"Bluefisher", L"Bluefisher",
	//	SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
	//	driverPath, NULL, NULL, NULL, NULL, NULL);
	hService = CreateService (hManager, BLUEFISHER_DRIVER, BLUEFISHER_DRIVER_DISP,
		SERVICE_ALL_ACCESS | GENERIC_EXECUTE | SERVICE_START | SERVICE_STOP, 
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
		driverPath, NULL, NULL, NULL, NULL, NULL);

	if (hService == NULL)
	{
		dwLastError = GetLastError();
		LOG(L"Bluefisher was not created. Error: %d.\n", dwLastError );
		SetLastError(0);
		CloseServiceHandle (hManager);
		return ERROR_INVALID_ACCESS;
	}

	CloseServiceHandle (hService);

	hService = OpenService( hManager, BLUEFISHER_DRIVER, SERVICE_ALL_ACCESS );

	if( hService == NULL )
	{
		dwLastError = GetLastError();
		LOG( L"Bluefisher was not opened. Error: %d.\n", dwLastError );
		SetLastError( 0 );
		CloseServiceHandle( hManager );
		return ERROR_INVALID_ACCESS;
	}

	if( !StartService (hService, 0, NULL) )
	{
		dwLastError = GetLastError();
		LOG(L"Bluefisher was not started. Error: %d.\n", dwLastError );
		SetLastError(0);
		CloseServiceHandle (hManager);
	}

	//DeleteService (hService);

	CloseServiceHandle (hManager);
	CloseServiceHandle (hService);

	dwLastError = GetLastError();
	if( dwLastError != 0 )
	{
		LOG(L"Error: %d.\n", dwLastError );
	}else
	{
		LOG(L"Bluefisher driver installed successfully.\n" );
	}

	return ERROR_SUCCESS;
}


int StartDriverService()
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
			
			LOG (L"ADMIN_PRIVILEGES_DRIVER No admin privileges of this driver.\n", 0);
			return ERROR_ACCESS_DENIED;
		}

		return ERROR_ACCESS_DENIED;
	}


	hService = OpenService (hManager, BLUEFISHER_DRIVER, SERVICE_ALL_ACCESS);

	if (hService == NULL)
	{
		LOG(L"Bluefisher was not opened. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
		return ERROR_INVALID_DRIVE;
	}

	if( !StartService (hService, 0, NULL) )
	{
		LOG(L"Bluefisher was not started. Error: %d.\n", GetLastError() );
		SetLastError(0);
		CloseServiceHandle (hManager);
	}

	//DeleteService (hService);

	CloseServiceHandle (hManager);
	CloseServiceHandle (hService);

	if( GetLastError() != 0 )
	{
		LOG(L"Error: %d.\n", GetLastError() );
	}else
	{
		LOG(L"Bluefisher driver was started successfully.\n" );
	}

	return !res ? ERROR_INVALID_DRIVE : ERROR_SUCCESS;
}

BOOL DriverUnload ()
{
	SC_HANDLE hManager = NULL, hService = NULL;
	BOOL bRet = FALSE;
	SERVICE_STATUS status = {0};
	int x = 0;
	DWORD dwLastError = 0;

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		goto error;

	hService = OpenService (hManager, BLUEFISHER_DRIVER, SERVICE_ALL_ACCESS | GENERIC_EXECUTE | SERVICE_START | SERVICE_STOP );
	if (hService == NULL)
	{
		dwLastError = GetLastError();
		LOG( L"Open Service for Bluefisher was failed. Error: %d.\n", dwLastError );
		SetLastError(0);
		goto error;
	}

	bRet = QueryServiceStatus (hService, &status);
	if (bRet != TRUE)
	{
		dwLastError = GetLastError();
		LOG( L"Query Service status failed. Error: %d.\n", dwLastError );
		SetLastError(0);
		goto error;
	}

	if (status.dwCurrentState != SERVICE_STOPPED)
	{
		if( !ControlService (hService, SERVICE_CONTROL_STOP, &status) )
		{
			DWORD dwErrorCode = GetLastError();
			LOG( L"CAN NOT STOP THE DRIVER OF 'Bluefisher' filter driver. Code: %d.\n ", dwErrorCode );
		}

		for (x = 0; x < 5; x++)
		{
			bRet = QueryServiceStatus (hService, &status);
			if (bRet != TRUE)
			{
				LOG( L"Query Service status failed. Loop %d. Error: %d.\n", x, GetLastError() );
				SetLastError(0);
				goto error;
			}
			
			if (status.dwCurrentState == SERVICE_STOPPED)
				break;

			Sleep (200);

			if( !ControlService (hService, SERVICE_CONTROL_STOP, &status) )
			{
				DWORD dwErrorCode = GetLastError();
				LOG( L"CAN NOT STOP THE DRIVER OF 'Bluefisher' filter driver. Code: %d. Try it gain: %d\n.", dwErrorCode, x );
			}
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

static int _getDigitalIdSize()
{
	return 34;
}

static int _getUnitIdSize()
{
	return 14;
}

static void DigitalIdPrint( unsigned char* hash, TCHAR* Buffer )
{
	_stprintf_s( Buffer, _getDigitalIdSize(), _T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
		hash[0], hash[1],hash[2],hash[3],hash[4],hash[5],hash[6],hash[7],
		hash[8], hash[9],hash[10],hash[11],hash[12],hash[13],hash[14],hash[15] );
}

static void UnitPrint( unsigned char* unit, TCHAR* Buffer )
{
	_stprintf_s( Buffer, _getUnitIdSize(), _T("%c%c%c%c%c%c"), 
		unit[0], unit[1],unit[2],unit[3],unit[4],unit[5]);
}

static void CopyKeyRequestToGingkoHeader( PrivateKeyRequest *request, GINGKO_HEADER *pHeader)
{
	pHeader->SizeOfBlock = request->SizeOfBlock;
	pHeader->SizeOfHeader = request->SizeOfHeader;
	pHeader->SizeOfFile = request->SizeOfFile;
	pHeader->Reserved = request->Reserved;
	memcpy( pHeader->Company, request->Company, COMPANY_LENGTH );
	memcpy( pHeader->Identifier, request->Identifier, GINGKO_IDENTIFIER_LENGTH );
	memcpy( pHeader->Md5, request->Md5, MD5_DIGEST_LENGTH );
	memcpy( pHeader->Method, request->Method, GINGKO_METHOD_LENGTH );
	memcpy( pHeader->Version, request->Version, GINGKO_VERSION_LENGTH );
	memset( pHeader->ReservedContent, 0, FILE_RESERVED_NORMAL );
	memcpy( pHeader->ReservedBytes, request->FileReserved, FILE_RESERVED_BYTES );
}

int QueryPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response )
{
	if( __gClientProcessId == request->dwProcessId )
	{
		response->Permission = 0x80000000 | MAKE_PERMISSION( FALSE, FALSE, FALSE, FALSE, FALSE, FALSE );
		return ERROR_SUCCESS;
	}

	

	TCHAR szUnitId[14] = {0};
	TCHAR szDigitalId[34] = {0};
	UnitPrint( request->Company, szUnitId );
	DigitalIdPrint( request->Md5, szDigitalId );

	if( COMPARE_SELF_DIGITAL(szDigitalId, szUnitId) )
	{
		unsigned char chBuf[65] = {0};
		int passwdLength = 0;
		int nTimes = 0;
		GINGKO_HEADER Header = {0};
		response->Permission = 0x80000000;

		CopyKeyRequestToGingkoHeader( request, &Header );

		do{
			if( GetDigitalPassword( szDigitalId, request->Filename, chBuf, 64, &passwdLength, request->dwProcessId ) )
			{
				if( GingkoBuildCryptoInfo( chBuf, passwdLength, response->CryptoKey, &Header ) )
				{
					response->Permission = 0x80000000 | MAKE_PERMISSION( TRUE, TRUE, TRUE, TRUE, TRUE, TRUE );
					return ERROR_SUCCESS;
				}
			}
			nTimes ++;
		}while( nTimes < 4 );
		response->Permission = 0x8000000F;
		return ERROR_ACCESS_DENIED;
	}else
	{
		WaitForDetectGingkoServer();
		if( IsSessionValid( NULL ) )
		{
			GingkoPermissionPack permission = {0};
			if( findDigitalPermission( szUnitId, szDigitalId, &permission ) == TRUE )
			{
				if( permission.readable | permission.printable )
				{
					LPVOID lpKey = Base64Decode( permission.publicKey );
					memcpy( response->CryptoKey, (unsigned char*)lpKey, 512 );
					FreeBase64Buffer( lpKey );
				}

				response->Permission = 0x80000000 | MAKE_PERMISSION( permission.owner, permission.holder, permission.readable, permission.writable, permission.printable, permission.deletable );
				return ERROR_SUCCESS;
			}
		}
	}

	return ERROR_INVALID_DATA;
}


int RenewPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response )
{
	return ERROR_INVALID_DATA;
}


BOOL InstallEventSource()
{
	TCHAR* logName = _T("Application");
	// Event Source name.
	TCHAR* sourceName = _T("GingkoSecuritySystem");
	// DLL that contains the event messages (descriptions).
	// This number of categories for the event source.
	DWORD dwCategoryNum = 1;
	HKEY hk; 
	DWORD dwData, dwDisp; 
	TCHAR szBuf[MAX_PATH]; 
	size_t cchSize = MAX_PATH;
	const TCHAR* szExeName = GetExecutablePath();

   // Create the event source as a subkey of the log. 
	_sntprintf_s( szBuf, cchSize, cchSize,
		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s"),
		logName, sourceName );

   HRESULT hr = S_OK; 

   if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuf, 
          0, NULL, REG_OPTION_NON_VOLATILE,
          KEY_WRITE, NULL, &hk, &dwDisp)) 
   {
      LOG(L"Could not create the registry key."); 
      return 0;
   }
 
   // Set the name of the message file. 
 
   if ( RegSetValueEx(hk,				// subkey handle 
          L"EventMessageFile",			// value name 
          0,							// must be zero 
          REG_EXPAND_SZ,				// value type 
          (LPBYTE) szExeName,				// pointer to value data 
          (DWORD) ( _tcslen(szExeName) + 1 ) * sizeof(TCHAR) ) ) // data size
   {
      LOG(L"Could not set the event message file."); 
      RegCloseKey(hk); 
      return 0;
   }
 
   // Set the supported event types. 
 
   dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
        EVENTLOG_INFORMATION_TYPE; 
 
   if (RegSetValueEx(hk,      // subkey handle 
           L"TypesSupported",  // value name 
           0,                 // must be zero 
           REG_DWORD,         // value type 
           (LPBYTE) &dwData,  // pointer to value data 
           sizeof(DWORD)))    // length of value data 
   {
      LOG(L"Could not set the supported types."); 
      RegCloseKey(hk); 
      return 0;
   }
 
   // Set the category message file and number of categories.

   if (RegSetValueEx(hk,              // subkey handle 
           L"CategoryMessageFile",     // value name 
           0,                         // must be zero 
           REG_EXPAND_SZ,             // value type 
           (LPBYTE) szExeName,          // pointer to value data 
           (DWORD) ( _tcslen( szExeName ) + 1 ) * sizeof( TCHAR ) ) ) // data size
   {
      LOG(L"Could not set the category message file."); 
      RegCloseKey(hk); 
      return 0;
   }
 
   if (RegSetValueEx(hk,            // subkey handle 
           L"CategoryCount",         // value name 
           0,                       // must be zero 
           REG_DWORD,               // value type 
           (LPBYTE) &dwCategoryNum, // pointer to value data 
           sizeof(DWORD)))          // length of value data 
   {
      LOG(L"Could not set the category count."); 
      RegCloseKey(hk); 
      return 0;
   }

   RegCloseKey(hk); 
   return 1;

}