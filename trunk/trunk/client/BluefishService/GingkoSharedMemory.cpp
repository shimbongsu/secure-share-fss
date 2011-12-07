#include "stdafx.h"
#include <Softpub.h>
#include <wincrypt.h>
#include <Wintrust.h>
#include "GingkoSharedMemory.h"
#include "GingkoLibrary.h"
#include "cheapman.h"

#pragma comment (lib, "wintrust")

#define  GINGKO_MAPPING_FILE "GingkoIdentify.pid"
#define  GINGKO_PIPE_BUFFER_SIZE	8192
#define  GINGKO_READ_BUFFER_SIZE	1024

static HANDLE __g_MapHandle			= NULL;
static HANDLE __g_MapFileHandle		= NULL;
static HANDLE __g_PipeListenerThread= NULL;
static LPVOID __g_MapViewHandle		= NULL;
static TCHAR* szExecutablePath		= NULL;
HANDLE				__gClientSyncEventHandle		= NULL;
LPCTSTR				__gszNamedPipe		= GINGKO_PIPE_NAME;
BOOL				__gPipeRunning = FALSE;
ULONG				__gClientProcessId = 0L;
TCHAR				__g_SessionToken[GINGKO_READ_BUFFER_SIZE] = {0};
HANDLE				_gNotifyMessageMutex;
GingkoNotifyMessage _gGlobalNotifyMessage;

DWORD WINAPI ConnectedInstanceThread(LPVOID lpvParam);

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
	GetProcessBaseName( pNotify->dwProcessId, szFmtExefile, MAX_PATH );
	FormatPermission( szFmtPerm, MAX_PATH, pNotify->dwPermission );
	_stprintf_s( lpszMessage, MaxSize, szFmt, szFmtExefile, pNotify->szFileName, szFmtPerm );
	return MaxSize;
}

const TCHAR* GetExecutablePath()
{
	if( szExecutablePath == NULL )
	{
		DWORD dwActualSize = MAX_PATH * 2 * sizeof(TCHAR);
		TCHAR *szFilePath = (TCHAR*) malloc( dwActualSize );
		if( szFilePath != NULL )
		{
			memset( szFilePath, 0, dwActualSize ); 

			DWORD dwSize = GetModuleFileName( NULL, szFilePath, 2 * MAX_PATH - 1 );
			if( dwSize > 0 )
			{
				while( dwSize -- )
				{
					if( szFilePath[dwSize] == _T('\\') )
					{
						break;
					}
				}
				szExecutablePath = (TCHAR*) malloc( sizeof(TCHAR) * (dwSize + 1) );
				
				if( szExecutablePath != NULL )
				{
					memset( szExecutablePath, 0, ( dwSize + 1 ) * sizeof(TCHAR) );
					memcpy( szExecutablePath, szFilePath, dwSize * sizeof(TCHAR) );
				}
			}
			free( szFilePath );
		}
	}

	return szExecutablePath;
}


static BOOL _loadConfig()
{
	BOOL bret = FALSE;
	TCHAR* szConfPath = NULL;
	GingkoOptions *pOptions = NULL;
	const TCHAR* tszPath  = GetExecutablePath();
	BOOL blang = FALSE;
	if( tszPath != NULL )
	{
		size_t sz = sizeof(TCHAR) * (_tcslen( tszPath ) + 40);
		szConfPath = (TCHAR*) malloc( sz );
		if( szConfPath != NULL )
		{
			memset( szConfPath, 0, sz );
			_sntprintf_s( szConfPath, sz / sizeof(TCHAR), sz, _T("%s\\%s"), tszPath, GINGKO_CONFIG_FILE );
			bret = ParseConfigFile( szConfPath );
			GetGingkoOptions( &pOptions );
			if( pOptions != NULL )
			{

				if( pOptions->Language != NULL )
				{
					memset( szConfPath, 0, sz );
					_sntprintf_s( szConfPath, sz / sizeof(TCHAR), sz, _T("%s\\Language_%s.xml"), tszPath, pOptions->Language );
					blang = TRUE;
				}
			}

			if( !blang )
			{
				memset( szConfPath, 0, sz );
				_sntprintf_s( szConfPath, sz / sizeof(TCHAR), sz, _T("%s\\Language.xml"), tszPath );
				blang = TRUE;
			}

			
			ParseLanguageFile( szConfPath );
			
			free( szConfPath );
		}
	}
	return bret;
}

BOOL InitialServiceUrl()
{
	USES_CONVERSION;
	GingkoOptions *pOptions = NULL;
	GetGingkoOptions( &pOptions );

	if( pOptions != NULL )
	{
		addGingkoServiceUrl(T2A(pOptions->MasterServer));
		addGingkoServiceUrl(T2A(pOptions->UnitServer));
	}

	return TRUE;
}

BOOL  WINAPI GkoInitialize()
{
	if( !_loadConfig() )
	{
		GkoUnInitialize();
		return FALSE;
	}

	return TRUE;
	//return TRUE;
}

BOOL  WINAPI GkoUnInitialize()
{
	if( szExecutablePath != NULL )
	{
		free( szExecutablePath );
	}

	if( __g_MapFileHandle )
	{
		CloseHandle( __g_MapFileHandle );
	}

	
	FreeConfig();

	return TRUE;
}

DWORD WINAPI DetectGingkoServerThread( LPVOID lpCaller )
{
	return ::DetectGingkoServer();
}


static BOOL ReplyRequest( LPVOID szRequest, LPTSTR szReplayBuf, DWORD* cch)
{
	GingkoCommand* pCmd = (GingkoCommand*) szRequest;
	if( pCmd->dwSize >= pCmd->dwBufSize + sizeof(GingkoCommand) - 1 )
	{
		if( *cch < pCmd->dwBufSize )
		{
			return FALSE;
		}

		if( _tcscmp( pCmd->szCommand, _T("GetSession:") ) == 0 )
		{
			if( GINGKO_READ_BUFFER_SIZE <= (*cch) )
			{
				memcpy( szReplayBuf, __g_SessionToken, GINGKO_READ_BUFFER_SIZE );
				*cch = (DWORD)((_tcslen( __g_SessionToken ) + 1 )* sizeof(TCHAR)); 
				return TRUE;
			}
		}else if(_tcscmp( pCmd->szCommand, _T("PutSession:") ) == 0)
		{
			DWORD dwSize = (GINGKO_READ_BUFFER_SIZE * sizeof(TCHAR)) ;
			dwSize = dwSize > pCmd->dwBufSize ? pCmd->dwBufSize : dwSize;
			memcpy( __g_SessionToken, pCmd->szCommandBody, dwSize);
			if( szReplayBuf != NULL )
			{
				_stprintf_s( szReplayBuf, (*cch / sizeof(TCHAR)), _T("OK") );
				*cch = (DWORD)((_tcslen( szReplayBuf ) + 1 )* sizeof(TCHAR)); 
			}
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("Logout:") ) == 0)
		{
			clearSession();
			memset( __g_SessionToken, 0, GINGKO_READ_BUFFER_SIZE  * sizeof(TCHAR) );
			if( szReplayBuf != NULL )
			{
				_stprintf_s( szReplayBuf, (*cch / sizeof(TCHAR)) , _T("OK") );
				*cch = (DWORD)((_tcslen( szReplayBuf ) + 1 )* sizeof(TCHAR)); 
			}
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("__ReloadConfig:") ) == 0)
		{
			_loadConfig();
			ThreadBag *pbag = NULL;
			StartThread( DetectGingkoServerThread, NULL, &pbag );
			//memset( __g_SessionToken, 0, GINGKO_READ_BUFFER_SIZE  * sizeof(TCHAR) );
			if( szReplayBuf != NULL )
			{
				_stprintf_s( szReplayBuf, (*cch / sizeof(TCHAR)), _T("OK") );
				*cch = (DWORD)((_tcslen( szReplayBuf ) + 1 )* sizeof(TCHAR)); 
			}
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("SetClientProcessId:") ) == 0)
		{
			__gClientProcessId = _ttol( pCmd->szCommandBody );
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("GetPermission:") ) == 0)
		{
			ULONG uProcessId = (ULONG)_ttol( pCmd->szCommandBody );
			ULONG Permission = 0L;
			BOOL dwRet = DeviceIoQueryPermission( uProcessId, &Permission );
			if( dwRet == TRUE )
			{
				_stprintf_s( szReplayBuf, (*cch / sizeof(TCHAR)), _T("%d"), Permission );
				*cch = (DWORD)((_tcslen( szReplayBuf ) + 1 )* sizeof(TCHAR)); 
			}
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("__InternalStopServer:") ) == 0)
		{
			ULONG dwCmdProcessId = _ttol( pCmd->szCommandBody );
			
			if( dwCmdProcessId == GetCurrentProcessId() )
			{
				__gPipeRunning = FALSE;
			}

			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("__SendDecryptPassword:") ) == 0)
		{
			GingkoPassword *gpwd = (GingkoPassword*) pCmd->szCommandBody;
			
			if( InsertGingkoPassword( gpwd->FileHash, gpwd->PasswordText, gpwd->dwPasswordLength, gpwd->dwProcessId ) )
			{
				SetEvent( __gPasswordEventHandle );
			}
			return TRUE;
		}else if(_tcscmp( pCmd->szCommand, _T("ReadNotifyMessage:") ) == 0)
		{
			WaitForSingleObject( _gNotifyMessageMutex, INFINITE );
			GingkoNotifyMessage *gnm = &_gGlobalNotifyMessage;
			if( gnm )
			{
				if( *cch >= sizeof(GingkoNotifyMessage) )
				{
					memcpy( szReplayBuf, gnm, sizeof(GingkoNotifyMessage) );
					*cch = sizeof( GingkoNotifyMessage );
				}else
				{
					*cch = 0;
				}
			}
			ReleaseMutex( _gNotifyMessageMutex );
			return TRUE;
		}
	}
	return FALSE;
}

PSECURITY_DESCRIPTOR GkoInitializeSecurityDescriptor(SECURITY_ATTRIBUTES*	pSa)
{
	PSECURITY_DESCRIPTOR	pSD;
                                       // create a security NULL security
                                       // descriptor, one that allows anyone
                                       // to write to the pipe... WARNING
                                       // entering NULL as the last attribute
                                       // of the CreateNamedPipe() will
                                       // indicate that you wish all
                                       // clients connecting to it to have
                                       // all of the same security attributes
                                       // as the user that started the
                                       // pipe server.
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(	LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );
	
	if( pSD == NULL )
	{
		return NULL;
	}

	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		LocalFree((HLOCAL)pSD);
		return NULL;
	}
	// add a NULL disc. ACL to the
	// security descriptor.
	if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE))
	{
			LocalFree((HLOCAL)pSD);
			return NULL;
	}

	pSa->nLength = sizeof(SECURITY_ATTRIBUTES);

	pSa->lpSecurityDescriptor = pSD;
	
	pSa->bInheritHandle = FALSE;
	
	return pSD;
}

static DWORD WINAPI PipeListenerServer( LPVOID lpData )
{
	BOOL fConnected = FALSE;
	HANDLE hPipe = NULL; 
	HANDLE hThread = NULL;
	SECURITY_ATTRIBUTES sa;
	PSECURITY_DESCRIPTOR pSD = NULL;
	pSD = GkoInitializeSecurityDescriptor( &sa );
	if( pSD == NULL )
	{
		return 0;
	}

	while( __gPipeRunning ) 
	{
		hPipe = CreateNamedPipe( 
			__gszNamedPipe,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			GINGKO_PIPE_BUFFER_SIZE,  // output buffer size 
			GINGKO_PIPE_BUFFER_SIZE,  // input buffer size 
			0,                        // client time-out 
			&sa );                    // default security attribute 

      if ( hPipe == INVALID_HANDLE_VALUE )
      {
			if( pSD != NULL )
			{
				LocalFree( pSD );
			}
          return 0;
      }
  
		do{
			fConnected = ConnectNamedPipe(hPipe, NULL) ?  TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

			if( fConnected ) 
			{
				// Create a thread for this client. 
				DWORD dwThreadId = 0;
				hThread = CreateThread( 
									NULL,              // no security attribute 
									0,                 // default stack size 
									ConnectedInstanceThread,    // thread proc
									(LPVOID) hPipe,    // thread parameter 
									0,                 // not suspended 
									&dwThreadId);      // returns thread ID 

				if (hThread == NULL) 
				{
					LOG(L"CreateThread Faild. Command Service will exit.\n"); 
					break;
				}
				//WaitForSingleObject( hThread, INFINITE );
				CloseHandle(hThread); 
			}

			if( !__gPipeRunning )
			{
				DisconnectNamedPipe( hPipe );
				CloseHandle( hPipe );
				break;
			}

			if( __g_PipeListenerThread != INVALID_HANDLE_VALUE && __g_PipeListenerThread != NULL )
			{
				WaitForSingleObject( __g_PipeListenerThread, 100 );
			}

		}while( fConnected == FALSE );

	  //	CloseHandle(hPipe); 
		
	}

	CloseHandle( hPipe );

	if( pSD != NULL )
	{
		LocalFree( pSD );
	}

	return 0;
}

BOOL PipeListenerServerThreadStart()
{
	DWORD dwThreadId = 0;
	__gPipeRunning = TRUE;
	if( __g_PipeListenerThread == NULL )
	{
		__g_PipeListenerThread = CreateThread( 
					NULL,              // no security attribute 
					0,                 // default stack size 
					PipeListenerServer,// thread proc
					(LPVOID) NULL,     // thread parameter 
					0,                 // not suspended 
					&dwThreadId);      // returns thread ID 
		if( __g_PipeListenerThread != NULL )
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL PipeListenerServerThreadStop()
{
	if( __g_PipeListenerThread != NULL )
	{
		__gPipeRunning = FALSE;
		do{
			NotifyStopServer();
		}while( WaitForSingleObject( __g_PipeListenerThread, 500 ) == WAIT_TIMEOUT );

		CloseHandle( __g_PipeListenerThread );
		return TRUE;
	}
	return FALSE;
}


DWORD WINAPI ConnectedInstanceThread(LPVOID lpvParam) 
{ 
	TCHAR chRequest[GINGKO_READ_BUFFER_SIZE]; 
	TCHAR chReply[GINGKO_READ_BUFFER_SIZE]; 
	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0; 
	BOOL fSuccess = FALSE;  
	HANDLE hPipe; 
 
// The thread's parameter is a handle to a pipe instance. 
 
	hPipe = (HANDLE) lpvParam; 
 
	fSuccess = ReadFile( 
	 hPipe,        // handle to pipe 
	 chRequest,    // buffer to receive data 
	 GINGKO_READ_BUFFER_SIZE * sizeof(TCHAR), // size of buffer 
	 &cbBytesRead, // number of bytes read 
	 NULL);        // not overlapped I/O 

	if (! fSuccess || cbBytesRead == 0) 
		goto __exit; 

	cbReplyBytes = GINGKO_READ_BUFFER_SIZE * sizeof(TCHAR);

	if( ReplyRequest( chRequest, chReply, &cbReplyBytes ) )
	{
	  fSuccess = WriteFile( 
		 hPipe,        // handle to pipe 
		 chReply,      // buffer to write from 
		 cbReplyBytes, // number of bytes to write 
		 &cbWritten,   // number of bytes written 
		 NULL);        // not overlapped I/O 
	}
 
// Flush the pipe to allow the client to read the pipe's contents 
// before disconnecting. Then disconnect the pipe, and close the 
// handle to this pipe instance. 
 
__exit:

   FlushFileBuffers(hPipe); 
   DisconnectNamedPipe(hPipe); 
   CloseHandle(hPipe); 

   return 1;
}



BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    LONG lStatus;
    DWORD dwLastError = 0;

    // Initialize the WINTRUST_FILE_INFO structure.

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
    
    1) The certificate used to sign the file chains up to a root 
    certificate located in the trusted root certificate store. This 
    implies that the identity of the publisher has been verified by 
    a certification authority.
    
    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the  
    end entity certificate is stored in the trusted publisher store,  
    implying that the user trusts content from this publisher.
    
    3) The end entity certificate has sufficient permission to sign 
    code, as indicated by the presence of a code signing EKU or no 
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;

    // Initialize the WinVerifyTrust input data structure.

    // Default all fields to 0.
    memset(&WinTrustData, 0, sizeof(WinTrustData));

    WinTrustData.cbStruct = sizeof(WinTrustData);
    
    // Use default code signing EKU.
    WinTrustData.pPolicyCallbackData = NULL;

    // No data to pass to SIP.
    WinTrustData.pSIPClientData = NULL;

    // Disable WVT UI.
    WinTrustData.dwUIChoice = WTD_UI_NONE;

    // No revocation checking.
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

    // Verify an embedded signature on a file.
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

    // Default verification.
    WinTrustData.dwStateAction = 0;

    // Not applicable for default verification of embedded signature.
    WinTrustData.hWVTStateData = NULL;

    // Not used.
    WinTrustData.pwszURLReference = NULL;

    // Default.
    WinTrustData.dwProvFlags = WTD_SAFER_FLAG;

    // This is not applicable if there is no UI because it changes 
    // the UI to accommodate running applications instead of 
    // installing applications.
    WinTrustData.dwUIContext = 0;

    // Set pFile.
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID 
    // and Wintrust_Data.
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus) 
    {
        case ERROR_SUCCESS:
            /*
            Signed file:
                - Hash that represents the subject is trusted.

                - Trusted publisher without any verification errors.

                - UI was disabled in dwUIChoice. No publisher or 
                    time stamp chain errors.

                - UI was enabled in dwUIChoice and the user clicked 
                    "Yes" when asked to install and run the signed 
                    subject.
            */
			return TRUE;
        case TRUST_E_NOSIGNATURE:
            // The file was not signed or had a signature 
            // that was not valid.

            // Get the reason for no signature.
            //dwLastError = GetLastError();
            //if (TRUST_E_NOSIGNATURE == dwLastError ||
            //        TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
            //        TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
            //{
            //     The file was not signed.
            //    wprintf_s(L"The file \"%s\" is not signed.\n",
            //        pwszSourceFile);
            //} 
            //else 
            //{
            //     The signature was not valid or there was an error 
            //     opening the file.
            //    wprintf_s(L"An unknown error occurred trying to "
            //        L"verify the signature of the \"%s\" file.\n",
            //        pwszSourceFile);
            //}
			return FALSE;
            //break;

        case TRUST_E_EXPLICIT_DISTRUST:
            // The hash that represents the subject or the publisher 
            // is not allowed by the admin or user.
            //wprintf_s(L"The signature is present, but specifically "
            //    L"disallowed.\n");
			return FALSE;
            //break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
            // The user clicked "No" when asked to install and run.
            //wprintf_s(L"The signature is present, but not "
            //    L"trusted.\n");
            return TRUE;
        case CRYPT_E_SECURITY_SETTINGS:
            /*
            The hash that represents the subject or the publisher 
            was not explicitly trusted by the admin and the 
            admin policy has disabled user trust. No signature, 
            publisher or time stamp errors.
            */
            //wprintf_s(L"CRYPT_E_SECURITY_SETTINGS - The hash "
            //    L"representing the subject or the publisher wasn't "
            //    L"explicitly trusted by the admin and admin policy "
            //    L"has disabled user trust. No signature, publisher "
            //    L"or timestamp errors.\n");
			return TRUE;
            //break;

        default:
            // The UI was disabled in dwUIChoice or the admin policy 
            // has disabled user trust. lStatus contains the 
            // publisher or time stamp chain error.
            //wprintf_s(L"Error is: 0x%x.\n",
            //    lStatus);
            break;
    }

    return FALSE;
}


DWORD NotifyProcessEvent( LPCTSTR szFileName, ULONG dwProcessId, ULONG dwPermission )
{
	WaitForSingleObject( _gNotifyMessageMutex, 10 );
	GetProcessBaseName( dwProcessId, _gGlobalNotifyMessage.ProcessName, MAX_PATH - 1 );
	_gGlobalNotifyMessage.dwProcessId = dwProcessId;
	_gGlobalNotifyMessage.dwPermission = dwPermission;
	_gGlobalNotifyMessage.dwFileNameLength = _tcslen( szFileName );
	_tcscpy_s( _gGlobalNotifyMessage.szFileName, MAX_PATH - 1, szFileName );

	if( _gNotifyMessageMutex != NULL )
	{
		ReleaseMutex( _gNotifyMessageMutex );
	}

	if( __gClientSyncEventHandle != NULL )
	{
		SetEvent( __gClientSyncEventHandle );
	}

	return 0;
}

