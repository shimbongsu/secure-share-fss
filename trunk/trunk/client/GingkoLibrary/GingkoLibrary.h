#ifndef __GINGKO_LIBRARY_H__
#define __GINGKO_LIBRARY_H__

#ifndef GINGKO_API
#define GINGKO_API	  __stdcall
#endif

#ifdef GINGKO_LIBRARY_DLL
#ifdef __cplusplus
#define GINGKO_METHOD extern "C" __declspec(dllexport)
#else
#define GINGKO_METHOD __declspec(dllexport)
#endif
#else
#ifdef GINGKO_LIBRARY_NONE
#define GINGKO_METHOD 
#else
#ifdef __cplusplus
#define GINGKO_METHOD extern "C" __declspec(dllimport)
#else
#define GINGKO_METHOD __declspec(dllimport)
#endif
#endif
#endif


#define GINGKO_CONFIG_FILE			_T("config.xml")
#define GINGKO_LIBRARY_NAME			_T("libgingko.dll")
#define COMPARE_SELF_DIGITAL( __digitalId, __unitId )  ( __unitId == NULL ) || _tcscmp( __unitId, _T("\0\0\0\0\0\0") ) == 0
#define GINGKO_METHOD_LENGTH		5
#define GINGKO_IDENTIFIER_LENGTH	5
#define GINGKO_VERSION_LENGTH		2
#define MD5_DIGEST_LENGTH			16
#define COMPANY_LENGTH				6
#define GINGKO_BLOCK_SIZE			1024
#define GINGKO_COMMAND_LENGTH		64
#define FILE_RESERVED_NORMAL		10
#define FILE_RESERVED_BYTES			448
#define GINGKO_PIPE_NAME			_T("\\\\.\\pipe\\GingkoPipe-{B7608273-E2DA-42a4-8357-4530DD5F7F3F}");
#ifndef PASSWORDCALLBACK
typedef BOOL (PASCAL *PASSWORDCALLBACK)(int *, unsigned char*, int); 
#endif

#ifndef PROGRESSCALLBACK
typedef BOOL (PASCAL *PROGRESSCALLBACK)(LPVOID __pack, int, const TCHAR* pszMsg); 
#endif

typedef enum {
	PACK_SUCCESS = 0, 
	PACK_NOTFOUND = 1, 
	PACK_DISABLED = 2, 
	PACK_EXISTING = 3, 
	PACK_NOTAVAILABLE = 4, 
	PACK_DELETED = 5, 
	PACK_ERROR = 6,
	PACK_CALLERERROR = 7
}SoapStatusEnum;

typedef struct __GingkoSessionTag
{
	UINT	dwSize;
	TCHAR	GingkoToken[ GINGKO_BLOCK_SIZE * 2];
	TCHAR	GingkoSessionId[GINGKO_BLOCK_SIZE];
	TCHAR	GingkoReserved[GINGKO_BLOCK_SIZE];
}GingkoSession;

typedef struct __GingkoPassword
{
	UINT dwPasswordLength;
	ULONG	dwProcessId;
	unsigned char PasswordText[64];
	unsigned char FileHash[32];
}GingkoPassword;

typedef struct __GingkoNotifyMessage
{
	struct __GingkoNotifyMessage *Next;
	ULONG	dwProcessId;
	ULONG	dwPermission;
	TCHAR	ProcessName[MAX_PATH];
	UINT	dwFileNameLength;
	TCHAR	szFileName[MAX_PATH];
}GingkoNotifyMessage;

typedef struct __GingkoCommand
{
	TCHAR szCommand[GINGKO_COMMAND_LENGTH];
	DWORD dwSize;
	DWORD dwBufSize;
	TCHAR szCommandBody[1];
}GingkoCommand;

#pragma pack(1)
typedef struct _GINGKO_HEADER{
	unsigned char Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
	unsigned char Version[GINGKO_VERSION_LENGTH];			///first char is the major version, second char is the minor version. the version struct is major.minor
	ULONG SizeOfHeader;										///the size of the file header.
	ULONGLONG SizeOfFile;									///the file size of file
	unsigned char Company[COMPANY_LENGTH];				///The organization code 
	unsigned char Md5[MD5_DIGEST_LENGTH];					///md5 of this file.
	unsigned char Method[GINGKO_METHOD_LENGTH];				///engine of crypto //RSA#1
	UINT SizeOfBlock;										///one block of size should to take to decrypt. the size should be depend to the method of crypto
	ULONG Reserved;											///Reserved
	unsigned char  ReservedContent[FILE_RESERVED_NORMAL];
	unsigned char  ReservedBytes[FILE_RESERVED_BYTES];
} GINGKO_HEADER, *PGINGKO_HEADER;
#pragma pack()

typedef struct{
	TCHAR* digitalId;
	TCHAR* unitId;
	TCHAR* fileHash;
	TCHAR* author;
	TCHAR* description;
	TCHAR* owner;
	TCHAR* title;
	TCHAR* keyword;
	TCHAR* status;
	TCHAR* securityType;
	TCHAR* permissionType;
	TCHAR* limitationType;
	TCHAR* PublicKey;
	TCHAR* PrivateKey;
	BOOL   IsGingkoFile;
	LPVOID CallObject;
	PROGRESSCALLBACK ProgressCallback;
}GingkoDigitalPack;

#define FreePackValue( __pack, __field )		\
	if( __pack->__field != NULL ){ FreeFieldValue( __pack->__field ); __pack->__field = NULL; } \


//#define FreeDidgitalPack( __pack)			\
//		if( __pack != NULL ){\
//			FreePackValue( __pack, digitalId ); \
//			FreePackValue( __pack, unitId ); \
//			FreePackValue( __pack, fileHash ); \
//			FreePackValue( __pack, author ); \
//			FreePackValue( __pack, description ); \
//			FreePackValue( __pack, owner ); \
//			FreePackValue( __pack, title ); \
//			FreePackValue( __pack, keyword ); \
//			FreePackValue( __pack, status ); \
//			FreePackValue( __pack, securityType ); \
//			FreePackValue( __pack, permissionType ); \
//			FreePackValue( __pack, limitationType ); \
//			FreePackValue( __pack, PublicKey ); \
//			FreePackValue( __pack, PrivateKey ); \
//		}\
//
//
//#define FreeUserPack( __pack )					\
//	if( __pack ){								\
//		FreePackValue( __pack, loginId )		\
//		FreePackValue( __pack, bindingId )		\
//		FreePackValue( __pack, gingkoId )		\
//		FreePackValue( __pack, displayName )		\
//		FreePackValue( __pack, status )		\
//		FreePackValue( __pack, email )		\
//		FreePackValue( __pack, unitId )		\
//	}										\


#define SetPackValue( __pack, __field, __value__ )		\
					if( __pack->__field ) {			\
						FreeFieldValue( __pack->__field );	\
						__pack->__field = NULL;		\
					} \
					__pack->__field = __value__ ? DuplicateFiledValue( __value__ ) : NULL; \


#define SetPackString( __pack, __field, __value__ )		\
					if( __pack->__field ) {			\
						FreeFieldValue( __pack->__field );	\
						__pack->__field = NULL;		\
					} \
					__pack->__field = __value__ ? DuplicateFiledValue( __value__->data() ) : NULL; \

#define GingkoFree( __xx__ ) FreeFieldValue( __xx__ );
//#define FreePermissionPack( __pack )					\
//	if( __pack ){								\
//		FreePackValue( __pack, digitalId )		\
//		FreePackValue( __pack, unitId )		\
//		FreePackValue( __pack, privateKey )		\
//		FreePackValue( __pack, publicKey )		\
//		FreePackValue( __pack, loginId )		\
//		FreePackValue( __pack, gingkoId )		\
//		FreePackValue( __pack, displayName )	\
//		FreePackValue( __pack, assignedBy )		\
//		FreePackValue( __pack, assignedByName )	\
//	}	\
//

typedef struct{
	TCHAR* loginId;
	TCHAR* bindingId;
	TCHAR* gingkoId;
	TCHAR* displayName;
	TCHAR* status;
	TCHAR* email;
	TCHAR* unitId;
}GingkoUserPack;

typedef struct{
	TCHAR* digitalId;
	TCHAR* unitId;
	TCHAR* privateKey;
	TCHAR* publicKey;
	TCHAR* loginId;
	TCHAR* gingkoId;
	TCHAR* displayName;
	TCHAR* assignedBy;
	TCHAR* assignedByName;
	BOOL   holder;
	BOOL   owner;
	BOOL   deleted;
	BOOL   deletable;
	BOOL   readable;
	BOOL   printable;
	BOOL   writable;
}GingkoPermissionPack;


typedef struct {
	TCHAR* MasterServer;
	TCHAR* UnitServer;
	TCHAR* Connection;
	TCHAR* ProxyServer;
	TCHAR* ProxyPort;
	TCHAR* ProxyUser;
	TCHAR* ProxyPassword;
	TCHAR* MicsTimeout;
	TCHAR* Language;
}GingkoOptions;


GINGKO_METHOD const TCHAR* GINGKO_API  GetGlobalCaption( UINT Id, const TCHAR* pszDefault );

GINGKO_METHOD BOOL GINGKO_API UpdateDialogUI( HWND hDlg, UINT IDD );

GINGKO_METHOD BOOL GINGKO_API ParseConfigFile( const TCHAR* szFilename );

GINGKO_METHOD BOOL GINGKO_API GetGingkoOptions( GingkoOptions** pOpts );

GINGKO_METHOD BOOL GINGKO_API SaveConfigFile( const TCHAR* szFilename );

GINGKO_METHOD BOOL GINGKO_API ParseLanguageFile( const TCHAR* lpszFilePath );

GINGKO_METHOD BOOL GINGKO_API FreeConfig();

GINGKO_METHOD BOOL GINGKO_API GingkoFree( LPVOID lpData);

GINGKO_METHOD BOOL GINGKO_API FreePermissionPack( GingkoPermissionPack* __pack, BOOL bFreePack );

/***
 * MEMORY MANAGEMENT TO FREE
 ***/
GINGKO_METHOD BOOL GINGKO_API FreeDidgitalPack( GingkoDigitalPack* __pack, BOOL bFreePack);

/***
 * MEMORY MANAGEMENT TO FREE
 ***/
GINGKO_METHOD BOOL GINGKO_API FreeUserPack( GingkoUserPack* __pack, BOOL bFreePack );

GINGKO_METHOD TCHAR* GINGKO_API DuplicateFiledValue( const TCHAR* _field );
/****
 * Free only one point
 ***/
GINGKO_METHOD BOOL GINGKO_API FreeFieldValue( LPVOID _pValue );
/***
 * Set the basic connection options for this client
 ***/
GINGKO_METHOD BOOL GINGKO_API SetConnectionOption( bool EnableProxy, const char* _proxyHost, const char* _proxyPort,
												   const char* _proxyUser, const char* _proxyPasswd, 
												   int connTimeout, int sendTimeout, int recvTimeout );
/****
 * Fetch all GingkoUser by Callback, if the callback return false, it will be stoped.
 ***/
typedef BOOL (GINGKO_API *FetchGingkoUserCallback)( GingkoUserPack *pUser );

/****
 * Fetch all GingkoPermissionPack by Callback, if the callback return false, it will be stoped.
 ***/
typedef BOOL (GINGKO_API *FetchPermissionCallback)( LPVOID pCaller, const GingkoPermissionPack *pPerms );

/***
 * This client library supports multiple servers of the gingko system. 
 * So, we can add more than one service endpoint for this client. 
 * The service client, will call the service url by every one.
 ***/
GINGKO_METHOD BOOL  GINGKO_API addGingkoServiceUrl(const char* serviceEndpoint);

/***
 * Provide the ability to detect the Gingko Server automatically. 
 * The DetectGingkoServer will check the WebService call by gingkoVersion operation.
 * Which calling time will be calculated by the method from GingkoServiceUrl list.
 * Then the system will put to the server url.
 ***/
GINGKO_METHOD BOOL GINGKO_API DetectGingkoServer();

/***
 * Check current session was valid for calling system.
 ***/
GINGKO_METHOD BOOL GINGKO_API IsSessionValid(const GingkoUserPack **pack);

/***
 * Logout will invalid the current session.
 ***/
GINGKO_METHOD BOOL GINGKO_API logout();

/***
 * Get the latest status of the service call. The service call will returned the MessagePack that includes Status and Message
 * by defaults. 
 * This method will retrive this information after the service method failed
 ***/
GINGKO_METHOD SoapStatusEnum GINGKO_API GetLastServiceStatus(const TCHAR** pMessage);

/***
 * Query the server site GingkoVersion. 
 * If the multiple server's version is not consistence, the client will just get the 
 * lowest version.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoVersion(int *major, int *middle, int *minor, char** sp);

/**
 * login to the system
 * if login success, we will goto a Session Token
 **/
GINGKO_METHOD BOOL GINGKO_API gingkoLogin( const TCHAR *userName, const TCHAR* password );

/**
 *Register the user and binding to a root user id. 
 *If register to the root server, the bindingId and bindUserPasswd will be ignored.
 **/
GINGKO_METHOD BOOL GINGKO_API registerUser(const TCHAR *loginId, const TCHAR* displayName, const TCHAR* passwd, const TCHAR *bindingId, const TCHAR *bindUserPasswd  );

/***
 *Get a new GingkoId
 ***/
GINGKO_METHOD BOOL GINGKO_API newGingkoId( TCHAR** pNewGingkoId); 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**  The Digital Service method
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Check the current token about the system i
 **/
//GINGKO_METHOD BOOL GINGKO_API checkToken(TCHAR* Token);
GINGKO_METHOD BOOL GINGKO_API checkToken(TCHAR* Token, GingkoUserPack *pUserPack);

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
						const TCHAR* privateKey);

/**
 * Assign the permission to special user by login id or gingko id.
 **/
GINGKO_METHOD BOOL GINGKO_API assignPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, const TCHAR *ptszLoginId, const TCHAR* ptszGingkoId,
					  BOOL bOwner, BOOL bHolder, BOOL bRead, BOOL bWrite, BOOL bPrint, BOOL bDelete, BOOL bActived);

/**
 * Assign the permission to special user by login id or gingko id.
 **/
GINGKO_METHOD BOOL GINGKO_API requestPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, const TCHAR* ptszGingkoId,
					  BOOL bOwner, BOOL bHolder, BOOL bRead, BOOL bWrite, BOOL bPrint, BOOL bDelete, const TCHAR* ptszMessage );

/***
 *delete the digital by given digital id and the unit id
 ***/
GINGKO_METHOD BOOL GINGKO_API deleteDigitalInfo(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId);

/***
 * find all assigned permissions by digital id and the unit id
 * to execute this method, the current login user should have permission owner or holder.
 ***/
GINGKO_METHOD BOOL GINGKO_API findAssignedPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, LPVOID pCaller, 
													 FetchPermissionCallback PermissionCallback);

/***
 * find the digital information by digital id and unit. 
 * this method just get the information of digital not include security information like the key and permission.
 ***/
GINGKO_METHOD BOOL GINGKO_API findDigitalInfo(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, GingkoDigitalPack *pack);

/***
 * find the digital permission for current user and if the user has read/write rights to this digital, the key will also be 
 * download by this method.
 ***/
GINGKO_METHOD BOOL GINGKO_API findDigitalPermission(const TCHAR* ptszUnit, const TCHAR *ptszDigitalId, GingkoPermissionPack *pack);

/***
 * just update the basic information
 ***/
GINGKO_METHOD BOOL GINGKO_API updateDigitalInfo( const TCHAR* digitalId,
					    const TCHAR* author,
						const TCHAR* description,
						const TCHAR* title,
						const TCHAR* keyword,
						const TCHAR* status,
						const TCHAR* securityType,
						const TCHAR* permissionType,
						const TCHAR* limitationType);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**  The User Service method
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/***
 * change current user's password
 ***/
GINGKO_METHOD BOOL GINGKO_API changePassword(const TCHAR* pszLoginId, const TCHAR* pszOldPasswd, const TCHAR* pszNewPassword);

/***
 * find the user pack information by the gingko id.
 ***/
GINGKO_METHOD BOOL GINGKO_API  findUserByGingkoId(const TCHAR* pszGingkoId, GingkoUserPack *pUser);

/***
 * find the user's information by login id
 ***/
GINGKO_METHOD BOOL GINGKO_API  findUserByLoginId(const TCHAR* pszLoginId, GingkoUserPack *pUser);


/***
 * find the user pack information by the gingko id.
 ***/
GINGKO_METHOD BOOL GINGKO_API  searchUsers(const TCHAR* pszNames, FetchGingkoUserCallback fetchUserCallback);

/***
 * binding current login user to the new login id. and change the displayName and password
 ***/
GINGKO_METHOD BOOL GINGKO_API bindingGingkoUser(const TCHAR* loginId,
					   const TCHAR*  thePassword,
					   const TCHAR*  theDisplayName);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//**  File encrypt/decrypt supports
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***
 * Encrypt a local file and generate the output to another file
 * The GingkoDigitalPack include all information about this local file.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoEncryptFile(HANDLE hFile, HANDLE hOutputFile, 
												GingkoDigitalPack* pack,
												PASSWORDCALLBACK PasswordCallback);

/***
 * Decrypt a local file and generate the output to another file
 * The GingkoDigitalPack include all information about this local file.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoDecryptFile(HANDLE hFile, HANDLE hOutputFile, 
												GingkoDigitalPack* pack,
												PASSWORDCALLBACK PasswordCallback);

/***
 * Decrypt a local file and generate the output to another file
 * The GingkoDigitalPack include all information about this local file.
 ***/
GINGKO_METHOD BOOL GINGKO_API DecryptReservedPart( unsigned char *pHeaderKey, DWORD dwKeyLength, 
												GINGKO_HEADER* FileHeader);
/****
 * Check file which have the Gingko Identify.
 ***/
GINGKO_METHOD BOOL GINGKO_API GingkoGetDigitalPack( HANDLE hFile, GingkoDigitalPack *pack );

/*****
 * Waiting for the DetectGingkoServer process completed.
 ****/
GINGKO_METHOD BOOL GINGKO_API WaitForDetectGingkoServer();

GINGKO_METHOD BOOL GINGKO_API IsValidGingkoDigital(const GingkoDigitalPack *pDigitalPack);

GINGKO_METHOD BOOL GINGKO_API clearSession();

/**
 * SAVE THE MESSAGE
 **/
GINGKO_METHOD BOOL GINGKO_API ThreadSetLastMessage( const TCHAR* Message, int status );

GINGKO_METHOD BOOL GINGKO_API GingkoBuildCryptoInfo( unsigned char* password, int length, LPVOID pOutBuf, PVOID pHeader );

GINGKO_METHOD TCHAR* GINGKO_API Base64Encode( LPVOID src,UINT srclen );
GINGKO_METHOD LPVOID GINGKO_API Base64Decode(TCHAR *src);
GINGKO_METHOD BOOL   GINGKO_API FreeBase64Buffer(LPVOID lp);

GINGKO_METHOD BOOL GINGKO_API MD5FileObject( const TCHAR* szFileName, unsigned char* lpszMd5, INT iBufSize );

GINGKO_METHOD BOOL GINGKO_API PostClientProcessId();
GINGKO_METHOD BOOL GINGKO_API NotifyReloadConfig();
GINGKO_METHOD BOOL GINGKO_API NotifyStopServer();
GINGKO_METHOD BOOL GINGKO_API GingkoSendDecryptPassword( unsigned char* szFileHash, unsigned char* password, ULONG dwProcessId, int retLength );
GINGKO_METHOD BOOL GINGKO_API ReadNotifyMessage( GingkoNotifyMessage* gnm, DWORD BufSize );

#endif
