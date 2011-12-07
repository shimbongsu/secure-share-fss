#ifndef __GINGKO_LIB_H__
#define __GINGKO_LIB_H__


#include "gingko_ioqueue.h"

#define GINGKO_DRIVER_NAME      L"BLUEFISH.SYS"
#define GINGKO_DEVICE_NAME      L"Bluefisher"
#define GINGKO_W32_DEVICE_NAME  L"\\\\.\\Bluefisher"
#define GINGKO_DOSDEVICE_NAME   L"\\DosDevices\\Bluefisher"
#define GINGKO_FULLDEVICE_NAME1 L"\\FileSystem\\Filters\\Bluefisher"
#define GINGKO_FULLDEVICE_NAME2 L"\\FileSystem\\BluefisherCDO"

#define GINGKO_POOL_TAG        'ptGK'          //misc POOL allocations

#define DEFAULT_MAX_RECORDS_TO_ALLOCATE 100;
#define DEFAULT_MAX_NAMES_TO_ALLOCATE   100;
#define GINGKO_SESSION_MAX_BUFFER       128
#define MAXSIZE_PRIVATE_KEY				1024
#define OPERATION_NAME_BUFFER_SIZE 80


#define GET_DEVICE_TYPE_NAME( _type ) \
            ((((_type) > 0) && ((_type) < (SizeOfDeviceTypeNames / sizeof(PCHAR)))) ? \
                DeviceTypeNames[ (_type) ] : \
                "[Unknown]")

#define IS_SUPPORTED_DEVICE_TYPE(_type)					\
     ((_type) == FILE_DEVICE_DISK_FILE_SYSTEM)		||	\
     ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM)	||	\
	 ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM)	||	\
	 ((_type) == FILE_DEVICE_DFS_FILE_SYSTEM)		||	\
	 ((_type) == FILE_DEVICE_MAILSLOT)				|| \
	 ((_type) == FILE_DEVICE_NAMED_PIPE)			|| \
	 ((_type) == FILE_DEVICE_PARALLEL_PORT)			|| \
	 ((_type) == FILE_DEVICE_PRINTER)

	// ((_type) == FILE_DEVICE_SERIAL_PORT) || \
	// ((_type) == FILE_DEVICE_STREAMS) || \
	// ((_type) == FILE_DEVICE_8042_PORT) || \
	// ((_type) == FILE_DEVICE_NETWORK) || \
	// ((_type) == FILE_DEVICE_TRANSPORT) 

	//((_type) == FILE_DEVICE_MAILSLOT) || \
	// ((_type) == FILE_DEVICE_NAMED_PIPE) || \
	// ((_type) == FILE_DEVICE_PARALLEL_PORT) || \
	// ((_type) == FILE_DEVICE_PRINTER) || \




#define GINGKO_METHOD_LENGTH		5
#define GINGKO_IDENTIFIER_LENGTH	5
#define GINGKO_VERSION_LENGTH		2
#define MD5_DIGEST_LENGTH			16
#define COMPANY_LENGTH				6
#define FILE_RESERVED_BYTES			448
#define FILE_RESERVED_NORMAL		10
#ifndef UINT
#define UINT unsigned int
#endif

#ifndef THREAD_SET_CONTEXT
#define THREAD_SET_CONTEXT 0x0010
#endif

//typedef struct _GINGKO_HEADER{
//	unsigned char Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
//	unsigned char Version[GINGKO_VERSION_LENGTH];			///first char is the major version, second char is the minor version. the version struct is major.minor
//	ULONG		  SizeOfHeader;										///the size of the file header.
//	ULONGLONG	  SizeOfFile;									///the file size of file
//	unsigned char Company[COMPANY_LENGTH];				///The organization code 
//	unsigned char Md5[MD5_DIGEST_LENGTH];					///md5 of this file.
//	unsigned char Method[GINGKO_METHOD_LENGTH];				///engine of crypto //RSA#1
//	UINT		  SizeOfBlock;										///one block of size should to take to decrypt. the size should be depend to the method of crypto
//	ULONG         Reserved;											///Reserved
//	byte          ReservedBytes[FILE_RESERVED_BYTES];
//} GINGKO_HEADER, *PGINGKO_HEADER;

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


typedef struct _GINGKO_FILE_DESCRIPTOR
{
	PFILE_OBJECT   FileObject;
	ULONG		   AddressOfFilemap;
	UNICODE_STRING FileName;
	PKEVENT		   WaitEvent;
}GINGKO_FILE_DESCRIPTOR, *PGINGKO_FILE_DESCRIPTOR;

typedef struct _GINGKO_OBJECT
{
	//GINGKO_FILE_DESCRIPTOR GingkoDescriptor;
	PFILE_OBJECT		   FileObject;
	UNICODE_STRING		   FileName;
	GINGKO_HEADER          GingkoHeader;
	LARGE_INTEGER		   OffsetBytes;
	ULONG				   dwProcessId;
	EncryptedIoQueue	   Queue;
	LIST_ENTRY			   ListEntry;
	ULONG				   Permission;  ///(__owner ? 0x0F000000) | (__holder ? 0x00F00000) | (__read ? 0x000F0000 ) | (__write ? 0x0000F000 ) |  (__print ? 0x00000F00 ) | (__delete ? 0x000000F0) 
	struct _GINGKO_OBJECT *pNext;
}GINGKO_OBJECT, *PGINGKO_OBJECT;


typedef struct {
	HANDLE				FileObject;
	HANDLE				RequestKeyEvent;
	ULONG				dwProcessId;
	unsigned char		Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
	unsigned char		Version[GINGKO_VERSION_LENGTH];				///first char is the major version, second char is the minor version. the version struct is major.minor
	ULONG				SizeOfHeader;								///the size of the file header.
	ULONGLONG			SizeOfFile;									///the file size of file
	unsigned char		Company[COMPANY_LENGTH];					///The organization code 
	unsigned char		Md5[MD5_DIGEST_LENGTH];						///md5 of this file.
	unsigned char		Method[GINGKO_METHOD_LENGTH];				///engine of crypto //RSA#1
	unsigned char		FileReserved[FILE_RESERVED_BYTES];			///Reserved Part of File, it's very imports
	UINT SizeOfBlock;												///one block of size should to take to decrypt. the size should be depend to the method of crypto
	ULONG Reserved;													///Reserved
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


typedef struct _GINGKO_PROCESS_HOOK_LIST
{

	HANDLE ProcessId;
	BOOLEAN WasHooked;
	BOOLEAN WasInit;
	struct _GINGKO_PROCESS_HOOK_LIST *NextHook;

}GINGKO_PROCESS_HOOK_LIST, *PGINGKO_PROCESS_HOOK_LIST;

typedef NTSTATUS (__stdcall *GingkoCallback)(
      IN PGINGKO_OBJECT pGingkoObject,
	  IN ULONGLONG Offset,
	  IN ULONG ReqLength,
	  OUT PVOID Buffer, 
	  OUT PULONG ActualLength
    );

typedef struct _GINGKO_FILE_OBJECT_PERMISSION{
	PFILE_OBJECT FileObject;
	BOOLEAN		 Owner;
	BOOLEAN		 Holder;
	BOOLEAN		 Deleted;
	BOOLEAN		 Copyable;
	BOOLEAN		 Writable;
	BOOLEAN		 Printable;
	BOOLEAN		 Readable;
	BOOLEAN		 Transable;
}GINGKO_FILE_OBJECT_PERMISSION, *PGINGKO_FILE_OBJECT_PERMISSION;

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

typedef struct _GINGKO_APC_PARAMETER
{
	WCHAR		FileName[262];			///the filename of which we are opening for reading.
	ULONG		FileNameLength;			///the length of filename 
	LONG_PTR	FileObject;				///The FileObject
	ULONG		hProcessId;				///ProcessId;
	ULONG		WaitEvent;				///WaitEvent
	ULONG		Size;					//The size of this structs.
	BOOLEAN		bHooked;				///WAS HOOKED.
	UCHAR		Company[8];				///The Company Id
	UCHAR		Md5[16];				///md5 length
	PVOID		OutputBuffer;			///the output buffer
	ULONGLONG	Offset;					///the offset of reading 
	ULONG		BufferLength;			///the size of output buffer
	ULONG		Length;					///require to reading the length of file data
	ULONG		OutputLength;			///the actually output Length, the value should zero when driver call.
	ULONG		Status;					///
}GINGKO_APC_PARAMETER, *PGINGKO_APC_PARAMETER;



typedef struct _GINGKO_SHARED_NOTIFICATION
{
	ULONG  cbStruct;
	HANDLE WriteEvent;              ////Write Event. WaitForSingleObject will called by this value. 
	HANDLE ReadEvent;				////Read Event,  WaitForSingleObject it called by 
	HANDLE ServerProcess;
	HANDLE ClientProcess;
	HANDLE ServerThread;
	ULONG_PTR GingkoDecryptBuffer;
} SharedNotification, *SharedNotificationPtr;

typedef struct _GINGKO_WORKITEM_CONTEXT
{
	PGINGKO_APC_PARAMETER Parameter;
	PKEVENT				  pKevent;
	PIRP				  Irp;
}GINGKO_WORKITEM_CONTEXT, *PGINGKO_WORKITEM_CONTEXT;

typedef struct _GET_NAME_CONTROL {
    PCHAR AllocatedBuffer;
    //CHAR  smallBuffer[512];
} GET_NAME_CONTROL, *PGET_NAME_CONTROL;

typedef struct _GINGKO_EVENT{
	PHANDLE		  Handler;
	LONG		  Status;
	LONG		  OutputLength;
	LONG		  Hooked;
	LONG		  Reserved;
	unsigned char *Buffer;
}GINGKO_EVENT, *PGINGKO_EVENT;


NTSTATUS GingkoFileFlushCache( PFILE_OBJECT FileObject );

NTSTATUS HandlePagingIoPassThrough( PDEVICE_OBJECT DeviceObject, PIRP Irp, LONGLONG  ActualSize, PCRYPTO_INFO CryptoInfo );

BOOLEAN FindCryptoInfo( PGINGKO_HEADER Header, PCRYPTO_INFO *pRevCryptoInfo );

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
GingkoDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
GingkoCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoFilterFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoDeviceIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
GingkoFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    );

NTSTATUS
GingkoFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoFsControlCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
GingkoFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
GingkoCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    );


VOID
GingkoCloseControlDevice (
    );

#if WINVER >= 0x0501 /* See comment in DriverEntry */

NTSTATUS
GingkoPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

VOID
GingkoPostFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );

#endif


/**
 * Support function
 * 
 */
VOID
GingkoReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    );

VOID
GingkoCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501
VOID
GingkoLoadDynamicFunctions (
    );

VOID
GingkoGetCurrentVersion (
    );
#endif

#if WINVER >= 0x0501
//
//  MULTIVERSION NOTE:
//
//  If built in the Windows XP environment or later, we will dynamically import
//  the function pointers for routines that were not supported on Windows 2000
//  so that we can build a driver that will run, with modified logic, on 
//  Windows 2000 or later.
//
//  Below are the prototypes for the function pointers that we need to 
//  dynamically import because not all OS versions support these routines.
//

typedef
NTSTATUS
(*PGINGKO_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS) (
    IN PDRIVER_OBJECT DriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    );

typedef
NTSTATUS
(*PGINGKO_ENUMERATE_DEVICE_OBJECT_LIST) (
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT *DeviceObjectList,
    IN  ULONG DeviceObjectListSize,
    OUT PULONG ActualNumberDeviceObjects
    );

typedef
NTSTATUS
(*PGINGKO_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE) (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

typedef    
PDEVICE_OBJECT
(*PGINGKO_GET_LOWER_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  DeviceObject
    );

typedef
PDEVICE_OBJECT
(*PGINGKO_GET_DEVICE_ATTACHMENT_BASE_REF) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PGINGKO_GET_DISK_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    );

typedef
PDEVICE_OBJECT
(*PGINGKO_GET_ATTACHED_DEVICE_REFERENCE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PGINGKO_GET_VERSION) (
    IN OUT PRTL_OSVERSIONINFOW VersionInformation
    );

typedef struct _GINGKO_DYNAMIC_FUNCTION_POINTERS {

    PGINGKO_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS RegisterFileSystemFilterCallbacks;
    PGINGKO_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE AttachDeviceToDeviceStackSafe;
    PGINGKO_ENUMERATE_DEVICE_OBJECT_LIST EnumerateDeviceObjectList;
    PGINGKO_GET_LOWER_DEVICE_OBJECT GetLowerDeviceObject;
    PGINGKO_GET_DEVICE_ATTACHMENT_BASE_REF GetDeviceAttachmentBaseRef;
    PGINGKO_GET_DISK_DEVICE_OBJECT GetDiskDeviceObject;
    PGINGKO_GET_ATTACHED_DEVICE_REFERENCE GetAttachedDeviceReference;
    PGINGKO_GET_VERSION GetVersion;

} GINGKO_DYNAMIC_FUNCTION_POINTERS, *PGINGKO_DYNAMIC_FUNCTION_POINTERS;

extern GINGKO_DYNAMIC_FUNCTION_POINTERS gGingkoDynamicFunctions;

//
//  MULTIVERSION NOTE: For this version of the driver, we need to know the
//  current OS version while we are running to make decisions regarding what
//  logic to use when the logic cannot be the same for all platforms.  We
//  will look up the OS version in DriverEntry and store the values
//  in these global variables.
//

extern ULONG gGingkoOsMajorVersion;
extern ULONG gGingkoOsMinorVersion;

//
//  Here is what the major and minor versions should be for the various OS versions:
//
//  OS Name                                 MajorVersion    MinorVersion
//  ---------------------------------------------------------------------
//  Windows 2000                             5                 0
//  Windows XP                               5                 1
//  Windows Server 2003                      5                 2
//

#define IS_WINDOWSXP_OR_LATER() \
    (((gGingkoOsMajorVersion == 5) && (gGingkoOsMinorVersion >= 1)) || \
     (gGingkoOsMajorVersion > 5))

#endif

//#define GINGKO_LOG_PRINT( _dbgLevel, _string )                 \
//    (FlagOn(gGingkoDebugLevel,(_dbgLevel)) ?               \
//        DbgPrint _string  :                                 \
//        ((void)0))

VOID
GingkoInitNamingEnvironment(
    VOID
    );

VOID
GingkoCleanupDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
GingkoGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );

/**
 * Debug Support Functions
 */
typedef enum {

    CHECK_IF_POSSIBLE = 1,
    READ,
    WRITE,
    QUERY_BASIC_INFO,
    QUERY_STANDARD_INFO,
    LOCK,
    UNLOCK_SINGLE,
    UNLOCK_ALL,
    UNLOCK_ALL_BY_KEY,
    DEVICE_CONTROL,
    DETACH_DEVICE,
    QUERY_NETWORK_OPEN_INFO,
    MDL_READ,
    MDL_READ_COMPLETE,
    MDL_WRITE,
    MDL_WRITE_COMPLETE,
    READ_COMPRESSED,
    WRITE_COMPRESSED,
    MDL_READ_COMPLETE_COMPRESSED,
    PREPARE_MDL_WRITE,
    MDL_WRITE_COMPLETE_COMPRESSED,
    QUERY_OPEN,

    FASTIO_MAX_OPERATION=QUERY_OPEN
} FASTIO_TYPE/*, *PFASTIO_TYPE*/;


VOID
DebugDumpIrpOperation (
    IN BOOLEAN InOriginatingPath,
    IN PIRP Irp
    );

VOID
DebugDumpFastIoOperation (
    IN BOOLEAN InPreOperation,
    IN FASTIO_TYPE FastIoOperation
    );

#if WINVER >= 0x0501 /* See comment in DriverEntry */
VOID
DebugDumpFsFilterOperation (
    IN BOOLEAN InPreOperationCallback,
    IN PFS_FILTER_CALLBACK_DATA Data
    );
#endif

NTSTATUS
GingkoAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );
VOID
GingkoGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
GingkoInitDeviceNamingEnvironment( 
	IN PDEVICE_OBJECT DeviceObject 
	);
////////////////////////////////////////////////////////////////////////
//
//       Attaching/detaching to all volumes in system routines
//                  implemented in gingko_lib_fun.c
//
////////////////////////////////////////////////////////////////////////

NTSTATUS
GingkoAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
GingkoDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501
NTSTATUS
GingkoEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    );
#endif


////////////////////////////////////////////////////////////////////////
//
//         Common attachment and detachment routines
//              implemented in fspylib.c
//
////////////////////////////////////////////////////////////////////////

NTSTATUS 
GingkoIsAttachedToDeviceByUserDeviceName (
    IN PUNICODE_STRING DeviceName,
    IN OUT PBOOLEAN IsAttached,
    IN OUT PDEVICE_OBJECT *StackDeviceObject,
    IN OUT PDEVICE_OBJECT *OurAttachedDeviceObject
    );

BOOLEAN
GingkoIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );

BOOLEAN
GingkoIsAttachedToDeviceW2K (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );


#if WINVER >= 0x0501
BOOLEAN
GingkoIsAttachedToDeviceWXPAndLater (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );
#endif

NTSTATUS
GingkoAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT FilespyDeviceObject
    );

VOID
GingkoCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    );


//////////////////////////////////////////////////////////////////////////////////////////
//////
////// Gingko Implements Function Which will be used to check the File is encrypted by Gingko
//////
//////////////////////////////////////////////////////////////////////////////////////////
extern LIST_ENTRY		gGingkoWriteFileListEntry;
extern KSPIN_LOCK		gGingkoWriteFileSpinLock;


extern LIST_ENTRY		gGingkoPublicKeyRequestQueue;
extern KSPIN_LOCK		gGingkoPublicKeyRequestLock;
extern KEVENT			gAcquireRequestKeyEvent; 

BOOLEAN FindWriteFileObject( PFILE_OBJECT FileObject, ULONG dwProcessId, PGINGKO_OBJECT *pRevObject, BOOLEAN Remove );

BOOLEAN FindGingkoObjectByFileName( WCHAR *pFileName, ULONG FileNameLength, ULONG dwProcessId, PGINGKO_OBJECT *pRevObject );
/***
 * Check the FileObject with the first part of read buffer includes the Gingko Header information.
 *
 */

BOOLEAN HasGingkoIdentifier(
			PGINGKO_HEADER Header
		);


PUNICODE_STRING GetFileObjectFullName( PFILE_OBJECT FileObject, PUNICODE_STRING pUnicode );

/**
 * If the pointer of FileObject is been added to the GingkoObject list, then return true. else return false.
 *
 **/
//BOOLEAN IsGingkoEncryptedFile( 
//		    PFILE_OBJECT FileObject,
//			PGINGKO_OBJECT *pObject );

/**
 * Add the FileObject and the GingkoHeader to the list.
 *
 **/
//PGINGKO_OBJECT AddGingkoFileObjectToTail( 
//			PFILE_OBJECT FileObject, 
//			PGINGKO_HEADER pHeader );

/***
 * Release the GingkoList.
 */
VOID	ReleaseGingkoList();

/***
 * Initialize the Gingko list
 **/
VOID	InitializeGingkoList();

/***
 * Delete the GingkoObject by FileObject.
 */
BOOLEAN DeleteGingkoObject(
			PFILE_OBJECT FileObject);


/**
 * THE GINGKO PROCESS PROCESS
 */
BOOLEAN SetAndHookProcess( HANDLE hProcessId, BOOLEAN bHooked );
BOOLEAN CheckProcessToHook( HANDLE hProcessId, PBOOLEAN pWasHooked  );
BOOLEAN SetAndHookProcess( HANDLE hProcessId, BOOLEAN bHooked );
BOOLEAN RemoveHookedProcess( HANDLE processId );
BOOLEAN RemoveAllHookedProcess();

NTSTATUS GingkoInitializePublicKeyRequestQueue();

/**
 ** Decryption Process Table
 **
 ***/
#define MAX_PROCESS_TABLE	120

typedef struct _FILE_CRYPTO_BLOCK
{
	unsigned char FileHash[MD5_DIGEST_LENGTH];
	unsigned char CryptoInfo[MAXSIZE_PRIVATE_KEY];
	struct _FILE_CRYPTO_BLOCK *Next;
} FileCryptoBlock;

typedef struct _DESCRYPTED_PROCESSES{
	ULONG			 ProcessesId[2];
	FileCryptoBlock *FileCryptoBlock;
	struct _DESCRYPTED_PROCESSES *Next;
} DecryptedProcessesTable;

extern DecryptedProcessesTable *gDecryptedProcessesTable;
extern KMUTEX				    gDecryptedProcessesMutex;	

#define IsProcessUnderDecryption(ProcessId) (0x0FFFFFFF != GetPermissionUnderDecryptionByProcessId( ProcessId, NULL ))

NTSTATUS GingkoInitializeDecryptProcessTable();
NTSTATUS GingkoUnInitializeDecryptProcessTable();
//Check the ProcessId whether is under in decryption process.
ULONG GetPermissionUnderDecryptionByProcessId( ULONG ProcessId, KIRQL* irql );
// Remove the ProcessId
NTSTATUS RemoveProcessUnderDecryption( ULONG ProcessId );
//
// Update Process's permission
//
//ULONG UpdateProcessPermission( ULONG ProcessId, ULONG Permission );
ULONG UpdateProcessPermission(  ULONG ProcessId, ULONG Permission, char* fileHash, char* cryptoKeys, KIRQL* irql );

//NTSTATUS GingkoAddPublicKeyRequest( 
//	ULONG		ProcessId,				///ProcessId;
//	UCHAR*		Company,				///The Company Id
//	UCHAR*		Md5				///md5 length
//);

//NTSTATUS GingkoBuildApcParameter( 
//		    IN PGINGKO_OBJECT pGingkoObject,
//			IN ULONGLONG Offset,
//			IN ULONG	 Length,
//			IN PVOID	 Buffer OPTIONAL,
//			OUT PGINGKO_APC_PARAMETER *Parameter,
//			OUT PVOID *pAddress,
//			OUT PVOID	*pTempLocked,
//			OUT PMDL	*MdlAddress);

BOOLEAN IsGingkoServerProcess();

LONG GingkoDereferenceSharedNotification(SharedNotificationPtr pg);
SharedNotificationPtr GingkoReferenceSharedNotification();
//NTSTATUS GingkoRequest(
//		    IN PGINGKO_OBJECT pGingkoObject, 
//			IN ULONG ProcessId,
//			IN ULONG ThreadId,
//			IN  PIRP Irp,
//			OUT PVOID Buffer, 
//			OUT ULONG *OutLength,
//			OUT ULONG *pStatus,
//			IN ULONG Length, 
//			IN PLARGE_INTEGER Offset);

BOOLEAN PutFileObjectPermission(PFILE_OBJECT FileObject, 
							 BOOLEAN Owner, BOOLEAN Holder, BOOLEAN IsDeleted, BOOLEAN Readable, 
							 BOOLEAN Writable, BOOLEAN Printable, BOOLEAN Copyable, BOOLEAN Trans 
							 );

BOOLEAN RemoveAllFileObjectQueue();

BOOL FindCryptoPermission(ULONG ProcessId, char* fileHash, 
						  ULONG *Permission, 
						  unsigned char* CryptoKeysBuf, 
						  KIRQL *irql );
size_t GetCpuCount ();


#endif
