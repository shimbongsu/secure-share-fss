// GingkoLibrary.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GingkoLibrary.h"
#include <list>
#include "soapGingkoLoginSoapServiceBeanServiceSoapBindingProxy.h"
#include "random.h"
#include "crypto.h"
#include "md5.h"
#include "GingkoEncoding.h"

#define GINGKO_LOGIN_SERVICE	"GingkoLoginService"
#define GINGKO_UNIT_SERVICE		"GingkoUnitService"
#define GINGKO_USER_SERVICE		"GingkoUserService"
#define GINGKO_DIGITAL_SERVICE	"GingkoDigitalService"
#define GINGKO_TOKEN_NAME		"GINGKO_TOKEN" ///Cookie name

#define CALLER_UNKOWN_ERROR		_T("Unkown Error")
#define GINGKO_MESSAGE_SIZE		1024
#define BYTES_OF_SECTOR			512
#define NUMBER_OF_SECTORS		512
#define READ_FILE_BUFFER		8 * 1024

static char* LoginServiceEndpoint = NULL;
static char* UnitServiceEndpoint = NULL;
static char* UserServiceEndpoint = NULL;
static char* DigitalServiceEndpoint = NULL;
static std::vector<const char*> servers;
static BOOL  bUnderDetect = FALSE;

DWORD  dwGingkoTlsIndex = 0;

//TCHAR* GingkoToken = NULL;

char*  _gingkoToken = NULL;

GingkoUserPack _gingkoUserPack = {0};

static HANDLE		  _g_SessionFileMap = NULL;
static GingkoSession* _gingkoSession = NULL;

FILE* hLogFile = NULL;
HANDLE hLogMutex = NULL;

HANDLE _g_DetectServerMutex = NULL;

typedef struct __ThreadMessagePack{
	TCHAR Message[GINGKO_MESSAGE_SIZE];
	INT	  Status;
} ThreadMessagePack;

typedef struct __ConnectionOptions
{
	bool	_enable_proxy;
	int		_locked;
	HANDLE	_wait_handler;
	char*	_proxy_host;
	int		_proxy_port;
	char*	_proxy_userid;
	char*	_proxy_passwd;
	int		_send_timeout;
	int		_recv_timeout;
	int		_conn_timeout;
}ConnectionOptions;

extern "C" int CreatePrimaryKeyInMemoryWithPassword(	char *header, 
								int ea, 
								int mode,
								int pkcs5_prf, 
								char* szPasswd,
								int PasswordLength,
								PCRYPTO_INFO *retInfo);

static ConnectionOptions __ThisConnectionOptions = {0};

static BOOL OpenGingkoSession()
{
	_g_DetectServerMutex = CreateMutex( NULL, FALSE, NULL );
	
	if( _g_DetectServerMutex == NULL )
	{
		return FALSE;
	}

    if (( dwGingkoTlsIndex = TlsAlloc() ) == TLS_OUT_OF_INDEXES) 
        return FALSE; 	

	_gingkoSession = (GingkoSession*) malloc (sizeof( GingkoSession) );
	if( _gingkoSession != NULL )
	{
		_gingkoSession->dwSize = sizeof( GingkoSession );
	}
	
	//_g_SessionFileMap = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, GINGKO_SHARED_MEMORY_IDENTIFY );

	//DWORD dwError = 0;
	//if( _g_SessionFileMap == NULL )
	//{
	//	dwError = GetLastError();
	//	return TRUE;
	//}

	//_gingkoSession = (GingkoSession*) MapViewOfFile(_g_SessionFileMap, // handle to map object
	//					FILE_MAP_ALL_ACCESS,  // read/write permission
	//					0,
	//					0,
	//					FILE_MAPPING_SIZE);

	//if( _gingkoSession == NULL )
	//{
	//	dwError = GetLastError();
	//	return TRUE;
	//}

	return TRUE;
}


static BOOL CloseGingkoSession()
{
	LPVOID lpvData = NULL; 
    lpvData = TlsGetValue(dwGingkoTlsIndex); 

    if (lpvData != NULL) 
        LocalFree((HLOCAL) lpvData); 

    // Release the TLS index.
    TlsFree( dwGingkoTlsIndex ); 

	if( _gingkoToken )
	{
		free( _gingkoToken );
		_gingkoToken = NULL;
	}
            
	if( _gingkoSession )
	{
		free( _gingkoSession );
		_gingkoSession = NULL;
		//UnmapViewOfFile( _gingkoSession );
	}

	if( _g_SessionFileMap )
	{
		
		CloseHandle( _g_SessionFileMap );
	}
	return TRUE;
}


#ifdef GINGKO_LIBRARY_DLL
BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // DLL module handle
    DWORD fdwReason,                     // reason called
    LPVOID lpvReserved)                  // reserved
{ 
    BOOL fIgnore; 
	LPVOID lpvData;
    switch (fdwReason) 
    {
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary. 
 
        case DLL_PROCESS_ATTACH: 
            // Allocate a TLS index.
			LOG_START();
			if( !OpenGingkoSession() )
			{
				CloseGingkoSession();
				return FALSE;
			}

			//LOG( L"Process Attach CURRENT PROCESS ID: %d\n", GetCurrentProcessId() );

            // No break: Initialize the index for first thread.
			// The attached process creates a new thread.  
        case DLL_THREAD_ATTACH: 
 
            // Initialize the TLS index for this thread.
            lpvData = (LPVOID) LocalAlloc(LPTR, sizeof(ThreadMessagePack) ); 
            if (lpvData != NULL)
			{
                fIgnore = TlsSetValue(dwGingkoTlsIndex, lpvData); 
			}
			//LOG( L"Thread Attach CURRENT PROCESS ID: %d\n", GetCurrentProcessId() );
            break; 
        // The thread of the attached process terminates.
        case DLL_THREAD_DETACH: 
            // Release the allocated memory for this thread.
            lpvData = TlsGetValue(dwGingkoTlsIndex); 
            if( lpvData != NULL ) 
                LocalFree( (HLOCAL) lpvData ); 
			TlsSetValue( dwGingkoTlsIndex, NULL );

            //LOG( L"Thread Detach CURRENT PROCESS ID: %d\n", GetCurrentProcessId() );
			break; 
 
        // DLL unload due to process termination or FreeLibrary. 
 
        case DLL_PROCESS_DETACH: 
 
            // Release the allocated memory for this thread.
			CloseGingkoSession();
			LOG( L"Process Detach CURRENT PROCESS ID: %d\n", GetCurrentProcessId() );
			LOG_SHUTDOWN();
			break;
        default: 
            break; 
    }
 
    return TRUE; 
    UNREFERENCED_PARAMETER(hinstDLL); 
    UNREFERENCED_PARAMETER(lpvReserved); 
}
#endif

#ifdef GINGKO_LIBRARY_NONE

int  main(int argc, _TCHAR* argv[])
{
	int major = 0;
	int middle = 0;
	int minor = 0;
	SoapStatusEnum status;
	const TCHAR *pErrorMsg = NULL;
	if ((dwGingkoTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
	{
		  printf("TlsAlloc failed"); 
		  exit(1);
	}

#if 1
	addGingkoServiceUrl( "http://localhost:8080/rsm/gkws/" );

	DetectGingkoServer();

	GingkoVersion( &major, &middle, &minor, NULL );
	status = GetLastServiceStatus( &pErrorMsg );

	printf( "Gingko Version: %d.%d.%d, Status: %d, msg: %S.\n", major, middle, minor, status, pErrorMsg );
	printf( "Attempt to login:\n" );
	gingkoLogin( _T("longzou@bamboo.net"), _T("poethexp") );

	status = GetLastServiceStatus( &pErrorMsg );
	printf( "Login %S. The token is %S\n", pErrorMsg, GingkoToken );

	TCHAR *pNewGingkoId = NULL;
	newGingkoId( &pNewGingkoId );
	if( pNewGingkoId != NULL )
	{
		printf( "NewGingkoId is %S\n", pNewGingkoId );
		free( pNewGingkoId );
		pNewGingkoId = NULL;
	}

	BOOL ret = checkToken( GingkoToken );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("Check the current token with a return a status %d, %S.\n", status, pErrorMsg );

	if( ret )
	{
		printf("Check the current token was successfully. \n");
	}

	registerUser( _T("test10@bamboo.net"), _T("Xiaolin Gong"), _T("poetgxl"), _T(""), _T("") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("Register a User with return a status %d, %S.\n", status, pErrorMsg );

	////test digital soap service
	printf("Now, we are going to testing the GingkoDigitalService.\n");

	createDigitalInfo( _T("Long"), _T("description of this digital"), _T("DC0000010000000011"), 
		_T("acisdladfkdsfas0d-dsfasdfad"),
		_T("The title of this digital"), _T("Cookie"), 
		_T("asdflonadsf92323.adfad"), _T("Enable"), _T("PL"),
		_T("SAA"), _T("WW"), _T("aaaa"), _T("bbbbbbb") );

	status = GetLastServiceStatus( &pErrorMsg );
	printf("CreateDigitalInfo return a status %d, %S.\n", status, pErrorMsg );
	
	updateDigitalInfo( _T("DC0000010000000011"), _T("Long"), _T("description"), _T("title"), _T("Keyword"), 
		_T("Enable"), _T("PL"), _T("SAA"), _T("WW") );

	status = GetLastServiceStatus( &pErrorMsg );
	printf("UpdateDigitalInfo return a status %d, %S.\n", status, pErrorMsg );

	assignPermission( _T("000000"), _T("DC0000010000000011"), _T("sabrina@bamboo.net"), _T(""), TRUE, TRUE, TRUE, TRUE, TRUE, TRUE );

	status = GetLastServiceStatus( &pErrorMsg );
	printf("AssignPermission return a status %d, %S.\n", status, pErrorMsg );

	findDigitalInfo( _T("000000"), _T("DC0000010000000011") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("FindDigitalInfo return a status %d, %S.\n", status, pErrorMsg );

	findDigitalPermission( _T("000000"), _T("DC0000010000000011") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("FindDigitalPermission return a status %d, %S.\n", status, pErrorMsg );

	findAssignedPermission( _T("000000"), _T("DC0000010000000011") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("FindAssignedPermission return a status %d, %S.\n", status, pErrorMsg );

	deleteDigitalInfo( _T("000000"), _T("DC0000010000000011") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("DeleteDigitalInfo return a status %d, %S.\n", status, pErrorMsg );

	printf("End of the GingkoDigitalService.\n");

	////test user soap service
	printf("Now, we are going to testing the GingkoUserService.\n");
	
	changePassword( _T("longzou@bamboo.net"), _T("poethexp"), _T("poethexp") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("ChangePassword return a status %d, %S.\n", status, pErrorMsg );
	changePassword( _T("longzou@bamboo.net"), _T("poethexp"), _T("poetgxl") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("ChangePassword return a status %d, %S.\n", status, pErrorMsg );
	changePassword( _T("longzou@bamboo.net"), _T("poetgxl"), _T("poethexp") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("ChangePassword return a status %d, %S.\n", status, pErrorMsg );
	changePassword( _T("sabrina@bamboo.net"), _T("poethexp"), _T("poethexp") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("ChangePassword return a status %d, %S.\n", status, pErrorMsg );
	
	findUserByGingkoId( _T("0e_oLSA000000000") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("FindUserByGingkoId return a status %d, %S.\n", status, pErrorMsg );
	
	findUserByLoginId( _T("sabrina@bamboo.net") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("FindUserByLoginId return a status %d, %S.\n", status, pErrorMsg );
	
	
	bindingGingkoUser( _T("testing@bamboo.net"), _T("poethexp"), _T("Tester") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("BindingGingkoUser return a status %d, %S.\n", status, pErrorMsg );

	bindingGingkoUser( _T("longzou@bamboo.net"), _T("poethexp"), _T("Long Zou") );
	status = GetLastServiceStatus( &pErrorMsg );
	printf("BindingGingkoUser return a status %d, %S.\n", status, pErrorMsg );

	printf("End of the GingkoUserService.\n");
#endif
	printf("Now, we are going to test encrypt file.\n");
	HANDLE hFile = CreateFile( _T("D:\\test.pdf"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile != NULL && hFile != INVALID_HANDLE_VALUE )
	{
		HANDLE hOutputFile = CreateFile( _T("D:\\test_out.pdf"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hOutputFile != NULL && hOutputFile != INVALID_HANDLE_VALUE )
		{
			GingkoDigitalPack pack = {0};
			if( GingkoEncryptFile( hFile, hOutputFile, &pack ) )
			{
				printf("Filehash:  %S.\n", pack.fileHash );
				printf("DigitalId: %S.\n", pack.digitalId );
				if( pack.digitalId != NULL )
				{
					free( pack.digitalId );
				}

				if( pack.fileHash != NULL )
				{
					free( pack.fileHash );
				}
			}else
			{
				status = GetLastServiceStatus( &pErrorMsg );
				printf("GingkoEncryptFile return a status %d, %S.\n", status, pErrorMsg );
			}
			CloseHandle( hOutputFile );
		}
		CloseHandle( hFile );
	}

	printf("End of Encryption testing.\n");


	TlsFree( dwGingkoTlsIndex );

	return 0;
}

#endif




static void TlsSaveSoapMessage( const TCHAR* Message, int status )
{
	ThreadMessagePack* pack = (ThreadMessagePack*) TlsGetValue( dwGingkoTlsIndex );
	
	if( pack == NULL )
	{
		pack =(ThreadMessagePack*) LocalAlloc( LPTR,sizeof(ThreadMessagePack) );
	}

	if( pack != NULL )
	{
		memset( pack->Message, 0, GINGKO_MESSAGE_SIZE * sizeof(TCHAR) );
		if( Message != NULL )
		{
			_tcscpy_s( pack->Message, GINGKO_MESSAGE_SIZE, Message );
		}
		pack->Status = status;
	}

	TlsSetValue( dwGingkoTlsIndex, pack );
}

static const char* GetLoginServiceEndpoint()
{
	return LoginServiceEndpoint;
}

static const char* GetUserServiceEndpoint()
{
	return UserServiceEndpoint;
}

static const char* GetUnitServiceEndpoint()
{
	return UnitServiceEndpoint;
}

static const char* GetDigitalServiceEndpoint()
{
	return DigitalServiceEndpoint;
}

static void setTheCallingSiteUrl( const char* pSiteUrl )
{
	if( pSiteUrl != NULL )
	{
		char* pEndpoint = (char*) malloc( sizeof(char) * GINGKO_MESSAGE_SIZE );
		if( pEndpoint != NULL )
		{
			memset( pEndpoint, 0, sizeof(char) * GINGKO_MESSAGE_SIZE );
			if( UserServiceEndpoint != NULL )
			{
				free( UserServiceEndpoint );
			}
			_snprintf_s( pEndpoint, GINGKO_MESSAGE_SIZE, GINGKO_MESSAGE_SIZE, "%s%s", pSiteUrl,GINGKO_USER_SERVICE );
			UserServiceEndpoint = _strdup( pEndpoint );

			memset( pEndpoint, 0, sizeof(char) * GINGKO_MESSAGE_SIZE );
			if( LoginServiceEndpoint != NULL )
			{
				free( LoginServiceEndpoint );
			}
			_snprintf_s( pEndpoint, GINGKO_MESSAGE_SIZE, GINGKO_MESSAGE_SIZE, "%s%s", pSiteUrl,GINGKO_LOGIN_SERVICE );
			LoginServiceEndpoint = _strdup( pEndpoint );
			
			memset( pEndpoint, 0, sizeof(char) * GINGKO_MESSAGE_SIZE );

			if( UnitServiceEndpoint != NULL )
			{
				free( UnitServiceEndpoint );
			}
			_snprintf_s( pEndpoint, GINGKO_MESSAGE_SIZE, GINGKO_MESSAGE_SIZE, "%s%s", pSiteUrl,GINGKO_UNIT_SERVICE );
			UnitServiceEndpoint = _strdup( pEndpoint );

			memset( pEndpoint, 0, sizeof(char) * GINGKO_MESSAGE_SIZE );

			if( DigitalServiceEndpoint != NULL )
			{
				free( DigitalServiceEndpoint );
			}
			_snprintf_s( pEndpoint, GINGKO_MESSAGE_SIZE, GINGKO_MESSAGE_SIZE, "%s%s", pSiteUrl,GINGKO_DIGITAL_SERVICE );
			DigitalServiceEndpoint = _strdup( pEndpoint );

			free( pEndpoint );
		}
	}
}



static void soap_connection_option(struct soap *soap)
{
	while( __ThisConnectionOptions._locked )
	{
		WaitForSingleObject( __ThisConnectionOptions._wait_handler, -1 );
	}

	if( soap != NULL )
	{
		if( __ThisConnectionOptions._enable_proxy )
		{
			soap->proxy_host = __ThisConnectionOptions._proxy_host;
			soap->proxy_port = __ThisConnectionOptions._proxy_port;
			soap->proxy_userid = __ThisConnectionOptions._proxy_userid;
			soap->proxy_passwd = __ThisConnectionOptions._proxy_passwd;
			soap->proxy_http_version = "1.1";
		}
		soap->recv_timeout = __ThisConnectionOptions._recv_timeout;
		soap->send_timeout = __ThisConnectionOptions._send_timeout;
		soap->connect_timeout = __ThisConnectionOptions._conn_timeout;
	}
}

GINGKO_METHOD BOOL GINGKO_API SetConnectionOption( bool EnableProxy, const char* _proxyHost, const char* _proxyPort,
												   const char* _proxyUser, const char* _proxyPasswd, 
												   int connTimeout, int sendTimeout, int recvTimeout )
{
	while( __ThisConnectionOptions._locked )
	{
		WaitForSingleObject( __ThisConnectionOptions._wait_handler, -1 );
	}

	__ThisConnectionOptions._locked = 1;
	
	__ThisConnectionOptions._wait_handler = CreateEvent( NULL, FALSE, FALSE, NULL );
	__ThisConnectionOptions._enable_proxy = EnableProxy;
	__ThisConnectionOptions._proxy_host = _strdup( _proxyHost );
	__ThisConnectionOptions._proxy_port = atoi( _proxyPort );
	__ThisConnectionOptions._proxy_userid = _strdup( _proxyUser );
	__ThisConnectionOptions._proxy_passwd = _strdup( _proxyPasswd );
	__ThisConnectionOptions._conn_timeout = connTimeout;
	__ThisConnectionOptions._send_timeout = sendTimeout;
	__ThisConnectionOptions._recv_timeout = recvTimeout;
	
	SetEvent( __ThisConnectionOptions._wait_handler );
	__ThisConnectionOptions._locked = 0;
	__ThisConnectionOptions._wait_handler = NULL;
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API WaitForDetectGingkoServer()
{
	
	while( bUnderDetect )
	{
		if( WAIT_OBJECT_0 == WaitForSingleObject( _g_DetectServerMutex, -1 ) )
		{
			ReleaseMutex( _g_DetectServerMutex );
		}
	}
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API DetectGingkoServer()
{
	if( WAIT_OBJECT_0 == WaitForSingleObject( _g_DetectServerMutex, -1 ) )
	{
		if( bUnderDetect == TRUE )
		{
			return FALSE;
		}

		bUnderDetect = TRUE;

		char *pEndpoint = (char*)malloc(sizeof(char) * GINGKO_MESSAGE_SIZE);
		if( pEndpoint != NULL )
		{
			BOOL Found = FALSE;
			time_t uSmallEffort = 0x7FFFFFFFFFFFFFFFL;
			std::vector<const char*>::const_iterator iter = servers.begin();
			while( iter != servers.end() ){
				const char* pServerName = (*iter);
				////Call gingkoVersion to test
				time_t t = time( NULL );
				ns1__gingkoVersion gv;
				ns1__gingkoVersionResponse gvr; // = {0};
				GingkoLoginSoapServiceBeanServiceSoapBinding binding;
				soap_connection_option(binding.soap);
				//binding.__ns1__gingkoVersion( null, null );
				memset( pEndpoint, 0, sizeof(char) * GINGKO_MESSAGE_SIZE );
				_snprintf_s( pEndpoint, GINGKO_MESSAGE_SIZE, GINGKO_MESSAGE_SIZE,  "%s%s", *iter, GINGKO_LOGIN_SERVICE );
				binding.endpoint = pEndpoint;
				if( SOAP_OK == binding.__ns1__gingkoVersion( &gv, &gvr ) )
				{
					if( gvr.return_ != NULL )
					{
						if( gvr.return_->status != NULL && ((*(gvr.return_->status)) == ns1__soapPackStatusEnum__SUCCESS) )
						{
							//check the status
							time_t e = time( NULL );
							///calculate the time left. 
							time_t diff = e - t;
							////Call gingko
							if( diff < uSmallEffort )
							{
								uSmallEffort = diff;
								///copy this url to url
								Found = TRUE;
								///Copy to the site url
								setTheCallingSiteUrl( *iter );
							}
						}
					}
				}
				iter ++;
			}
			free( pEndpoint );
		}

		bUnderDetect = FALSE;
		ReleaseMutex( _g_DetectServerMutex );
	}
	return TRUE;
}

/***
 * Query the server site GingkoVersion. 
 * If the multiple server's version is not consistence, the client will just get the 
 * lowest version.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoVersion(int *major, int *middle, int *minor, char** sp)
{
	ns1__gingkoVersion gv;
	ns1__gingkoVersionResponse gvr; // = {0};
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetLoginServiceEndpoint();
	soap_connection_option(binding.soap);
	int ret = binding.__ns1__gingkoVersion( &gv, &gvr );

	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	if( gvr.return_ == NULL )
	{
		return FALSE;
	}
	
	ns1__gingkoVersionPack* pack = gvr.return_;

	TlsSaveSoapMessage( pack->message != NULL ? pack->message->data() : NULL,(int) (*pack->status) );

	if( pack->status == NULL || (*pack->status) != ns1__soapPackStatusEnum__SUCCESS )
	{
		return FALSE;
	}


	if( major != NULL ){ *major = pack->major; }
	if( middle != NULL ){ *middle = pack->middle; }
	if( minor != NULL ){ *minor = pack->minor; }
	//if( sp != NULL ){ strcpy( *sp, pack->servicePack ); }	
	return TRUE;
}


/***
 * This client library supports multiple servers of the gingko system. 
 * So, we can add more than one service endpoint for this client. 
 * The service client, will call the service url by every one.
 * This is a global operation.
 ***/
GINGKO_METHOD BOOL GINGKO_API addGingkoServiceUrl(const char* serviceEndpoint)
{
	BOOL ret = FALSE;
	if( WaitForSingleObject( _g_DetectServerMutex, -1 ) == WAIT_OBJECT_0 )
	{
		
		if( serviceEndpoint != NULL )
		{
			servers.push_back( _strdup(serviceEndpoint) );
			ret = TRUE;
		}
		ReleaseMutex( _g_DetectServerMutex );
	}
	return FALSE;
}

/***
 * Get the latest status of the service call. The service call will returned the MessagePack that includes Status and Message
 * by defaults. 
 * This method will retrive this information after the service method failed
 * Thread safe method
 ***/
GINGKO_METHOD SoapStatusEnum GINGKO_API GetLastServiceStatus(const TCHAR** pMessage)
{
	ThreadMessagePack* pack = (ThreadMessagePack*) TlsGetValue( dwGingkoTlsIndex );
	if( pack != NULL )
	{
		if( pMessage != NULL )
		{
			*pMessage = pack->Message;
		}
		return (SoapStatusEnum)(pack->Status);
	}
	return PACK_NOTAVAILABLE;
}

static void setGingkoToken(const TCHAR* token, BOOL bWriteShare)
{
	if( token == NULL )
	{
		return;
	}

	if( bWriteShare )
	{
		if( _gingkoSession != NULL )
		{
			memcpy( _gingkoSession->GingkoToken, token, sizeof(TCHAR) * _tcslen( token ) );
		}
		GingkoCommand *pCmd = NULL;
		if( BuildCommand( &pCmd, _T("PutSession:"),(DWORD)( sizeof(TCHAR) * _tcslen( token )), _gingkoSession->GingkoToken ) )
		{
			ExecCommand( pCmd, NULL, 0 );
			FreeCommand( pCmd );
			pCmd = NULL;
		}
	}

	//GingkoToken = _tcsdup( token );

	int byteSize = WideCharToMultiByte( CP_ACP, 0, token, (int)_tcslen( token ), NULL, 0, NULL, NULL );
	if( byteSize > 0 )
	{
		if( _gingkoToken != NULL )
		{
			free( _gingkoToken );
			_gingkoToken = NULL;
		}

		_gingkoToken = (char*) malloc( byteSize + 2 );
		if( _gingkoToken )
		{
			memset( _gingkoToken, 0, byteSize + 2 );
			WideCharToMultiByte( CP_ACP, 0, token, (int)_tcslen( token ), _gingkoToken, byteSize, NULL, NULL );
		}
	}
}

/**
 * login to the system
 * if login success, we will goto a Session Token
 * Global method
 **/
GINGKO_METHOD BOOL GINGKO_API  gingkoLogin( const TCHAR *userName, const TCHAR* password )
{
	ns1__userLogin ul;
	ns1__userLoginResponse ulr;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetLoginServiceEndpoint();
	soap_connection_option(binding.soap);
	ul.loginId = new std::wstring( userName );
	ul.password = new std::wstring( password );
	int ret = binding.__ns1__userLogin( &ul, &ulr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( ulr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( ulr.return_->message->data(), *ulr.return_->status );

	if( (*ulr.return_->status) == ns1__soapPackStatusEnum__SUCCESS )
	{
		if( ulr.return_->token != NULL )
		{
			setGingkoToken( ulr.return_->token->data(), TRUE );
		}

		if( ulr.return_->gingkoUser )
		{
			SetPackString( (&_gingkoUserPack), loginId, ulr.return_->gingkoUser->loginId );
			SetPackString( (&_gingkoUserPack), bindingId, ulr.return_->gingkoUser->bindingId );
			SetPackString( (&_gingkoUserPack), gingkoId, ulr.return_->gingkoUser->gingkoId );
			SetPackString( (&_gingkoUserPack), displayName, ulr.return_->gingkoUser->name );
			SetPackString( (&_gingkoUserPack), status, ulr.return_->gingkoUser->status );
		}
	}

	soap_free_cookies(binding.soap);

	return _gingkoToken != NULL;
}


/***
 * Check current session was valid for calling system.
 ***/
GINGKO_METHOD BOOL GINGKO_API IsSessionValid(const GingkoUserPack **pack)
{
	if( _gingkoToken == NULL && _gingkoSession != NULL )
	{
		///Convert to 
		
		GingkoCommand *pCmd = NULL;
		BuildCommand( &pCmd, _T("GetSession:"), 0, NULL );
		if( pCmd != NULL )
		{
			if( ExecCommand( pCmd, _gingkoSession->GingkoToken, GINGKO_BLOCK_SIZE ) )
			{
				int slen = (int)_tcslen( _gingkoSession->GingkoToken ); 
				if( slen > 0 )
				{
					setGingkoToken( _gingkoSession->GingkoToken, FALSE );
				}

				FreeUserPack( (&_gingkoUserPack), FALSE );
				
				if( FALSE == checkToken( _gingkoSession->GingkoToken, (&_gingkoUserPack) ) )
				{
					if( _gingkoToken )
					{
						free( _gingkoToken );
					}
					_gingkoToken = NULL;
				}
			}
			FreeCommand( pCmd );
		}
	}

	if( _gingkoToken != NULL )
	{
		///Send the current client process Id will not be handled by Bluefish.
		///PostClientProcessId();

		if( pack )
		{
			(*pack) = &_gingkoUserPack;
		}
		return TRUE;
	}
	return FALSE;
}



GINGKO_METHOD BOOL GINGKO_API clearSession()
{
	if( _gingkoSession != NULL )
	{
		memset( _gingkoSession, 0, sizeof(GingkoSession) );
	}

	if( _gingkoToken )
	{
		free( _gingkoToken );
		_gingkoToken = NULL;
	}
	return TRUE;
}

/***
 * Logout will invalid the current session.
 ***/
GINGKO_METHOD BOOL GINGKO_API logout()
{

	clearSession();

	GingkoCommand *pCmd = NULL;
	
	BuildCommand( &pCmd, _T("Logout:"), 0, NULL );
	
	if( pCmd != NULL )
	{
		ExecCommand( pCmd, NULL, 0 );
		FreeCommand( pCmd );
	}

	return TRUE;
}

/**
 *Register the user and binding to a root user id. 
 *If register to the root server, the bindingId and bindUserPasswd will be ignored.
 **/
GINGKO_METHOD BOOL GINGKO_API  registerUser(const TCHAR *loginId, const TCHAR* displayName, const TCHAR* passwd, const TCHAR *bindingId, const TCHAR *bindUserPasswd  )
{

	ns1__registerUser ru;
	ns1__registerUserResponse rur;
	ru.bindingId = new std::wstring( bindingId );
	ru.bindPassword = new std::wstring( bindUserPasswd );
	ru.loginId = new std::wstring( loginId );
	ru.name = new std::wstring( displayName );
	ru.password = new std::wstring( passwd );
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetLoginServiceEndpoint();
	soap_connection_option(binding.soap);
	int ret = binding.__ns1__registerUser( &ru, &rur );
	
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( rur.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( rur.return_->message->data(), (*rur.return_->status) );

	soap_free_cookies(binding.soap);

	return (*rur.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}
/***
 *Get a new GingkoId
 ***/
GINGKO_METHOD BOOL GINGKO_API newGingkoId(TCHAR** pNewGingkoId)
{
	ns1__newGingkoId ng;
	ns1__newGingkoIdResponse response;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetLoginServiceEndpoint();
	soap_connection_option(binding.soap);
	int ret = binding.__ns1__newGingkoId( &ng, &response );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( response.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( response.return_->message->data(), (*response.return_->status) );

	if( pNewGingkoId != NULL )
	{
		*pNewGingkoId = _tcsdup( response.return_->result->data() );
	}

	soap_free_cookies(binding.soap);

	return TRUE;

}

/**
 * Check the current token about the system i
 **/
GINGKO_METHOD BOOL GINGKO_API checkToken(TCHAR* Token, GingkoUserPack *pUserPack)
{
	ns1__checkToken ct;
	ns1__checkTokenResponse ctr;
	if( Token != NULL )
	{
		ct.token = new std::wstring( Token );
	}
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetLoginServiceEndpoint();
	soap_connection_option(binding.soap);
	int ret = binding.__ns1__checkToken( &ct, &ctr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( ctr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( ctr.return_->message->data(), (*ctr.return_->status) );

	if( pUserPack != NULL && ctr.return_->gingkoUser != NULL )
	{
		SetPackString( pUserPack, loginId, ctr.return_->gingkoUser->loginId );
		SetPackString( pUserPack, bindingId, ctr.return_->gingkoUser->bindingId );
		SetPackString( pUserPack, gingkoId, ctr.return_->gingkoUser->gingkoId );
		SetPackString( pUserPack, displayName, ctr.return_->gingkoUser->name );
		SetPackString( pUserPack, status, ctr.return_->gingkoUser->status );
	}

	soap_free_cookies(binding.soap);

	if( (*ctr.return_->status) == ::ns1__soapPackStatusEnum__SUCCESS )
	{
		return TRUE;
	}
	return FALSE;
}

/***
 * create a digital info registry to the server
 * the registry operation also upload the publickey and private key to database
 ***/
GINGKO_METHOD BOOL GINGKO_API createDigitalInfo(const TCHAR* author,
						const TCHAR* description,
						const TCHAR* digitalId,
						const TCHAR* fileHash,
						const TCHAR* title,
						const TCHAR* keyword,
						const TCHAR* owner,
						const TCHAR* status,
						const TCHAR* securityType,
						const TCHAR* permissionType,
						const TCHAR* limitationType,
						const TCHAR* publicKey,
						const TCHAR* privateKey)
{
	ns1__createDigitalInfo udi;
	ns1__createDigitalInfoResponse udr;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);
	udi.author = author ?  new std::wstring( author ) : NULL;
	udi.description = description ? new std::wstring( description ) : NULL;
	udi.digitalId = digitalId  ? new std::wstring( digitalId ) : NULL;
	udi.keyword = keyword ? new std::wstring( keyword ) : NULL;
	udi.limitationType = limitationType ? new std::wstring( limitationType ) : NULL;
	udi.permissionType = permissionType ? new std::wstring( permissionType ) : NULL;
	udi.securityType = securityType ? new std::wstring( securityType ) : NULL;
	udi.title = title ? new std::wstring( title ) : NULL;
	udi.fileHash = fileHash ? new std::wstring( fileHash ) : NULL;
	udi.owner = owner ? new std::wstring( owner ) : NULL;
	udi.privateKey = privateKey ? new std::wstring( privateKey ) : NULL;
	udi.publicKey = publicKey ? new std::wstring( publicKey ) : NULL;
	udi.status = status ? new std::wstring( status ) : NULL;
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, binding.soap->cookie_domain, binding.soap->cookie_path );
	}

	int ret = binding.__ns1__createDigitalInfo( &udi, &udr );

	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( udr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( udr.return_->message->data(), (*udr.return_->status) );

	soap_free_cookies(binding.soap);

	return (*udr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/**
 * Assign the permission to special user by login id or gingko id.
 **/
GINGKO_METHOD BOOL GINGKO_API  assignPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, const TCHAR *ptszLoginId, const TCHAR* ptszGingkoId,
					  BOOL bOwner, BOOL bHolder, BOOL bRead, BOOL bWrite, BOOL bPrint, BOOL bDelete, BOOL bActived)
{
	ns1__assignPermission udi;
	ns1__assignPermissionResponse udr;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);
	udi.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	udi.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;
	udi.loginId = ptszLoginId ? new std::wstring( ptszLoginId ) : NULL;
	udi.gingkoId = ptszGingkoId ? new std::wstring( ptszGingkoId ) : NULL;
	udi.deletable = bDelete ? true : false;
	udi.isHolder = bHolder ? true : false;
	udi.isOwner = bOwner ? true : false;
	udi.printable = bPrint ? true : false;
	udi.readable = bRead ? true : false;
	udi.writable = bWrite ? true : false;
	udi.actived = bActived ? true : false;
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__assignPermission( &udi, &udr );

	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( udr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( udr.return_->message->data(), (*udr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*udr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;

}

/***
 *delete the digital by given digital id and the unit id
 ***/
GINGKO_METHOD BOOL GINGKO_API  deleteDigitalInfo(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId)
{
	ns1__deleteDigitalInfo ddi;
	ns1__deleteDigitalInfoResponse ddr;

	ddi.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	ddi.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__deleteDigitalInfo( &ddi, &ddr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( ddr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( ddr.return_->message->data(), (*ddr.return_->status) );

	soap_free_cookies(binding.soap);

	return (*ddr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/***
 * find all assigned permissions by digital id and the unit id
 * to execute this method, the current login user should have permission owner or holder.
 ***/
GINGKO_METHOD BOOL GINGKO_API  findAssignedPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, LPVOID pCaller, FetchPermissionCallback PermissionCallback)
{
	ns1__findAssignedPermission fap;
	ns1__findAssignedPermissionResponse fapr;
	fap.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	fap.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__findAssignedPermission( &fap, &fapr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fapr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( fapr.return_->message->data(), (*fapr.return_->status) );

	if( PermissionCallback != NULL )
	{
		std::vector<ns1__gingkoPermission *>::const_iterator itr = fapr.return_->resultsList.begin();
		while( itr != fapr.return_->resultsList.end() )
		{
			GingkoPermissionPack gpp = {0};
			
			ns1__gingkoPermission *ngp = (*itr);
			
			SetPackString( (&gpp), unitId, ngp->gingkoId );
			SetPackString( (&gpp), digitalId, ngp->digitalId );
			SetPackString( (&gpp), loginId, ngp->userName );
			SetPackString( (&gpp), displayName, ngp->userName );
			SetPackString( (&gpp), gingkoId, ngp->gingkoId );
			SetPackString( (&gpp), assignedBy, ngp->assignedBy );
			SetPackString( (&gpp), assignedByName, ngp->assignedByName );
			gpp.holder = ngp->holder;
			gpp.owner = ngp->owner;
			gpp.readable = ngp->readable;
			gpp.deletable = ngp->deletable;
			gpp.writable = ngp->writable;
			gpp.printable = ngp->printable;
			BOOL bRet = PermissionCallback( pCaller, &gpp );
			FreePackValue( (&gpp), unitId );
			FreePackValue( (&gpp), digitalId );
			FreePackValue( (&gpp), loginId );
			FreePackValue( (&gpp), displayName );
			FreePackValue( (&gpp), gingkoId );
			FreePackValue( (&gpp), assignedBy );
			FreePackValue( (&gpp), assignedByName );
			if( bRet == FALSE )
			{
				return FALSE;
			}
			itr ++;
		}
	}
	
	soap_free_cookies(binding.soap);

	return (*fapr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}
/***
 * find the digital information by digital id and unit. 
 * this method just get the information of digital not include security information like the key and permission.
 ***/
GINGKO_METHOD BOOL GINGKO_API  findDigitalInfo(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, GingkoDigitalPack *pack)
{
	ns1__findDigitalInfo fdi;
	ns1__findDigitalInfoResponse fdr;
	fdi.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	fdi.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);

	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__findDigitalInfo( &fdi, &fdr);
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fdr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( fdr.return_->message->data(), (*fdr.return_->status) );

	if( fdr.return_->digitalContent )
	{
		SetPackString( pack, author, fdr.return_->digitalContent->author );
		SetPackString( pack, description, fdr.return_->digitalContent->description );
		SetPackString( pack, fileHash, fdr.return_->digitalContent->fileHash );
		SetPackString( pack, keyword, fdr.return_->digitalContent->keyword );
		SetPackString( pack, limitationType, fdr.return_->digitalContent->limitationType );
		SetPackString( pack, owner, fdr.return_->digitalContent->owner );
		SetPackString( pack, permissionType, fdr.return_->digitalContent->permissionType );
		SetPackString( pack, securityType, fdr.return_->digitalContent->securityType );
		SetPackString( pack, digitalId, fdr.return_->digitalContent->digitalId );
		SetPackString( pack, title, fdr.return_->digitalContent->title );
		SetPackString( pack, status, fdr.return_->digitalContent->status );
	}

	soap_free_cookies(binding.soap);

	return (*fdr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;

}

/***
 * find the digital permission for current user and if the user has read/write rights to this digital, the key will also be 
 * download by this method.
 ***/
GINGKO_METHOD BOOL GINGKO_API  findDigitalPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, GingkoPermissionPack *pack)
{
	ns1__findDigitalPermission fdp;
	ns1__findDigitalPermissionResponse fdpr;
	fdp.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	fdp.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);

	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__findDigitalPermission( &fdp, &fdpr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fdpr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}


	TlsSaveSoapMessage( fdpr.return_->message->data(), (*fdpr.return_->status) );

	if( fdpr.return_->permission != NULL )
	{
		if( pack != NULL )
		{
			SetPackString( pack, displayName, fdpr.return_->permission->userName );
			SetPackString( pack, assignedBy, fdpr.return_->permission->assignedBy );
			SetPackString( pack, assignedByName, fdpr.return_->permission->assignedByName );
			SetPackString( pack, digitalId, fdpr.return_->permission->digitalId );
			SetPackString( pack, gingkoId, fdpr.return_->permission->gingkoId );
			SetPackString( pack, loginId, fdpr.return_->permission->loginId );
			pack->holder = fdpr.return_->permission->holder;
			pack->owner = fdpr.return_->permission->owner;
			pack->readable = fdpr.return_->permission->readable;
			pack->printable = fdpr.return_->permission->printable;
			pack->writable = fdpr.return_->permission->writable;
			pack->deletable = fdpr.return_->permission->deletable;
			pack->deleted = fdpr.return_->deleted;
			////Get Public and Private Key Logic. It's too special.
			SetPackString( pack, publicKey, fdpr.return_->publicKey );
			SetPackString( pack, privateKey, fdpr.return_->privateKey );

			//SetPackString( pack, loginId, fdpr.return_->permission );
		}
	}else
	{
		if( pack != NULL )
		{
			pack->deleted = TRUE;  ///control this digital will be deleted.
		}
	}

	soap_free_cookies(binding.soap);

	return (*fdpr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/***
 * just update the basic information
 ***/
GINGKO_METHOD BOOL GINGKO_API  updateDigitalInfo( const TCHAR* digitalId,
					    const TCHAR* author,
						const TCHAR* description,
						const TCHAR* title,
						const TCHAR* keyword,
						const TCHAR* status,
						const TCHAR* securityType,
						const TCHAR* permissionType,
						const TCHAR* limitationType)
{
	ns1__updateDigitalInfo udi;
	ns1__updateDigitalInfoResponse udr;
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);
	udi.author = author ? new std::wstring( author ) : NULL;
	udi.description = description ? new std::wstring( description ) : NULL;
	udi.digitalId = digitalId ? new std::wstring( digitalId ) : NULL;
	udi.keyword = keyword ? new std::wstring( keyword ) : NULL;
	udi.limitationType = limitationType ? new std::wstring( limitationType ) : NULL;
	udi.permissionType = permissionType ? new std::wstring( permissionType ) : NULL; 
	udi.securityType = securityType ? new std::wstring( securityType ) : NULL;
	udi.title = title ? new std::wstring( title ) : NULL;
	udi.status = status ? new std::wstring( status ) : NULL;

	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__updateDigitalInfo( &udi, &udr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( udr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( udr.return_->message->data(), (*udr.return_->status) );

	soap_free_cookies(binding.soap);

	return (*udr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}


/***
 * change current user's password
 ***/
GINGKO_METHOD BOOL GINGKO_API  changePassword(const TCHAR* pszLoginId, const TCHAR* pszOldPasswd, const TCHAR* pszNewPassword)
{
	ns1__changePassword fdp;
	ns1__changePasswordResponse fdpr;
	
	fdp.loginId = pszLoginId ? new std::wstring( pszLoginId ) : NULL;
	fdp.oldPassword = pszOldPasswd ? new std::wstring( pszOldPasswd ) : NULL;
	fdp.newPassword = pszNewPassword ? new std::wstring( pszNewPassword ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetUserServiceEndpoint();
	soap_connection_option(binding.soap);
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__changePassword( &fdp, &fdpr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fdpr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( fdpr.return_->message->data(), (*fdpr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*fdpr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/***
 * find the user pack information by the gingko id.
 ***/
GINGKO_METHOD BOOL GINGKO_API  findUserByGingkoId(const TCHAR* pszGingkoId, GingkoUserPack *pUser)
{
	ns1__findUserByGingkoId fdp;
	ns1__findUserByGingkoIdResponse fdpr;
	fdp.gingkoId = pszGingkoId ? new std::wstring( pszGingkoId ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetUserServiceEndpoint();
	soap_connection_option(binding.soap);

	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__findUserByGingkoId( &fdp, &fdpr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fdpr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	if( fdpr.return_->gingkoUser != NULL && pUser != NULL )
	{
		SetPackString( pUser, bindingId, fdpr.return_->gingkoUser->bindingId );
		SetPackString( pUser, gingkoId, fdpr.return_->gingkoUser->gingkoId );
		SetPackString( pUser, displayName, fdpr.return_->gingkoUser->name );
		SetPackString( pUser, status, fdpr.return_->gingkoUser->status );
		SetPackString( pUser, loginId, fdpr.return_->gingkoUser->loginId );
		SetPackString( pUser, email, fdpr.return_->gingkoUser->loginId );
	}

	TlsSaveSoapMessage( fdpr.return_->message->data(), (*fdpr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*fdpr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/**
 * search the users by given name. and the result will be pass to fetchUserCallback
 **/
GINGKO_METHOD BOOL GINGKO_API  searchUsers(const TCHAR* pszNames, FetchGingkoUserCallback fetchUserCallback)
{
	ns1__searchUsers su;
	ns1__searchUsersResponse sur;

	su.theName = pszNames ? new std::wstring( pszNames ) : NULL;
	
	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	
	binding.endpoint = GetUserServiceEndpoint();
	soap_connection_option(binding.soap);
	
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__searchUsers( &su, &sur );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( sur.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	std::vector<ns1__gingkoUser*>::const_iterator it = sur.return_->users.begin();
	
	if( fetchUserCallback != NULL )
	{
		BOOL WasBreaked = FALSE;
		while( it != sur.return_->users.end() )
		{
			ns1__gingkoUser *gingkoUser = (*it);
			GingkoUserPack *pUser = (GingkoUserPack*)malloc( sizeof(GingkoUserPack ) );
			if( pUser != NULL )
			{
				memset( pUser, 0, sizeof(GingkoUserPack) );
				SetPackString( pUser, bindingId, gingkoUser->bindingId );
				SetPackString( pUser, gingkoId, gingkoUser->gingkoId );
				SetPackString( pUser, displayName, gingkoUser->name );
				SetPackString( pUser, status, gingkoUser->status );
				SetPackString( pUser, loginId, gingkoUser->loginId );
				SetPackString( pUser, email, gingkoUser->loginId );
				 
				if( !fetchUserCallback( pUser ) )
				{
					WasBreaked = TRUE;
				}

				///User must call freePack to
			}

			//FreePackValue( pUser, bindingId);
			//FreePackValue( pUser, gingkoId);
			//FreePackValue( pUser, displayName);
			//FreePackValue( pUser, status);
			//FreePackValue( pUser, loginId);
			//FreePackValue( pUser, email);

			if( WasBreaked == TRUE )
			{
				break;
			}
			it ++;
		}
		//free( pUser );
		TlsSaveSoapMessage( sur.return_->message->data(), (*sur.return_->status) );
	}

	
	soap_free_cookies(binding.soap);

	return (*sur.return_->status) == ns1__soapPackStatusEnum__SUCCESS;

}

/***
 * find the user's information by login id
 ***/
GINGKO_METHOD BOOL GINGKO_API  findUserByLoginId(const TCHAR* pszLoginId, GingkoUserPack *pUser)
{
	ns1__findUserByLoginId fdp;
	ns1__findUserByLoginIdResponse fdpr;
	fdp.loginId = pszLoginId ? new std::wstring( pszLoginId ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetUserServiceEndpoint();
	soap_connection_option(binding.soap);
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__findUserByLoginId( &fdp, &fdpr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( fdpr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	if( fdpr.return_->gingkoUser != NULL && pUser != NULL )
	{
		SetPackString( pUser, bindingId, fdpr.return_->gingkoUser->bindingId );
		SetPackString( pUser, gingkoId, fdpr.return_->gingkoUser->gingkoId );
		SetPackString( pUser, displayName, fdpr.return_->gingkoUser->name );
		SetPackString( pUser, status, fdpr.return_->gingkoUser->status );
		SetPackString( pUser, loginId, fdpr.return_->gingkoUser->loginId );
		SetPackString( pUser, email, fdpr.return_->gingkoUser->loginId );
	}

	TlsSaveSoapMessage( fdpr.return_->message->data(), (*fdpr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*fdpr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}

/***
 * binding current login user to the new login id. and change the displayName and password
 ***/
GINGKO_METHOD BOOL GINGKO_API  bindingGingkoUser(const TCHAR* loginId,
					   const TCHAR*  thePassword,
					   const TCHAR*  theDisplayName)
{
	ns1__bindingGingkoUser bgu;
	ns1__bindingGingkoUserResponse bgr;
	bgu.loginId = loginId ? new std::wstring( loginId ) : NULL;
	bgu.thePassword = thePassword ? new std::wstring( thePassword ) : NULL;
	bgu.theDisplayName = theDisplayName ? new std::wstring( theDisplayName ) : NULL;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetUserServiceEndpoint();
	soap_connection_option(binding.soap);
	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__bindingGingkoUser( &bgu, &bgr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( bgr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( bgr.return_->message->data(), (*bgr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*bgr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}


/***
 * Initial the FileHeader part
 ***/

static void InitGingkoHeader( PGINGKO_HEADER header )
{
	memset( header, 0, sizeof(GINGKO_HEADER) );
	memcpy( header->Company, "000000", COMPANY_LENGTH);
	memcpy( header->Identifier, "GKTF", GINGKO_IDENTIFIER_LENGTH );
	header->Version[0]='0';
	header->Version[1]='1';
	memcpy( header->Method, "AES#1", GINGKO_METHOD_LENGTH );
	header->SizeOfHeader = sizeof(GINGKO_HEADER);
	header->SizeOfBlock = AES_BLOCK_SIZE;
}

static void HashPrint( unsigned char* hash, TCHAR* Buffer )
{
	_stprintf_s( Buffer, 34, _T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
		hash[0], hash[1],hash[2],hash[3],hash[4],hash[5],hash[6],hash[7],
		hash[8], hash[9],hash[10],hash[11],hash[12],hash[13],hash[14],hash[15] );
}

static void Md5Print( unsigned char* hash, unsigned char* Buffer )
{
	 sprintf_s( (char*)Buffer, 33, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		hash[0], hash[1],hash[2],hash[3],hash[4],hash[5],hash[6],hash[7],
		hash[8], hash[9],hash[10],hash[11],hash[12],hash[13],hash[14],hash[15] );
}

static BOOL EncryptReservedPart( unsigned char *pHeaderKey, DWORD dwKeyLength, GINGKO_HEADER* FileHeader, DWORD dwLength )
{
	PCRYPTO_INFO pCryptoInfo = NULL;	
	char header[512] = {0};
	unsigned char md[66] = {0};
	unsigned char Md5[66] = {0};
	MD5_CTX ctx = {0};
	
	MD5_Init( &ctx );
	MD5_Update( &ctx, pHeaderKey, dwKeyLength );
	MD5_Final( md, &ctx );
	Md5Print( md, Md5 );
	MD5_Init( &ctx );
	//memcpy( buf, FileHeader, 64 );
	MD5_Update( &ctx, FileHeader, (sizeof(GINGKO_HEADER) - FILE_RESERVED_BYTES) );
	//MD5_Update( &ctx, buf, 64 );
	MD5_Final( &(md[16]), &ctx );
	Md5Print( &(md[16]), &(Md5[32]) );
	//KEY_INFO* pkeyInfo = (KEY_INFO*) pHeaderKey;

	int ret = CreatePrimaryKeyInMemoryWithPassword( header, AES, XTS, SHA512,(char*) Md5, 64, &pCryptoInfo );	
	
	if( TERR_SUCCESS == ret )
	{
		unsigned char resv[512] = {0}; 
		UINT64_STRUCT dataUnit = {0};
		dataUnit.Value = 0;

		for( int i = 0; i < 510; i += 10 )
		{
			memcpy( &(resv[i]), "GINGKOFISH", 10 );
		}

		//memcpy( resv, FileHeader->ReservedBytes, FILE_RESERVED_BYTES );
		//memcpy( resv + FILE_RESERVED_BYTES, FileHeader, sizeof(GINGKO_HEADER) - FILE_RESERVED_BYTES );
		EncryptDataUnits( resv, &dataUnit, 1, pCryptoInfo );
		crypto_close( pCryptoInfo );
		
		memcpy( &(FileHeader->ReservedBytes[0]), resv, FILE_RESERVED_BYTES );
		return TRUE;
	}

	return FALSE;

}

GINGKO_METHOD BOOL GINGKO_API DecryptReservedPart( unsigned char *pHeaderKey, DWORD dwKeyLength, GINGKO_HEADER* FileHeader )
{
	PCRYPTO_INFO pCryptoInfo = NULL;
	
	char header[512] = {0};
	unsigned char md[34] = {0};
	MD5_CTX ctx = {0};
	unsigned char Md5[66] = {0};

	MD5_Init( &ctx );
	MD5_Update( &ctx, pHeaderKey, dwKeyLength );
	MD5_Final( md, &ctx );
	Md5Print( md, Md5 );
	MD5_Init( &ctx );
	//memcpy( buf, FileHeader, 64 );
	//MD5_Update( &ctx, buf, 64 );
	MD5_Update( &ctx, FileHeader, (sizeof(GINGKO_HEADER) - FILE_RESERVED_BYTES) );
	MD5_Final( &(md[16]), &ctx );
	Md5Print( &(md[16]), &(Md5[32]) );

	int ret = CreatePrimaryKeyInMemoryWithPassword( header, AES, XTS, SHA512,(char*) Md5, 64, &pCryptoInfo );
	
	if( TERR_SUCCESS == ret )
	{
		UINT64_STRUCT dataUnit = {0};
		dataUnit.Value = 0;
		unsigned char resv[512] = {0};

		memcpy( resv, FileHeader->ReservedBytes, FILE_RESERVED_BYTES);
		//memcpy( resv + FILE_RESERVED_BYTES, FileHeader, sizeof(resv) - FILE_RESERVED_BYTES );
		DecryptDataUnits( resv, &dataUnit, 1, pCryptoInfo );
		crypto_close( pCryptoInfo );
		
		BOOL bok  = TRUE;

		for( int i = 0; i < ((FILE_RESERVED_BYTES) / 10 * 10); i += 10 )
		{
			if( !strncmp( (const char*) &(resv[i]), (const char*)"GINGKOFISH", 10 ) == 0 )
			{
				bok = FALSE;
				break;
			}
		}

		//memcpy( FileHeader->ReservedBytes, resv, FILE_RESERVED_BYTES );
		return bok;
	}

	return FALSE;
}

/***
 * Encrypt a local file and generate the output to another file
 * The GingkoDigitalPack include all information about this local file.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoEncryptFile(HANDLE hFile, HANDLE hOutputFile, 
												GingkoDigitalPack* pack,
												PASSWORDCALLBACK PasswordCallback)
{
	///Generate a Random primary key in memory.
	PCRYPTO_INFO pCryptoInfo = NULL;
	char header[512] = {0};
	TCHAR digitalId[ (MD5_DIGEST_LENGTH + 1) * 2 ];
	TCHAR fileHash[ (MD5_DIGEST_LENGTH + 1) * 2];
	unsigned char md5hash[ (MD5_DIGEST_LENGTH + 1) * 2];
	GINGKO_HEADER GingkoHeader;
	UINT64_STRUCT dataUnit = {0};
	LARGE_INTEGER FileSize = {0};

	if( pack == NULL )
	{
		return FALSE;
	}

	if( pack->unitId == NULL )
	{
		return FALSE;
	}


	if( !GetFileSizeEx( hFile, &FileSize) )
	{
		TlsSaveSoapMessage( _T("Cannot get file size.\n"), GetLastError() );
		return FALSE;
	}

	memset( header, 0, sizeof(header) );
	
	Randinit();

	int ret = CreatePrimaryKeyInMemoryWithCallback( header, AES, XTS, SHA512, PasswordCallback, &pCryptoInfo );
	
	if( TERR_SUCCESS == ret )
	{
		///Success to create the key 
		///now, we are going to encrypt the file 
		///The buffer of this Data unit
		///and the buffer size should be multiple of 512 bytes.
		unsigned char* pBuffer = (unsigned char*) malloc( BYTES_OF_SECTOR * NUMBER_OF_SECTORS );
		DWORD ReadBytes = 0;
		DWORD WriteBytes = 0;
		ULONGLONG TotalSizeOfFile = 0L;
		if( pBuffer == NULL )
		{
			TlsSaveSoapMessage( _T("No enough memory for Encrypt File.\n"), PACK_CALLERERROR );
			//Randfree();
			return FALSE;
		}
		//////////////////////////////////////////////////
		///Generate a digital id
		//////////////////////////////////////////////////
		InitGingkoHeader( &GingkoHeader );
		
		GUID guid;

		if( S_OK == CoCreateGuid(&guid) )
		{
			memcpy( GingkoHeader.Md5, &guid, sizeof(GUID) );
			HashPrint( GingkoHeader.Md5, digitalId );
		}


		if( PasswordCallback != NULL )
		{
			memset( GingkoHeader.Company, 0, sizeof(GingkoHeader.Company) );
		}else
		{
			char szUnitId[ COMPANY_LENGTH * 2 + 1 ] = {0};
			sprintf_s( szUnitId, COMPANY_LENGTH * 2, "%S", pack->unitId );
			memcpy( GingkoHeader.Company, szUnitId, COMPANY_LENGTH );
		}

#ifndef GINGKO_TAILER_SUPPORT
		///Write the header of file
		if( !WriteFile( hOutputFile, &GingkoHeader, sizeof(GINGKO_HEADER), &WriteBytes, NULL ) )
		{
			free( pBuffer );
			TlsSaveSoapMessage( _T("Write file header failed.\n"), PACK_CALLERERROR );
			///Randfree();
			return FALSE;
		}
#endif
		MD5_CTX ctx;
		MD5_Init( &ctx );
		do{
			if( !ReadFile( hFile, pBuffer, BYTES_OF_SECTOR * NUMBER_OF_SECTORS, &ReadBytes, NULL ) )
			{
				free( pBuffer );
				TlsSaveSoapMessage( _T("ReadFile failed.\n"), GetLastError() );
				//Randfree();
				return FALSE;
			}
			
			if( ReadBytes > 0 )
			{
				TotalSizeOfFile += ReadBytes;

				ULONG dataParts = (( ReadBytes / ENCRYPTION_DATA_UNIT_SIZE) + ((ReadBytes % ENCRYPTION_DATA_UNIT_SIZE) ? 1 : 0));
				dataUnit.Value = 0;

				MD5_Update( &ctx, pBuffer, ReadBytes );

				EncryptDataUnits( pBuffer, &dataUnit, dataParts, pCryptoInfo );
				if( pack->ProgressCallback != NULL )
				{
					int perc = (int)(((double)TotalSizeOfFile / (double)FileSize.QuadPart) * 100.0F);
					if( pack->ProgressCallback( pack, perc, _T("Encrypt File") ) == FALSE )
					{
						//IF THE PROGRESS CALLBACK RETURN FALSE, THAT THE PROGRESS WILL BE CANCELLED.
						return FALSE;
					}
				}
				
				if( !WriteFile( hOutputFile, pBuffer, (ReadBytes / BYTES_OF_SECTOR + ((ReadBytes % BYTES_OF_SECTOR) ? 1 : 0) ) *  BYTES_OF_SECTOR, &WriteBytes, NULL ) )
				{
					free( pBuffer );
					TlsSaveSoapMessage( _T("ReadFile failed.\n"), GetLastError() );
					//Randfree();
					return FALSE;
				}
			}
		}while( ReadBytes > 0 );

		free( pBuffer );

		MD5_Final( md5hash, &ctx );
		
		HashPrint( md5hash, fileHash );

		GingkoHeader.SizeOfFile = TotalSizeOfFile;

#ifndef GINGKO_TAILER_SUPPORT
		SetFilePointer( hOutputFile, 0, NULL,  FILE_BEGIN);
#endif

		EncryptReservedPart( (unsigned char*) header, 512, &GingkoHeader, sizeof(GINGKO_HEADER) );

		if( !WriteFile( hOutputFile, &GingkoHeader, sizeof(GINGKO_HEADER), &WriteBytes, NULL ) )
		{
			TlsSaveSoapMessage( _T("Write file header failed.\n"), PACK_CALLERERROR );
			//Randfree();
			return FALSE;
		}///Write the end of file.

		////Store the PublicKey and PrivateKey
		FreePackValue( pack, PrivateKey );
		FreePackValue( pack, PublicKey );

		pack->PrivateKey = Base64Encode((LPVOID)header, sizeof( header ) );
		pack->PublicKey  = Base64Encode((LPVOID)header, sizeof( header ) );

		crypto_close( pCryptoInfo );
	}

	pack->digitalId = _tcsdup( digitalId );
	pack->fileHash = _tcsdup( fileHash );
	if( pack->digitalId == NULL || pack->fileHash == NULL )
	{
		TlsSaveSoapMessage( _T("Can not duplicate the fileHash or digital Id\n"), PACK_CALLERERROR );
		//Randfree();
		return FALSE;
	}

	//BOOL cdi = createDigitalInfo( pack->author, pack->description, pack->digitalId, pack->fileHash, pack->title, 
	//	pack->keyword, pack->owner, pack->status, pack->securityType, pack->permissionType, pack->limitationType,
	//	NULL, NULL );

	//pack->digitalId = NULL;
	//pack->fileHash  = NULL;

	//Randfree();
	return TRUE;
}


/***
 * Decrypt a local file and generate the output to another file
 * The GingkoDigitalPack include all information about this local file.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoDecryptFile(HANDLE hFile, HANDLE hOutputFile, 
												GingkoDigitalPack* pack,
												PASSWORDCALLBACK PasswordCallback)
{
	///Generate a Random primary key in memory.
	PCRYPTO_INFO pCryptoInfo = NULL;
	char header[512] = {0};
	DWORD ReadBytes = 0;
	BOOL IgnoreHashCheck = FALSE;
	GingkoPermissionPack permission = {0} ;
	TCHAR fileHash[ (MD5_DIGEST_LENGTH + 1) * 2];
	unsigned char md5hash[ (MD5_DIGEST_LENGTH + 1) * 2];
	GINGKO_HEADER GingkoHeader;
	UINT64_STRUCT dataUnit = {0};

	memset( header, 0, sizeof(header) );

#ifdef GINGKO_TAILER_SUPPORT
	SetFilePointer( hFile, sizeof(GINGKO_HEADER), NULL,  FILE_END);
#else
	SetFilePointer( hFile, 0, NULL,  FILE_BEGIN);
#endif

	if( ReadFile( hFile, &GingkoHeader, sizeof(GINGKO_HEADER), &ReadBytes, NULL ) )
	{
		if( ReadBytes == sizeof(GINGKO_HEADER) )
		{
			if( GingkoHeader.Identifier[0] == 'G' &&  GingkoHeader.Identifier[1] == 'K'
			 && GingkoHeader.Identifier[2] == 'T' && GingkoHeader.Identifier[3] == 'F')
			{
				TCHAR DigitalId[34] = {0};
				TCHAR UnitId[8] = {0};
				HashPrint( GingkoHeader.Md5, DigitalId );
				if( pack  )
				{
					SetPackValue( pack, digitalId, DigitalId );
					//TRY TO GET THE PACK DETAIL INFORMATION BY CALLING FINDDIGITAL					
					_stprintf_s( UnitId, 7, _T("%C%C%C%C%C%C"), GingkoHeader.Company[0], GingkoHeader.Company[1],
						GingkoHeader.Company[2],GingkoHeader.Company[3],GingkoHeader.Company[4],GingkoHeader.Company[5] );
					
					SetPackValue( pack, unitId, UnitId );
					pack->IsGingkoFile = TRUE;
				}
				//return TRUE;
			}
		}
	}

	Randinit();

	
	if( COMPARE_SELF_DIGITAL( pack->digitalId, pack->unitId ) )
	{
		//findDigitalInfo( UnitId, pack->digitalId, pack );
		//if( PasswordCallback )
		//{
		//	PasswordCallback( );
		//}
		KEY_INFO ky = {0};
		int nTimes = 0;
		Password password = {0};

		IgnoreHashCheck = TRUE;
		
		//ky.salt;

		do{
			memset( &ky, 0, sizeof(KEY_INFO) );
			ky.ea = AES;
			ky.mode = XTS;
			ky.pkcs5_prf = SHA512;
			ky.noIterations = get_pkcs5_iteration_count (SHA512, FALSE);

			if( PasswordCallback != NULL )
			{
				if( PasswordCallback( (int*)&(password.Length), password.Text, MAX_PASSWORD ) == FALSE )
				{
					return TERR_USER_ABORT;
				}
				crypto_loadkey( &ky, (char*)(password.Text), password.Length );
				ky.keyLength = password.Length;
			}

			//memset( ky.salt, 0, PKCS5_SALT_SIZE );
			//memset( ky.slaver_keydata, 0, 108 );
			//memset( ky.master_keydata, 0, MASTER_KEYDATA_SIZE );

			memcpy( ky.master_keydata, (char*)(password.Text), password.Length);
			memcpy( ky.salt, (char*)(password.Text), password.Length);
			memcpy( ky.slaver_keydata, (char*)(password.Text), password.Length);

			if( ReadPrimaryKey((char*)(&ky), &pCryptoInfo ) != TERR_SUCCESS )
			{
				TlsSaveSoapMessage( _T("Restore the User Key was failed.\n"), GetLastError() );
				return FALSE;
			}

			if( DecryptReservedPart( (unsigned char*) &ky, sizeof(ky), &GingkoHeader ) )
			{
				///The Key is OK.
				break;
			}

			if( pCryptoInfo != NULL )
			{
				crypto_close( pCryptoInfo );
				pCryptoInfo = NULL;
			}
			nTimes ++;
		}while( nTimes < 4 );




		permission.owner = TRUE;
		permission.deletable = TRUE;
		//if( TERR_SUCCESS != CreatePrimaryKeyInMemoryWithCallback( header,  AES, XTS, SHA512, PasswordCallback, &pCryptoInfo ) )
		//{
		//	TlsSaveSoapMessage( _T("Get the password failed(by PasswordCallback).\n"), GetLastError() );
		//	return FALSE;
		//}


	}else
	{
		IgnoreHashCheck = FALSE;
		if( !findDigitalPermission( pack->unitId, pack->digitalId, &permission ) )
		{
			TlsSaveSoapMessage( _T("Can not get the Digital Permission.\n"), GetLastError() );
			return FALSE;
		}

		if( permission.publicKey == NULL )
		{
			TlsSaveSoapMessage( _T("Not assigned the permission to descrypt this file.\n"), GetLastError() );
			return FALSE;
		}

		LPVOID lpKeyBuf = Base64Decode( permission.publicKey );
		
		if( lpKeyBuf == NULL )
		{
			TlsSaveSoapMessage( _T("The Key was not be decrypted(Format Error).\n"), GetLastError() );
			return FALSE;
		}

		PKEY_INFO pKeyInfo = (PKEY_INFO)lpKeyBuf;

		if( ReadPrimaryKey((char*)lpKeyBuf, &pCryptoInfo ) != TERR_SUCCESS )
		{
			free( lpKeyBuf );
			lpKeyBuf = NULL;
			TlsSaveSoapMessage( _T("Restore the User Key was failed.\n"), GetLastError() );
			return FALSE;
		}

	}


	if( pCryptoInfo == NULL )
	{
		TlsSaveSoapMessage( _T("The Crypto Context was not acquired. Decryption process was abort.\n"), GetLastError() );
		return FALSE;
	}

	if( !(permission.owner && permission.deletable) )
	{
		crypto_close( pCryptoInfo );
		TlsSaveSoapMessage( _T("No permission to Decrypt this File.\n"), PACK_CALLERERROR );
		return FALSE;
	}

	///now, we are going to decrypt the file 
	///The buffer of this Data unit
	///and the buffer size should be multiple of 512 bytes.
	unsigned char* pBuffer = (unsigned char*) malloc( BYTES_OF_SECTOR * NUMBER_OF_SECTORS );

	ReadBytes = 0;
	DWORD WriteBytes = 0;
	ULONGLONG TotalSizeOfFile = 0L;
	LONG dwToMoveHigh = 0;
	
	if( pBuffer == NULL )
	{
		crypto_close( pCryptoInfo );
		TlsSaveSoapMessage( _T("No enough memory for Encrypt File.\n"), PACK_CALLERERROR );
		return FALSE;
	}

#ifndef GINGKO_TAILER_SUPPORT
	SetFilePointer( hFile, sizeof(GINGKO_HEADER), &dwToMoveHigh, FILE_BEGIN );
#else
	SetFilePointer( hFile, 0, &dwToMoveHigh, FILE_BEGIN );
#endif


	MD5_CTX ctx;
	if( !IgnoreHashCheck )
	{
		MD5_Init( &ctx );
	}
	do{
		if( !ReadFile( hFile, pBuffer, BYTES_OF_SECTOR * NUMBER_OF_SECTORS, &ReadBytes, NULL ) )
		{
			free( pBuffer );
			crypto_close( pCryptoInfo );
			TlsSaveSoapMessage( _T("ReadFile failed.\n"), GetLastError() );
			return FALSE;
		}
		
		if( ReadBytes > 0 )
		{
			TotalSizeOfFile += ReadBytes;
			
#ifdef GINGKO_TAILER_SUPPORT
			if( TotalSizeOfFile > GingkoHeader.SizeOfFile )
			{
				ReadBytes -= sizeof(GINGKO_HEADER);
				TotalSizeOfFile -= sizeof(GINGKO_HEADER);
			}
#endif

			ULONG dataParts = (( ReadBytes / ENCRYPTION_DATA_UNIT_SIZE) + ((ReadBytes % ENCRYPTION_DATA_UNIT_SIZE) ? 1 : 0));
			dataUnit.Value = 0;

			DecryptDataUnits( pBuffer, &dataUnit, dataParts, pCryptoInfo );

			if( TotalSizeOfFile > GingkoHeader.SizeOfFile )
			{
				ReadBytes = ReadBytes - (DWORD)(TotalSizeOfFile - GingkoHeader.SizeOfFile);
				TotalSizeOfFile -= (DWORD)(TotalSizeOfFile - GingkoHeader.SizeOfFile);
			}

			if( pack->ProgressCallback != NULL )
			{
				int perc = (int)(((double)TotalSizeOfFile / (double)(GingkoHeader.SizeOfFile) ) * 100.0F);
				if( pack->ProgressCallback( pack, perc, _T("Decrypt File") ) == FALSE )
				{
					return FALSE;
				}
			}

			if( !IgnoreHashCheck )
			{
				MD5_Update( &ctx, pBuffer, ReadBytes );
			}
			//EncryptDataUnits( pBuffer, &dataUnit, dataParts, pCryptoInfo );
			
			if( !WriteFile( hOutputFile, pBuffer, ReadBytes, &WriteBytes, NULL ) )
			{
				free( pBuffer );
				crypto_close( pCryptoInfo );
				TlsSaveSoapMessage( _T("ReadFile failed.\n"), GetLastError() );
				return FALSE;
			}
		}
	}while( ReadBytes > 0 );

	free( pBuffer );

	if( !IgnoreHashCheck )
	{
		MD5_Final( md5hash, &ctx );
	
		HashPrint( md5hash, fileHash );
	}
	
	BOOL bRet = TRUE;
	////Here to compare the file's md5
	if( !IgnoreHashCheck )
	{
		if( pack->fileHash != NULL )
		{
			if( _tcscmp( pack->fileHash, fileHash ) == 0 )
			{
				bRet = TRUE;
			}else
			{
				bRet = FALSE;
			}
		}
	}
	crypto_close( pCryptoInfo );
	return bRet;
}

GINGKO_METHOD BOOL GINGKO_API GingkoBuildCryptoInfo( unsigned char* password, int length, LPVOID pOutBuf, PVOID pHeader )
{
	KEY_INFO ky = {0};
	ky.ea = AES;
	ky.mode = XTS;
	ky.pkcs5_prf = SHA512;
	ky.noIterations = get_pkcs5_iteration_count (SHA512, FALSE);
	crypto_loadkey( &ky, (char*)password, length );
	ky.keyLength = length;
	//memset( ky.salt, 0, PKCS5_SALT_SIZE );
	//memset( ky.slaver_keydata, 0, 108 );
	//memset( ky.master_keydata, 0, MASTER_KEYDATA_SIZE );

	memcpy( ky.master_keydata, password, length);
	memcpy( ky.salt, password, length);
	memcpy( ky.slaver_keydata, password, length);	
	if( pHeader != NULL )
	{
		PGINGKO_HEADER pGingkoHeader = (PGINGKO_HEADER) pHeader;
		if( !DecryptReservedPart( (unsigned char*)&ky, sizeof(KEY_INFO), pGingkoHeader ) )
		{
			return FALSE;
		}
	}
	memcpy( pOutBuf, &ky, sizeof(KEY_INFO) );
	return TRUE;
}

/****
 * Check file which have the Gingko Identify.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoGetDigitalPack( HANDLE hFile, GingkoDigitalPack *pack )
{
	GINGKO_HEADER GingkoHeader;
	DWORD ReadBytes = 0;

#ifdef GINGKO_TAILER_SUPPORT
	SetFilePointer( hFile, sizeof(GINGKO_HEADER), NULL,  FILE_END);
#else
	SetFilePointer( hFile, 0, NULL,  FILE_BEGIN);
#endif

	if( ReadFile( hFile, &GingkoHeader, sizeof(GINGKO_HEADER), &ReadBytes, NULL ) )
	{
		if( ReadBytes == sizeof(GINGKO_HEADER) )
		{
			if( GingkoHeader.Identifier[0] == 'G' &&  GingkoHeader.Identifier[1] == 'K'
			 && GingkoHeader.Identifier[2] == 'T' && GingkoHeader.Identifier[3] == 'F')
			{
				TCHAR DigitalId[34] = {0};
				TCHAR UnitId[8] = {0};
				HashPrint( GingkoHeader.Md5, DigitalId );
				if( pack  )
				{
					
					SetPackValue( pack, digitalId, DigitalId );
					//TRY TO GET THE PACK DETAIL INFORMATION BY CALLING FINDDIGITAL					
					_stprintf_s( UnitId, 7, _T("%C%C%C%C%C%C"), GingkoHeader.Company[0], GingkoHeader.Company[1],
						GingkoHeader.Company[2],GingkoHeader.Company[3],GingkoHeader.Company[4],GingkoHeader.Company[5] );
					
					SetPackValue( pack, unitId, UnitId );

					if( !COMPARE_SELF_DIGITAL(DigitalId, UnitId) )
					{
						findDigitalInfo( UnitId, pack->digitalId, pack );
					}
					
					pack->IsGingkoFile = TRUE;
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}


GINGKO_METHOD BOOL GINGKO_API requestPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, const TCHAR* ptszGingkoId,
					  BOOL bOwner, BOOL bHolder, BOOL bRead, BOOL bWrite, BOOL bPrint, BOOL bDelete, const TCHAR* ptszMessage )
{
	ns1__requestPermission rp;
	ns1__requestPermissionResponse rpr;

	rp.unitId = ptszUnit ? new std::wstring( ptszUnit ) : NULL;
	rp.digitalId = ptszDigitalId ? new std::wstring( ptszDigitalId ) : NULL;
	rp.gingkoId = ptszGingkoId ? new std::wstring( ptszGingkoId ) : NULL;
	rp.message = ptszMessage ? new std::wstring( ptszMessage ) : NULL;
	rp.isOwner = bOwner ? true : false;
	rp.isHolder = bHolder ? true : false;
	rp.readable = bRead ? true : false;
	rp.writable = bWrite ? true : false;
	rp.printable = bPrint ? true : false;
	rp.deletable = bDelete ? true : false;

	GingkoLoginSoapServiceBeanServiceSoapBinding binding;
	binding.endpoint = GetDigitalServiceEndpoint();
	soap_connection_option(binding.soap);

	if( _gingkoToken != NULL )
	{
		soap_set_cookie( binding.soap, GINGKO_TOKEN_NAME, _gingkoToken, "", "/" );
	}

	int ret = binding.__ns1__requestPermission( &rp, &rpr );
	if( SOAP_OK != ret )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, ret );
		return FALSE;
	}

	if( rpr.return_ == NULL )
	{
		TlsSaveSoapMessage( CALLER_UNKOWN_ERROR, PACK_CALLERERROR );
		return FALSE;
	}

	TlsSaveSoapMessage( rpr.return_->message->data(), (*rpr.return_->status) );
	
	soap_free_cookies(binding.soap);

	return (*rpr.return_->status) == ns1__soapPackStatusEnum__SUCCESS;
}


GINGKO_METHOD BOOL GINGKO_API IsValidGingkoDigital(const GingkoDigitalPack *pDigitalPack)
{
	if( pDigitalPack == NULL ) return FALSE;
	if( pDigitalPack->unitId == NULL && pDigitalPack->digitalId == NULL && pDigitalPack->fileHash == NULL )
		return FALSE;
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API ThreadSetLastMessage( const TCHAR* Message, int status )
{
	TlsSaveSoapMessage( Message, status );
	return TRUE;
}


static BOOL BuildCommand(GingkoCommand** pCmd, LPCTSTR szCmd, DWORD cchBuf, LPVOID lpBuf)
{
	DWORD dwCmdSize = 0;
	if( pCmd != NULL )
	{
		GingkoCommand *pTempCmd = (GingkoCommand*) malloc( sizeof(GingkoCommand) + cchBuf + sizeof(TCHAR) );
		if( pTempCmd != NULL )
		{
			memset( pTempCmd, 0, sizeof(GingkoCommand) + cchBuf + sizeof(TCHAR) );
			
			dwCmdSize = (DWORD) _tcslen(szCmd);
			if( dwCmdSize  >= GINGKO_COMMAND_LENGTH )
			{
				FreeCommand(pTempCmd);
				return FALSE;
			}
			
			memcpy( pTempCmd->szCommand, szCmd, dwCmdSize * sizeof(TCHAR) );
			
			if( lpBuf != NULL )
			{
				memcpy( pTempCmd->szCommandBody, lpBuf, cchBuf );
			}

			pTempCmd->dwSize = sizeof(GingkoCommand) + cchBuf;
			pTempCmd->dwBufSize = cchBuf;

			*pCmd = pTempCmd;
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL __InternalExecCommand(GingkoCommand* pCmd, LPVOID lpRetBuf, DWORD cchBuf)
{
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;
	DWORD cbWritten = 0;
	DWORD cbRead = 0;
	LPCTSTR lpszPipename = GINGKO_PIPE_NAME;
	TCHAR*	ReplyBuff = (TCHAR*) malloc( sizeof(TCHAR) * GINGKO_BLOCK_SIZE );
	if( ReplyBuff == NULL )
	{
		return FALSE;
	}

	while (TRUE)
	{
		hPipe = CreateFile( 
			lpszPipename,   // pipe name 
			GENERIC_READ | GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 
		
		if (hPipe != INVALID_HANDLE_VALUE) 
			break; 
 
		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
 
		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			printf("Could not open pipe"); 
			if( ReplyBuff )
			{
				free( ReplyBuff );
			}
			return FALSE;
		}
 
		// All pipe instances are busy, so wait for 20 seconds. 
 
		if (!WaitNamedPipe(lpszPipename, 20000)) 
		{ 
			printf("Could not open pipe"); 
			CloseHandle( hPipe );
			if( ReplyBuff )
			{
				free( ReplyBuff );
			}
			return FALSE;
		} 
	}
 
	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess) 
	{
		printf("SetNamedPipeHandleState failed"); 
		CloseHandle( hPipe );
		if( ReplyBuff )
		{
			free( ReplyBuff );
		}
		return 0;
	}
 
// Send a message to the pipe server. 
 
	fSuccess = WriteFile( 
				hPipe,                  // pipe handle 
				pCmd,             // message 
				pCmd->dwSize, // message length 
				&cbWritten,             // bytes written 
				NULL);                  // not overlapped 

	if (!fSuccess) 
	{
		printf("WriteFile failed"); 
		CloseHandle( hPipe );
		if( ReplyBuff )
		{
			free( ReplyBuff );
		}
		return FALSE;
	}

	memset( ReplyBuff, 0, sizeof(TCHAR) * GINGKO_BLOCK_SIZE );
 
	fSuccess = ReadFile( 
				hPipe,    // pipe handle 
				ReplyBuff,    // buffer to receive reply 
				sizeof(TCHAR) * GINGKO_BLOCK_SIZE,  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

	if( fSuccess )
	{
		if( lpRetBuf )
		{
			memset( lpRetBuf, 0, cchBuf );
			memcpy( lpRetBuf, ReplyBuff, cbRead > cchBuf ? cchBuf : cbRead );
		}
	}

	if( ReplyBuff )
	{
		free( ReplyBuff );
	} 

   CloseHandle(hPipe);  
   return TRUE; 

}

static BOOL ExecCommand(GingkoCommand* pCmd, LPVOID lpRetBuf, DWORD cchBuf)
{
	int i = 0;
	
	while( __InternalExecCommand(pCmd, lpRetBuf, cchBuf) == FALSE && i < 5 )
	{
		i ++;
		Sleep( 50 );
	}

	return i < 5;

}

static BOOL FreeCommand(GingkoCommand* pCmd)
{
	if( pCmd != NULL )
	{
		free( pCmd );
		return TRUE;
	}
	return FALSE;
}


GINGKO_METHOD BOOL GINGKO_API FreeDidgitalPack( GingkoDigitalPack* __pack, BOOL bFreePack)
{
	if( __pack != NULL ){
		FreePackValue( __pack, digitalId ); 
		FreePackValue( __pack, unitId ); 
		FreePackValue( __pack, fileHash ); 
		FreePackValue( __pack, author ); 
		FreePackValue( __pack, description ); 
		FreePackValue( __pack, owner ); 
		FreePackValue( __pack, title ); 
		FreePackValue( __pack, keyword ); 
		FreePackValue( __pack, status ); 
		FreePackValue( __pack, securityType ); 
		FreePackValue( __pack, permissionType ); 
		FreePackValue( __pack, limitationType ); 
		FreePackValue( __pack, PublicKey ); 
		FreePackValue( __pack, PrivateKey ); 
		if( bFreePack )
		{
			free( __pack );
		}
	}
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API FreeUserPack( GingkoUserPack* __pack, BOOL bFreePack)
{
	if( __pack ){
		FreePackValue( __pack, loginId )
		FreePackValue( __pack, bindingId )
		FreePackValue( __pack, gingkoId )
		FreePackValue( __pack, displayName )
		FreePackValue( __pack, status )
		FreePackValue( __pack, email )
		FreePackValue( __pack, unitId )
		if( bFreePack )
		{
			free( __pack );
		}
	}
	return TRUE;
}


GINGKO_METHOD BOOL GINGKO_API FreePermissionPack( GingkoPermissionPack* __pack, BOOL bFreePack )
{
	if( __pack ){
		FreePackValue( __pack, digitalId )
		FreePackValue( __pack, unitId )
		FreePackValue( __pack, privateKey )		
		FreePackValue( __pack, publicKey )		
		FreePackValue( __pack, loginId )		
		FreePackValue( __pack, gingkoId )		
		FreePackValue( __pack, displayName )	
		FreePackValue( __pack, assignedBy )		
		FreePackValue( __pack, assignedByName )	
		if( bFreePack == TRUE )
		{
			free( __pack );
		}
	}

	return TRUE;
}


GINGKO_METHOD BOOL GINGKO_API FreeFieldValue( LPVOID _pValue )
{
	if( _pValue )
	{
		free( _pValue );
	}
	return TRUE;
}

GINGKO_METHOD TCHAR* GINGKO_API DuplicateFiledValue( const TCHAR* _field )
{
	return _tcsdup( _field );
}

GINGKO_METHOD BOOL GINGKO_API MD5FileObject( const TCHAR* szFileName, unsigned char* lpszMd5, INT iBufSize )
{
	unsigned char* md5Buf = NULL;
	DWORD dwRead = 0;
	MD5_CTX ctx;
	HANDLE hFile = NULL;
	if( lpszMd5 == NULL )
	{
		return FALSE;
	}

	md5Buf = (unsigned char*) malloc(READ_FILE_BUFFER);

	if( md5Buf == NULL )
	{
		return FALSE;
	}

	hFile = CreateFile( szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if( hFile == INVALID_HANDLE_VALUE )
	{
		free( md5Buf );
		return FALSE;
	}

	MD5_Init( &ctx );

	do{
		if( ReadFile( hFile, md5Buf, READ_FILE_BUFFER, &dwRead, NULL ) )
		{
			if( dwRead > 0 )
			{
				MD5_Update( &ctx, md5Buf, dwRead );
			}
		}
	}while( dwRead > 0 );
	CloseHandle( hFile );
	MD5_Final( lpszMd5, &ctx );
	free( md5Buf );
	md5Buf = NULL;
	return TRUE;
}

GINGKO_METHOD BOOL GINGKO_API PostClientProcessId( )
{
	DWORD dwProcessId = GetCurrentProcessId();
	GingkoCommand* pCmd = NULL;
	TCHAR szProcessId[17] = {0};
	_stprintf_s( szProcessId, 16, _T("%d"), dwProcessId );
	
	if( BuildCommand( &pCmd, _T("SetClientProcessId:"), 16 * sizeof(TCHAR), szProcessId ) )
	{
		ExecCommand( pCmd, NULL, 0 );
		FreeCommand( pCmd );
		return TRUE;
	}
	return FALSE;
}


GINGKO_METHOD BOOL GINGKO_API NotifyStopServer( )
{
	DWORD dwProcessId = GetCurrentProcessId();
	GingkoCommand* pCmd = NULL;
	TCHAR szProcessId[12] = {0};
	_stprintf_s( szProcessId, 10, _T("%d"), dwProcessId );
	
	if( BuildCommand( &pCmd, _T("__InternalStopServer:"), 12 * sizeof(TCHAR), szProcessId ) )
	{
		ExecCommand( pCmd, NULL, 0 );
		FreeCommand( pCmd );
		return TRUE;
	}
	return FALSE;
}

GINGKO_METHOD BOOL GINGKO_API ReadNotifyMessage( GingkoNotifyMessage* gnm, DWORD BufSize )
{
	DWORD dwProcessId = GetCurrentProcessId();
	GingkoCommand* pCmd = NULL;
	TCHAR szProcessId[12] = {0};
	_stprintf_s( szProcessId, 10, _T("%d"), dwProcessId );
	
	if( BuildCommand( &pCmd, _T("ReadNotifyMessage:"), 12 * sizeof(TCHAR), szProcessId ) )
	{
		memset( gnm, 0, sizeof(GingkoNotifyMessage) );
		BOOL dr = ExecCommand( pCmd, gnm,  BufSize );
		FreeCommand( pCmd );
		return dr;
	}
	return FALSE;
}

GINGKO_METHOD BOOL GINGKO_API NotifyReloadConfig( )
{
	DWORD dwProcessId = GetCurrentProcessId();
	GingkoCommand* pCmd = NULL;
	TCHAR szProcessId[12] = {0};
	_stprintf_s( szProcessId, 10, _T("%d"), dwProcessId );
	
	if( BuildCommand( &pCmd, _T("__ReloadConfig:"), 12 * sizeof(TCHAR), szProcessId ) )
	{
		ExecCommand( pCmd, NULL, 0 );
		FreeCommand( pCmd );
		return TRUE;
	}
	return FALSE;
}


GINGKO_METHOD BOOL GINGKO_API GingkoSendDecryptPassword( unsigned char* szFileHash, unsigned char* password, ULONG dwProcessId, int retLength )
{
	GingkoCommand* pCmd = NULL;
	GingkoPassword gpwd = {0};

	gpwd.dwPasswordLength = retLength;
	gpwd.dwProcessId = dwProcessId;
	memcpy( gpwd.FileHash, szFileHash, 32 );
	memcpy( gpwd.PasswordText,password, 64 );

	if( BuildCommand( &pCmd, _T("__SendDecryptPassword:"), sizeof(GingkoPassword), &gpwd ) )
	{
		ExecCommand( pCmd, NULL, 0 );
		FreeCommand( pCmd );
		return TRUE;
	}

	return FALSE;
}