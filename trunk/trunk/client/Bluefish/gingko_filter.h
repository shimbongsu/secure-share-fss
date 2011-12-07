#ifndef __GINGKO_FILTER_H__
#define __GINGKO_FILTER_H__
#include <ntifs.h>
#include "gingko_lib.h"

#define HAS_SET_GINGKO_PERMISSION   0x00000010L
typedef enum _GINGKO_DEBUG_FLAGS {

    GINGKO_DEBUG_DISPLAY_ATTACHMENT_NAMES       = 0x00000001,
    GINGKO_DEBUG_ERROR                          = 0x00000002,
    GINGKO_DEBUG_TRACE_NAME_REQUESTS            = 0x00000004,
    GINGKO_DEBUG_TRACE_IRP_OPS                  = 0x00000010,
    GINGKO_DEBUG_TRACE_FAST_IO_OPS              = 0x00000020,
    GINGKO_DEBUG_TRACE_FSFILTER_OPS             = 0x00000040,
    GINGKO_DEBUG_TRACE_CONTEXT_OPS              = 0x00000100,
    GINGKO_DEBUG_TRACE_DETAILED_CONTEXT_OPS     = 0x00000200,
    GINGKO_DEBUG_TRACE_MISMATCHED_NAMES         = 0x00001000,
    GINGKO_DEBUG_ASSERT_MISMATCHED_NAMES        = 0x00002000,
    GINGKO_DEBUG_BREAK_ON_DRIVER_ENTRY          = 0x80000000
} GINGKO_DEBUG_FLAGS;



typedef enum _CONTROL_DEVICE_STATE {

    OPENED,
    CLOSED,
    CLEANING_UP

} CONTROL_DEVICE_STATE;


typedef struct _GINGKO_COMPLETION_CONTEXT {

    unsigned int SendBack;

} GINGKO_COMPLETION_CONTEXT, *PGINGKO_COMPLETION_CONTEXT;

typedef struct _GINGKO_COMPLETION_CONTEXT_W2K {

    GINGKO_COMPLETION_CONTEXT;
    WORK_QUEUE_ITEM WorkItem;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    PDEVICE_OBJECT NewDeviceObject;
	KEVENT WaitEvent;

} GINGKO_COMPLETION_CONTEXT_W2K, *PGINGKO_COMPLETION_CONTEXT_W2K;

typedef struct _GINGKO_DEVICE_IO_CONTROL{
	ULONGLONG ByteOffset;
	ULONG     Length;
	LONG_PTR	  FileObject;
} GINGKO_DEVICE_READ, GINGKO_DEVICE_WRITE, *PGINGKO_DEVICE_READ, *PGINGKO_DEVICE_WRITE;

#if WINVER >= 0x0501
typedef struct _GINGKO_COMPLETION_CONTEXT_WXP_OR_LATER {

    GINGKO_COMPLETION_CONTEXT;
    
    KEVENT WaitEvent;

} GINGKO_COMPLETION_CONTEXT_WXP_OR_LATER, *PGINGKO_COMPLETION_CONTEXT_WXP_OR_LATER;
#endif


#define DEFAULT_GINGKO_DEBUG_LEVEL     GINGKO_DEBUG_DISPLAY_ATTACHMENT_NAMES;

#define GINGKO_ATTACH_ON_DEMAND    1   
    //  Filespy will only attach to a volume when a user asks to start logging 
    //  that volume.                                        
#define GINGKO_ATTACH_ALL_VOLUMES  2

#define DEVICE_NAMES_SZ  100

#define MAX_BUFFERS     100

#define USER_NAMES_SZ   64

#ifndef MAX_PATH
#define MAX_PATH        384
#endif

#define MAX_NAME_SPACE  (MAX_PATH*sizeof(WCHAR))

#define MAX(_a_, _b_) (_a_ > _b_ ? _a_ : _b_);
#define MIN(_a_, _b_) (_a_ < _b_ ? _a_ : _b_);
//
//  Size of the actual records with the name built in.
//

#define RECORD_SIZE     1024

#define WAIT_SECONDS(x) ((x)*10000000)

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)


#define IS_GINGKO_DEVICE_OBJECT( _devObj )                               \
    (((_devObj) != NULL) &&                                               \
     ((_devObj)->DriverObject == gGingkoDriverObject) &&                 \
     ((_devObj)->DeviceExtension != NULL))


#define IS_GINGKO_CONTROL_DEVICE_OBJECT(_devObj) \
    (((_devObj) == gControlDeviceObject) ? \
            (ASSERT(((_devObj)->DriverObject == gGingkoDriverObject) && \
                    ((_devObj)->DeviceExtension == NULL)), TRUE) : \
            FALSE)

typedef enum _FGINGKO_DEV_FLAGS {

    //
    //  If set, this is an attachment to a volume device object, 
    //  If not set, this is an attachment to a file system control device
    //  object.
    //

    IsVolumeDeviceObject = 0x00000001,

    //
    //  If set, logging is turned on for this device
    //

    LogThisDevice = 0x00000002,

    //
    //  If set, contexts are initialized
    //

    ContextsInitialized = 0x00000004,
    
    //
    //  If set, this is linked into the extension list
    //

    ExtensionIsLinked = 0x00000008

} FGINGKO_DEV_FLAGS;


typedef struct _GINGKO_PROCESS_LIST
{
	ULONG ProcessId;
	ULONG ThreadId;
	LIST_ENTRY NextEntry;
} GINGKO_PROCESS_LIST, *PGINGKO_PROCESS_LIST;

//
// Define the device extension structure that the FileSpy driver
// adds to each device object it is attached to.  It stores
// the context FileSpy needs to perform its logging operations on
// a device.
//

typedef struct _GINGKO_DEVICE_EXTENSION {

    //
    //  Device Object this extension is attached to
    //

    PDEVICE_OBJECT ThisDeviceObject;

    //
    //  Device object this filter is directly attached to
    //

    PDEVICE_OBJECT AttachedToDeviceObject;

    //
    //  When attached to Volume Device Objects, the physical device object
    //  that represents that volume.  NULL when attached to Control Device
    //  objects.
    //

    PDEVICE_OBJECT DiskDeviceObject;

    //
    //  Linked list of devices we are attached to
    //

    LIST_ENTRY NextFileSpyDeviceLink;

    //
    //  Flags for this device
    //

    FGINGKO_DEV_FLAGS Flags;

    //
    //  Linked list of contexts associated with this volume along with the
    //  lock.
    //

    LIST_ENTRY CtxList;

    ERESOURCE CtxLock;


	KEVENT     CreateEvent;

	LIST_ENTRY ListEntry;

	KSPIN_LOCK ListSpinLock;

	LIST_ENTRY ProcessListEntry;

	KSPIN_LOCK ProcessListSpinLock;

	PKTHREAD	pSysThread;

	BOOLEAN    ShouldExitThread;

	KSEMAPHORE RequestSemaphore;

    //
    //  When renaming a directory there is a window where the current names
    //  in the context cache may be invalid.  To eliminate this window we
    //  increment this count every time we start doing a directory rename 
    //  and decrement this count when it is completed.  When this count is
    //  non-zero then we query for the name every time so we will get a
    //  correct name for that instance in time.
    //

    ULONG AllContextsTemporary;

    //
    //  Name for this device.  If attached to a Volume Device Object it is the
    //  name of the physical disk drive.  If attached to a Control Device
    //  Object it is the name of the Control Device Object.
    //

    UNICODE_STRING DeviceName;

    //
    // Names the user used to start logging this device
    //

    UNICODE_STRING UserNames;

    //
    //  Buffers used to hold the above unicode strings
    //  Note:  We keep these two forms of the name so that we can build
    //         a nicer looking name when we are printing out file names.
    //         We want just the "c:" type device name at the beginning
    //         of a file name, not "\device\hardiskVolume1".
    //

    WCHAR DeviceNameBuffer[DEVICE_NAMES_SZ];

    WCHAR UserNamesBuffer[USER_NAMES_SZ];

} GINGKO_DEVICE_EXTENSION, *PGINGKO_DEVICE_EXTENSION;



//extern GINGKO_DEBUG_FLAGS gGingkoDebugLevel;
extern ULONG gGingkoAttachMode;

extern PDEVICE_OBJECT gControlDeviceObject;
extern PDRIVER_OBJECT gGingkoDriverObject;

extern FAST_MUTEX gGingkoDeviceExtensionListLock;
extern LIST_ENTRY gGingkoDeviceExtensionList;

extern CONTROL_DEVICE_STATE gControlDeviceState;
extern KSPIN_LOCK gControlDeviceStateLock;

extern SharedNotificationPtr gGingkoNotification;

extern const PCHAR DeviceTypeNames[];
extern ULONG SizeOfDeviceTypeNames;
extern BOOLEAN gGingkoServerStarted;

extern KSPIN_LOCK gGingkoObjectSpinLock;
extern GINGKO_OBJECT gGingkoObjectList;
extern FAST_MUTEX gGingkoFastMutex;
extern KSPIN_LOCK gOutputBufferLock;
extern LIST_ENTRY gOutputBufferList;

extern NPAGED_LOOKASIDE_LIST gFreeBufferList;


extern BOOLEAN gWasHooked;

extern LONG gStaticBufferInUse;
extern CHAR gOutOfMemoryBuffer[RECORD_SIZE];

extern FAST_MUTEX gGingkoAttachLock;


//VOID
//GingkoFsControlLoadFileSystemCompleteWorker (
//    IN PGINGKO_COMPLETION_CONTEXT_W2K Context
//    );
//
//VOID
//GingkoFsControlMountVolumeCompleteWorker (
//    IN PGINGKO_COMPLETION_CONTEXT_W2K Context
//    );

NTSTATUS
GingkoFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    );

PVOID GetReadBuffer( PIRP Irp, SIZE_T SizeOfBytes, PMDL* MdlAddress, BOOLEAN *AllocatedMapAddress  );

NTSTATUS
GingkoRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
GingkoQueryInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoQueryDirectoryInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoSetInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GingkoCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BluefisherUnload (
    IN PDRIVER_OBJECT DriverObject
    );

VOID GingkoImageLoadNotify(IN PUNICODE_STRING  FullImageName,
                       IN HANDLE  ProcessId, // Process where image is mapped
					   IN PIMAGE_INFO  ImageInfo);



#endif //End Of__GINGKO_FILTER_H__
