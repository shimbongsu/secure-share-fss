#ifndef __CHEAPMAN_H__
#define __CHEAPMAN_H__

#define GINGKO_METHOD_LENGTH		5
#define GINGKO_IDENTIFIER_LENGTH	5
#define GINGKO_VERSION_LENGTH		2
#define MD5_DIGEST_LENGTH			16
#define COMPANY_LENGTH				6
#define FILE_RESERVED_BYTES			448
#define FILE_RESERVED_NORMAL		10
#define MAXSIZE_PRIVATE_KEY			1024


#define MAKE_PERMISSION( __owner, __holder, __read, __write, __print, __delete )			\
			(__owner ? 0x0F000000 : 0x00000000) | (__holder ? 0x00F00000 : 0x00000000) |	\
			(__read ? 0x000F0000 : 0x00000000) | (__write ? 0x0000F000 : 0x00000000) |		\
			(__print ? 0x00000F00 : 0x00000000) | (__delete ? 0x000000F0: 0x00000000)		

typedef struct __GingkoPasswordTable
{
	unsigned char FileHash[32];
	unsigned char Password[64];
	ULONG	 dwProcessId;
	INT		 PasswordLength;
	BOOL	 PasswordGot;
	struct __GingkoPasswordTable *Next;
}GingkoPasswordTable;


typedef enum _GINGKO_DEVICE_IO_CONTROL_CODE
{
	GINGKO_DEVICE_START = 30001,      ///The Service of Decrypt and Encrypt will be started.
	GINGKO_DEVICE_STOP = 30002,       ///The service of decrypt and encrypt will be stoped.
	GINGKO_DEVICE_DECRYPTED = 30003,  ///The client will send the decrypted buffer to this device.
	GINGKO_DEVICE_ENCRYPTED = 30004,  ///The client will send the encrypted buffer to this device. 
	GINGKO_DEVICE_PAUSE = 30005,      ///The service will be paused. 
	GINGKO_DEVICE_RESUME = 30006,     ///The service will be resume.
	GINGKO_DEVICE_NOTIF =  30007,     ///The service will be paused. 
	GINGKO_GET_SESSION =  30100,      ///Read the session from device. 
	GINGKO_PUT_SESSION =  30102,       ///Write the session to device.
	GINGKO_PUT_FILEPERMISSION = 30103, ///Set the file permission to system.
	GINGKO_GET_REQUESTPARAMETER = 40001, ///GET THE REQUEST PARAMETER
	GINGKO_PUT_RESPONSEBUFFER	= 40002,  ///PUT THE RESPONSE INFORMATION
	GINGKO_GET_REQUEST = 40005, ///GET THE REQUEST PARAMETER
	GINGKO_PUT_RESPONSE = 40006, ///GET THE REQUEST PARAMETER
	GINGKO_SET_ENCRYPT_START	= 50001,	///Indicate the drive to start the process for encrypt the write buffer.
	GINGKO_SET_ENCRYPT_END	= 50002,		///Indicate the drive to end the process for encrypt the write buffer.
	GINGKO_SET_ENCRYPT_PROCESS	= 50003,	///add the process to encrypt queue.
	GINGKO_SET_DECRYPT_PROCESS	= 50004,		///add the process to decrypt queue.
	GINGKO_QUERY_PROCESS_PERMISSION	= 60001		///Query the permission allowed for this process
} GINGKO_DEVICE_IO_CONTROL_CODE;

//typedef struct _GINGKO_HEADER{
//	unsigned char Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
//	unsigned char Version[GINGKO_VERSION_LENGTH];			///first char is the major version, second char is the minor version. the version struct is major.minor
//	ULONG SizeOfHeader;										///the size of the file header.
//	ULONGLONG SizeOfFile;									///the file size of file
//	unsigned char Company[COMPANY_LENGTH];				///The organization code 
//	unsigned char Md5[MD5_DIGEST_LENGTH];					///md5 of this file.
//	unsigned char Method[GINGKO_METHOD_LENGTH];				///engine of crypto //RSA#1
//	UINT SizeOfBlock;										///one block of size should to take to decrypt. the size should be depend to the method of crypto
//	ULONG Reserved;											///Reserved
//	unsigned char  ReservedBytes[448];
//} GINGKO_HEADER, *PGINGKO_HEADER;

typedef struct {
	HANDLE				FileObject;
	HANDLE				RequestKeyEvent;
	ULONG				dwProcessId;
	unsigned char		Identifier[GINGKO_IDENTIFIER_LENGTH];	///the file format identfied,, should be GKTF
	unsigned char		Version[GINGKO_VERSION_LENGTH];			///first char is the major version, second char is the minor version. the version struct is major.minor
	ULONG				SizeOfHeader;							///the size of the file header.
	ULONGLONG			SizeOfFile;								///the file size of file
	unsigned char		Company[COMPANY_LENGTH];				///The organization code 
	unsigned char		Md5[MD5_DIGEST_LENGTH];					///md5 of this file.
	unsigned char		Method[GINGKO_METHOD_LENGTH];			///engine of crypto //RSA#1
	unsigned char		FileReserved[FILE_RESERVED_BYTES];		///Reserved Parts Of File. It's very important.
	UINT SizeOfBlock;											///one block of size should to take to decrypt. the size should be depend to the method of crypto
	ULONG Reserved;												///Reserved
	ULONG				FilenameLength;
	WCHAR				Filename[1];
} PrivateKeyRequest;

typedef struct {
	LONG_PTR				FileObject;
	LONG_PTR				RequestKeyEvent;
	ULONG					ProcessId;
	ULONG					Permission;
	unsigned char			Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
	unsigned char			Version[GINGKO_VERSION_LENGTH];			///first char is the major version, second char is the minor version. the version struct is major.minor
	ULONG					SizeOfHeader;										///the size of the file header.
	ULONGLONG				SizeOfFile;									///the file size of file
	unsigned char			Company[COMPANY_LENGTH];				///The organization code 
	unsigned char			Md5[MD5_DIGEST_LENGTH];					///md5 of this file.
	unsigned char			Method[GINGKO_METHOD_LENGTH];				///engine of crypto //RSA#1
	UINT					SizeOfBlock;										///one block of size should to take to decrypt. the size should be depend to the method of crypto
	ULONG					Reserved;			///Reserved
	int						KeySize;
	int						SizeOfResponse;
	unsigned char			CryptoKey[MAXSIZE_PRIVATE_KEY];
} PrivateKeyResponse;


typedef struct _GINGKO_SHARED_NOTIFICATION
{
	ULONG  cbStruct;
	HANDLE WriteEvent;              ////Write Event. WaitForSingleObject will called by this value. 
	HANDLE ReadEvent;				////Read Event,  WaitForSingleObject it called by 
	HANDLE ServerProcess;			
	HANDLE ClientProcess;
	HANDLE ServerThread;
	ULONG_PTR GingkoDecryptBuffer;
} SharedNotification;


typedef struct {
	HANDLE	hThread;
	DWORD   dwThreadId;
	HANDLE	hEvent;
	PVOID	ThreadArgs;
	BOOL	bThreadExit;
	BOOL	Suspend;
	DWORD	dwExitCode;
} ThreadBag;

extern ULONG		  __gClientProcessId;

BOOL DeviceIoGetPrivateKeyRequest(PrivateKeyRequest **pkr);

BOOL StartThread(LPTHREAD_START_ROUTINE ThreadRoutine, PVOID ThreadArgs, ThreadBag **pBag );

BOOL StopThread(ThreadBag *bag);

BOOL DeviceIoPutPrivateKeyResponse(PrivateKeyRequest *pkr, int KeySize, unsigned char* CryptoKeyBuf, ULONG permission);

DWORD WINAPI DeviceIoThreadProc(PVOID threadArg);

DWORD WINAPI RequestKeyThreadProc(PVOID threadArg);

BOOL DeviceIoSetRequestStop();

void HexToBytes( const unsigned char *p, unsigned char* pbuf, int maxPlen );

int QueryPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response );

int RenewPrimaryKey( PrivateKeyRequest *request, PrivateKeyResponse *response );

void CopyRequestToResponse( PrivateKeyRequest *request, PrivateKeyResponse *response );

BOOL GetDigitalPassword( const TCHAR *fileHashId, const TCHAR *szFileName, 
						unsigned char* lpvPassBuffer, int maxlength, int *retLength, 
						ULONG dwProcessId );

DWORD NotifyProcessEvent( LPCTSTR szFileName, ULONG dwProcessId, ULONG dwPermission );

void OpenDriver();

int DoDriverInstall();

int DriverLoad ();

BOOL DriverUnload();

int StartDriverService();

BOOL InstallEventSource();

BOOL InitCheapman();
BOOL UninitCheapman();
PSECURITY_DESCRIPTOR GkoInitializeSecurityDescriptor( SECURITY_ATTRIBUTES* pSa );
BOOL InsertGingkoPassword( unsigned char *fileHash, unsigned char *passwd, int pwdLength, ULONG dwProcessId );  
BOOL FindGingkoPassword( unsigned char *fileHash, ULONG dwProcessId, unsigned char *passwd, int* pwdLength, BOOL bRemove );
BOOL GetProcessBaseName(DWORD dwProcessId, TCHAR* szFileName, INT Length );

BOOL IsWindows64Bit();
BOOL GetProcessExeFile(DWORD dwProcessId, TCHAR* szFileName, INT Length );
DWORD TryRemoteInjection( DWORD dwProcessId );
BOOL IsProcessUnderService( DWORD dwProcessId );
BOOL DeviceIoQueryPermission(ULONG ProcessId, ULONG *Permission);
BOOL CloseBluefishDriver();

extern GingkoPasswordTable __gPasswordTable;
extern HANDLE __gPasswordEventHandle;
extern BOOL	  __gApplicationExit;

#endif
