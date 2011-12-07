#include "stdafx.h"
#include "cheapman.h"
#include "Crypto.h"

extern HANDLE hDriver;
HANDLE RequestEvent;


int CreatePrimaryKeyInMemory (	char *header, 
								int ea, 
								int mode,
								int pkcs5_prf, 
								PCRYPTO_INFO *retInfo);


BOOL CommitPrimaryKey(PrivateKeyRequest* pHeader, const unsigned char* primaryKey, int KeySize);

LPWSTR ConvertToGingkoFileId(PrivateKeyRequest* pHeader, LPWSTR Suffix);

BOOL LoadPrimaryKey(PrivateKeyRequest* pHeader, unsigned char* primaryKey, int KeySize);


BOOL DeviceIoSetRequestReadEvent(SharedNotification *notification)
{
	DWORD ret = 0;
	
	if( !notification )
	{
		return FALSE;
	}

	if( hDriver == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( hDriver == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}


	if( DeviceIoControl( hDriver, GINGKO_DEVICE_START, notification, sizeof(SharedNotification), NULL, 0, &ret, NULL ) )
	{
		return TRUE;
	}
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

	if( hDriver == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( hDriver == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	
	memset( request, 0 , sizeof(PrivateKeyRequest) + sizeof(WCHAR) * MAX_PATH );

	if( DeviceIoControl( hDriver, GINGKO_GET_REQUEST, NULL, 0, request, sizeof(PrivateKeyRequest) + sizeof(WCHAR) * MAX_PATH, &ret, NULL ) )
	{
		if( ret > 0 )
		{
			if( pkr != NULL )
			{
				*pkr = request;
				wprintf(L"The request file name: %s.\n", request->Filename );
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

	if( hDriver == INVALID_HANDLE_VALUE )
	{
		OpenDriver();
		if( hDriver == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Open driver failed with the error: %d\n", GetLastError());
			return FALSE;
		}
	}

	return DeviceIoControl( hDriver, GINGKO_PUT_RESPONSE, response, sizeof(PrivateKeyResponse), NULL, 0, &ret, NULL );
}

BOOL StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, PVOID ThreadArgs, ThreadBag **pBag )
{
	ThreadBag *bag = (ThreadBag*) malloc( sizeof(ThreadBag) );
	if( bag != NULL )
	{
		DWORD dwThreadId;
		bag->hEvent  = CreateEvent( NULL, FALSE, FALSE , NULL );
		if( bag->hEvent == NULL || bag->hEvent == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Create Event failed with error code: %d\n", GetLastError());
			free(bag);
			return FALSE;
		}

		bag->bThreadExit = FALSE;
		bag->ThreadArgs = ThreadArgs;
		bag->hThread = CreateThread( NULL, 0, ThreadRoutine, bag, CREATE_SUSPENDED, &dwThreadId ); 
		
		if( bag->hThread == NULL || bag->hThread == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Create Thread failed with error code: %d\n", GetLastError());
			free( bag );
			return FALSE;
		}

		if( pBag )
		{
			*pBag = bag;
		}

		ResumeThread( bag->hThread );

		return TRUE;
	}
	return FALSE;
}

BOOL StopThread(ThreadBag *bag)
{
	bag->bThreadExit = TRUE;
	SetEvent( bag->hEvent );
	WaitForSingleObject( bag->hThread, INFINITE );
	CloseHandle( bag->hEvent );
	CloseHandle( bag->hThread );
	return TRUE;
}

DWORD WINAPI DeviceIoThreadProc(PVOID threadArg)
{
	ThreadBag *bags = (ThreadBag*) threadArg;

	SharedNotification notification = {0};

	CoInitialize(NULL);

	notification.ReadEvent = bags->hEvent;

	SetLastError(0);

	if( DeviceIoSetRequestReadEvent(&notification) )
	{
		wprintf(L"The Driver was notified to started. The event object is %p\n", bags->hEvent );
		while( !bags->bThreadExit )
		{			
			DWORD dw = WaitForSingleObject( bags->hEvent, 3000 );
			
			if( WAIT_TIMEOUT == dw )
				continue;

			if( WAIT_OBJECT_0 == dw )
			{
				wprintf(L"Get a request from Kernel notification. %d==%d. Err: %d.\n", dw, WAIT_OBJECT_0, GetLastError() );
				PrivateKeyRequest *request = NULL;
				BOOL TryAgain = TRUE;

				do{
					if( DeviceIoGetPrivateKeyRequest( &request ) )
					{
						if( request != NULL )
						{
							wprintf(L"Got the Key Request.\n");
							TryAgain = TRUE;
							StartThread( RequestKeyThreadProc, request, NULL );
						}else{
							TryAgain = FALSE;
						}
					}else{ TryAgain = FALSE; }
				}while( TryAgain );
			}
			
		}
	}

	CoUninitialize();
	return 0L;
}

DWORD  WINAPI RequestKeyThreadProc(PVOID threadArg)
{
	ThreadBag *bag = (ThreadBag*) threadArg;
	if( bag == NULL )
	{
		return -1;
	}

	wprintf(L"Enter RequestKeyThreadproc.\n");
	PrivateKeyRequest *request = (PrivateKeyRequest*) bag->ThreadArgs;

	PrivateKeyResponse response = {0};

	if( request->Identifier[0] == 'G' &&
		request->Identifier[1] == 'K' &&
		request->Identifier[2] == 'T' &&
		request->Identifier[3] == 'F' )
	{
		///GET THE KEY FROM SERVER
		wprintf(L"Get the Key from server.\n");
		
		CopyRequestToResponse( request, &response );

		QueryPrimaryKey( request, &response );
		
	}
	else
	{
		RenewPrimaryKey( request, &response );
		
		CopyRequestToResponse( request, &response );
		response.Permission = 0x8000FFFF;
	}

	DeviceIoPutPrivateKeyResponse( request, &response );
	return 0;
}


void OpenDriver()
{
	if( hDriver == INVALID_HANDLE_VALUE )
	{
		hDriver = CreateFile( L"\\\\.\\Bluefisher", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hDriver == INVALID_HANDLE_VALUE )
		{
			wprintf(L"Open driver failed with the error: %d\n", GetLastError());
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
	_snwprintf( arrFileId, sizeof(arrFileId), 
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

