#include <ntifs.h>
#include <stdlib.h>
#include "gingko_filter.h"
#include "gingko_fastio.h"
#include "gingko_debug.h"
#include "gingko_redfish.h"



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, BluefisherUnload)
#pragma alloc_text(PAGE, GingkoFsNotification)
#pragma alloc_text(PAGE, GingkoCreate)
#pragma alloc_text(PAGE, GingkoClose)
#pragma alloc_text(PAGE, GingkoRead)
#pragma alloc_text(PAGE, GingkoWrite)
#pragma alloc_text(PAGE, GingkoCleanup)
#pragma alloc_text(PAGE, GingkoQueryInformation)
#pragma alloc_text(PAGE, GingkoSetInformation)
#pragma alloc_text(PAGE, GingkoQueryDirectoryInformation)
#pragma alloc_text(PAGE, GingkoFsControlMountVolume)
#pragma alloc_text(PAGE, GingkoFsControlMountVolumeComplete)
#pragma alloc_text(PAGE, GingkoFsControlLoadFileSystem)
#pragma alloc_text(PAGE, GingkoFsControlLoadFileSystemComplete)
#pragma alloc_text(PAGE, GingkoAttachDeviceToDeviceStack)
#pragma alloc_text(PAGE, GingkoAttachToFileSystemDevice)
#pragma alloc_text(PAGE, GingkoDetachFromFileSystemDevice)
#pragma alloc_text(PAGE, GingkoAttachToMountedDevice)
#pragma alloc_text(PAGE, GingkoIsAttachedToDevice)
#pragma alloc_text(PAGE, GingkoIsAttachedToDeviceW2K)
#pragma alloc_text(INIT, GingkoReadDriverParameters)

#pragma alloc_text(PAGE, SfFastIoCheckIfPossible)
#pragma alloc_text(PAGE, SfFastIoRead)
#pragma alloc_text(PAGE, SfFastIoWrite)
#pragma alloc_text(PAGE, SfFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, SfFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, SfFastIoLock)
#pragma alloc_text(PAGE, SfFastIoUnlockSingle)
#pragma alloc_text(PAGE, SfFastIoUnlockAll)
#pragma alloc_text(PAGE, SfFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, SfFastIoDeviceControl)
#pragma alloc_text(PAGE, SfFastIoDetachDevice)
#pragma alloc_text(PAGE, SfFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, SfFastIoMdlRead)
#pragma alloc_text(PAGE, SfFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, SfFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, SfFastIoReadCompressed)
#pragma alloc_text(PAGE, SfFastIoWriteCompressed)
#pragma alloc_text(PAGE, SfFastIoQueryOpen)
#endif

//ULONG gGingkoDebugLevel = DEFAULT_GINGKO_DEBUG_LEVEL;
#if WINVER >= 0x0501
ULONG gGingkoAttachMode =  GINGKO_ATTACH_ALL_VOLUMES;
#else
ULONG gGingkoAttachMode = GINGKO_ATTACH_ON_DEMAND;
#endif

PDEVICE_OBJECT gControlDeviceObject;
PDRIVER_OBJECT gGingkoDriverObject;

NTSTATUS GingkoAddDevice(
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

VOID GingkoProcessCreationMonitor(
	IN HANDLE ParentId,
	IN HANDLE ProcessId,
	IN BOOLEAN Create
);

#if WINVER >= 0x0501
//
//  The structure of function pointers for the functions that are not available
//  on all OS versions.
//

GINGKO_DYNAMIC_FUNCTION_POINTERS gGingkoDynamicFunctions = {0};

ULONG gGingkoOsMajorVersion = 0;
ULONG gGingkoOsMinorVersion = 0;
#endif


CONTROL_DEVICE_STATE gControlDeviceState = CLOSED;

KSPIN_LOCK gControlDeviceStateLock;


FAST_MUTEX gGingkoFastMutex;

KSPIN_LOCK gGingkoReadSpinLock;

// NOTE:  Like the gControlDeviceStateLock, gOutputBufferLock MUST be a spinlock
//   since we try to acquire it during the completion path in SpyLog, which 
//   could be called at DISPATCH_LEVEL (only KSPIN_LOCKs can be acquired at 
//   DISPATCH_LEVEL).
//
KSPIN_LOCK gOutputBufferLock;
LIST_ENTRY gOutputBufferList;

//
//  This lock is used to synchronize our attaching to a given device object.
//  This lock fixes a race condition where we could accidently attach to the
//  same device object more then once.  This race condition only occurs if
//  a volume is being mounted at the same time as this filter is being loaded.
//  This problem will never occur if this filter is loaded at boot time before
//  any file systems are loaded.
//
//  This lock is used to atomically test if we are already attached to a given
//  device object and if not, do the attach.
//

FAST_MUTEX gGingkoAttachLock;


#ifndef MEMORY_DBG
NPAGED_LOOKASIDE_LIST gFreeBufferList;
#endif



LONG gMaxRecordsToAllocate = DEFAULT_MAX_RECORDS_TO_ALLOCATE;
LONG gRecordsAllocated = 0;

LONG gMaxNamesToAllocate = DEFAULT_MAX_NAMES_TO_ALLOCATE;
LONG gNamesAllocated = 0;

LONG gStaticBufferInUse = FALSE;

FAST_MUTEX gGingkoDeviceExtensionListLock;
LIST_ENTRY gGingkoDeviceExtensionList;

const PCHAR DeviceTypeNames[] = {
    "",
    "BEEP",
    "CD_ROM",
    "CD_ROM_FILE_SYSTEM",
    "CONTROLLER",
    "DATALINK",
    "DFS",
    "DISK",
    "DISK_FILE_SYSTEM",
    "FILE_SYSTEM",
    "INPORT_PORT",
    "KEYBOARD",
    "MAILSLOT",
    "MIDI_IN",
    "MIDI_OUT",
    "MOUSE",
    "MULTI_UNC_PROVIDER",
    "NAMED_PIPE",
    "NETWORK",
    "NETWORK_BROWSER",
    "NETWORK_FILE_SYSTEM",
    "NULL",
    "PARALLEL_PORT",
    "PHYSICAL_NETCARD",
    "PRINTER",
    "SCANNER",
    "SERIAL_MOUSE_PORT",
    "SERIAL_PORT",
    "SCREEN",
    "SOUND",
    "STREAMS",
    "TAPE",
    "TAPE_FILE_SYSTEM",
    "TRANSPORT",
    "UNKNOWN",
    "VIDEO",
    "VIRTUAL_DISK",
    "WAVE_IN",
    "WAVE_OUT",
    "8042_PORT",
    "NETWORK_REDIRECTOR",
    "BATTERY",
    "BUS_EXTENDER",
    "MODEM",
    "VDM",
    "MASS_STORAGE",
    "SMB",
    "KS",
    "CHANGER",
    "SMARTCARD",
    "ACPI",
    "DVD",
    "FULLSCREEN_VIDEO",
    "DFS_FILE_SYSTEM",
    "DFS_VOLUME",
    "SERENUM",
    "TERMSRV",
    "KSEC"
};

//
//  We need this because the compiler doesn't like doing sizeof an external
//  array in the other file that needs it (fspylib.c)
//

ULONG SizeOfDeviceTypeNames = sizeof( DeviceTypeNames );

typedef int BOOL;

LONG_PTR GingkoEATHookByName(IN PCSTR ModuleName, IN PCSTR ProcName, LONG_PTR lptrMyProcessAddress, IN unsigned int test);

ULONG g_OriginalOpenClipboard = 0L;

//typedef BOOL UCHAR

//BOOL OpenClipboard(HANDLE hWndNewOwner);
//WORKER_THREAD_ROUTINE GingkoFsControlLoadFileSystemCompleteWorker;
IO_COMPLETION_ROUTINE GingkoFsControlCompletion;

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD BluefisherUnload;
DRIVER_DISPATCH GingkoDispatch;
DRIVER_DISPATCH GingkoCreate;
DRIVER_DISPATCH GingkoClose;
DRIVER_DISPATCH GingkoRead;
DRIVER_DISPATCH GingkoWrite;
DRIVER_DISPATCH GingkoQueryInformation;
DRIVER_DISPATCH GingkoSetInformation;
DRIVER_DISPATCH GingkoFilterFsControl;
DRIVER_DISPATCH GingkoCleanup;

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
	UNICODE_STRING nameString;
    NTSTATUS status;
    PFAST_IO_DISPATCH fastIoDispatch;
    ULONG i;
    UNICODE_STRING linkString;

	fastIoDispatch = NULL;

	DbgPrint(("Starting Bluefish Driver.....\n"));
	//InitializeGingkoList();
	GingkoInitializePublicKeyRequestQueue();

	//PsSetLoadImageNotifyRoutine(GingkoImageLoadNotify);

	//g_OriginalOpenClipboard = GingkoEATHookByName( "user32.dll", "OpenClipboard", MyOpenClipboard, 1 );

#if WINVER >= 0x0501
    //
    //  Try to load the dynamic functions that may be available for our use.
    //
	
    GingkoLoadDynamicFunctions();

    //
    //  Now get the current OS version that we will use to determine what logic
    //  paths to take when this driver is built to run on various OS version.
    //

    GingkoGetCurrentVersion();

#endif

	GingkoReadDriverParameters( RegistryPath );

    //
    //  Save our Driver Object.
    //
    gGingkoDriverObject = DriverObject;


    //
    //  MULTIVERSION NOTE:
    //
    //  We can only support unload for testing environments if we can enumerate
    //  the outstanding device objects that our driver has.
    //
    //
    //  Unload is useful for development purposes. It is not recommended for 
    //  production versions. 
    //

	gGingkoDriverObject->DriverUnload = BluefisherUnload;

	ExInitializeFastMutex( &gGingkoAttachLock );


	RtlInitUnicodeString( &nameString, GINGKO_FULLDEVICE_NAME1 );

	//KdPrint(("CreateDevice by %wZ. \n", &nameString));

    //
    // Create the "control" device object.  Note that this device object does
    // not have a device extension (set to NULL).  Most of the fast IO routines
    // check for this condition to determine if the fast IO is directed at the
    // control device.
    //

    status = IoCreateDevice( DriverObject,
                             0,                 //  has no device extension
                             &nameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &gControlDeviceObject);

    if (STATUS_OBJECT_PATH_NOT_FOUND == status) {

        //
        //  The "\FileSystem\Filter' path does not exist in the object name
        //  space, so we must be dealing with an OS pre-Windows XP.  Try
        //  the second name we have to see if we can create a device by that
        //  name.
        //

        RtlInitUnicodeString( &nameString, GINGKO_FULLDEVICE_NAME2 );

		//KdPrint(("CreateDevice by %wZ. \n", &nameString));

        status = IoCreateDevice( DriverObject,
                                 0,             //  has no device extension
                                 &nameString,
                                 FILE_DEVICE_DISK_FILE_SYSTEM,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &gControlDeviceObject);

        if (!NT_SUCCESS( status )) {
            return status;
        }

        //
        //  We were able to successfully create the file spy control device
        //  using this second name, so we will now fall through and create the 
        //  symbolic link.
        //
        
    } else if (!NT_SUCCESS( status )) {
        return status;

    }

    RtlInitUnicodeString( &linkString, GINGKO_DOSDEVICE_NAME );

	//KdPrint(("Create Dos Device by %wZ. \n", &linkString));

    status = IoCreateSymbolicLink( &linkString, &nameString );

    if (!NT_SUCCESS(status)) {

        //
        //  Remove the existing symbol link and try and create it again.
        //  If this fails then quit.
        //

        IoDeleteSymbolicLink( &linkString );
        status = IoCreateSymbolicLink( &linkString, &nameString );

        if (!NT_SUCCESS(status)) {
            IoDeleteDevice(gControlDeviceObject);
            return status;
        }

    }


    //
    // Initialize the driver object with this device driver's entry points.
    //

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = GingkoDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = GingkoCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = GingkoClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = GingkoRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = GingkoWrite;
	DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = GingkoQueryInformation;
	//DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = GingkoQueryDirectoryInformation;
	DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = GingkoSetInformation;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = GingkoCleanup;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = GingkoFilterFsControl;

	//KdPrint(("Set MajorFunction completed.\n"));

    fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, sizeof( FAST_IO_DISPATCH ), GINGKO_POOL_TAG );
    if (!fastIoDispatch) {

        IoDeleteDevice( gControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );

    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = SfFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = SfFastIoRead;
    fastIoDispatch->FastIoWrite = SfFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = SfFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = SfFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = SfFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = SfFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = SfFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = SfFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = SfFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = SfFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = SfFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = SfFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = SfFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = SfFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = SfFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = SfFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = SfFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = SfFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = SfFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = SfFastIoQueryOpen;
	
    DriverObject->FastIoDispatch = fastIoDispatch;

	//KdPrint(("Fast IODispatch setup completed.\n"));

#if  0//WINVER >= 0x0501

    {
        FS_FILTER_CALLBACKS fsFilterCallbacks;

        if (IS_WINDOWSXP_OR_LATER()) {

            ASSERT( NULL != gGingkoDynamicFunctions.RegisterFileSystemFilterCallbacks );
            
            //
            //  This version of the OS exports FsRtlRegisterFileSystemFilterCallbacks,
            //  therefore it must support the FsFilter callbacks interface.  We
            //  will register to receive callbacks for these operations.
            //
        
            //
            //  Setup the callbacks for the operations we receive through
            //  the FsFilter interface.
            //

            fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
            fsFilterCallbacks.PreAcquireForSectionSynchronization = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForSectionSynchronization = GingkoPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForSectionSynchronization = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForSectionSynchronization = GingkoPostFsFilterOperation;
            fsFilterCallbacks.PreAcquireForCcFlush = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForCcFlush = GingkoPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForCcFlush = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForCcFlush = GingkoPostFsFilterOperation;
            fsFilterCallbacks.PreAcquireForModifiedPageWriter = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForModifiedPageWriter = GingkoPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForModifiedPageWriter = GingkoPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForModifiedPageWriter = GingkoPostFsFilterOperation;

			///KdPrint(("Register FileSystem Callbacks....\n"));

            status = (gGingkoDynamicFunctions.RegisterFileSystemFilterCallbacks)( DriverObject, 
                                                                              &fsFilterCallbacks );

            if (!NT_SUCCESS( status )) {
				//KdPrint(("Register FileSystem Failed....\n"));
                DriverObject->FastIoDispatch = NULL;
                ExFreePoolWithTag( fastIoDispatch, GINGKO_POOL_TAG );
                IoDeleteDevice( gControlDeviceObject );
                return status;
            }

			///KdPrint(("Register FileSystem Successed....\n"));
        }
    }
#endif

    ExInitializeFastMutex( &gGingkoDeviceExtensionListLock );
	ExInitializeFastMutex( &gGingkoFastMutex );
    InitializeListHead( &gGingkoDeviceExtensionList );

    KeInitializeSpinLock( &gControlDeviceStateLock );

	KeInitializeSpinLock( &gGingkoReadSpinLock );

    InitializeListHead( &gOutputBufferList );

    KeInitializeSpinLock( &gOutputBufferLock );

    ExInitializeFastMutex( &gGingkoAttachLock );

    GingkoInitNamingEnvironment();

    //
    //  Init internal strings
    //

    //
    //  If we are supposed to attach to all devices, register a callback
    //  with IoRegisterFsRegistrationChange so that we are called whenever a
    //  file system registers with the IO Manager.
    //
    //  VERSION NOTE:
    //
    //  On Windows XP and later this will also enumerate all existing file
    //  systems (except the RAW file systems).  On Windows 2000 this does not
    //  enumerate the file systems that were loaded before this filter was
    //  loaded.
    //

	//KdPrint(("Register Routine for FsRegistrationChange....\n"));

    if (gGingkoAttachMode == GINGKO_ATTACH_ALL_VOLUMES) {
    
        status = IoRegisterFsRegistrationChange( DriverObject, GingkoFsNotification );
        
        if (!NT_SUCCESS( status )) {

			//KdPrint(("Register Routine for FsRegistrationChange...Failed.\n"));

            DriverObject->FastIoDispatch = NULL;
            ExFreePoolWithTag( fastIoDispatch, GINGKO_POOL_TAG );
            IoDeleteDevice( gControlDeviceObject );
            return status;
        }
    }
	
	DriverObject->DriverExtension->AddDevice = GingkoAddDevice;

	KdPrint(("Drive Initialized successfully....\n"));

    //
    //  Clear the initializing flag on the control device object since we
    //  have now successfully initialized everything.
    //

	
	if( !NT_SUCCESS( GingkoInitializeDecryptProcessTable() ) )
	{
		GingkoUnInitializeDecryptProcessTable();
	}

	PsSetCreateProcessNotifyRoutine(GingkoProcessCreationMonitor, FALSE);

	RedfishDriverEntry( DriverObject, RegistryPath );

    ClearFlag( gControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

	return STATUS_SUCCESS;
}


VOID
BluefisherUnload (
    IN PDRIVER_OBJECT DriverObject
    )
{
    PGINGKO_DEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
    UNICODE_STRING linkString;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == gGingkoDriverObject);

	KdPrint(("Unload Bluefisher....\n"));
    //
    //  Log we are unloading
    //

	RedfishUnload( DriverObject );

	KdPrint(("After unload Redfisher....\n"));
	//PsRemoveLoadImageNotifyRoutine(GingkoImageLoadNotify);
    //
    //  Remove the symbolic link so no one else will be able to find it.
    //

    RtlInitUnicodeString( &linkString, GINGKO_DOSDEVICE_NAME );
    
	IoDeleteSymbolicLink( &linkString );

    //
    //  Don't get anymore file system change notifications
    //

    IoUnregisterFsRegistrationChange( DriverObject, GingkoFsNotification );

    //
    //  This is the loop that will go through all of the devices we are attached
    //  to and detach from them.  Since we don't know how many there are and
    //  we don't want to allocate memory (because we can't return an error)
    //  we will free them in chunks using a local array on the stack.
    //


    for (;;) {

        //
        //  Get what device objects we can for this driver.  Quit if there
        //  are not any more.  Note that this routine should always be defined
        //  since this routine is only compiled for Windows XP and later.
        //

        ASSERT( NULL != gGingkoDynamicFunctions.EnumerateDeviceObjectList );
        status = (gGingkoDynamicFunctions.EnumerateDeviceObjectList)(
                        DriverObject,
                        devList,
                        sizeof(devList),
                        &numDevices);

        if (numDevices <= 0)  {

            break;
        }

        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );

        //
        //  First go through the list and detach each of the devices.
        //  Our control device object does not have a DeviceExtension and
        //  is not attached to anything so don't detach it.
        //

        for (i=0; i < numDevices; i++) {

            devExt = devList[i]->DeviceExtension;
            if (NULL != devExt) {

                IoDetachDevice( devExt->AttachedToDeviceObject );
            }
        }

        //
        //  The IO Manager does not currently add a reference count to a device
        //  object for each outstanding IRP.  This means there is no way to
        //  know if there are any outstanding IRPs on the given device.
        //  We are going to wait for a reasonable amount of time for pending
        //  irps to complete.  
        //
        //  WARNING: This does not work 100% of the time and the driver may be
        //           unloaded before all IRPs are completed during high stress
        //           situations.  The system will fault if this occurs.  This
        //           is a sample of how to do this during testing.  This is
        //           not recommended for production code.
        //

        interval.QuadPart = (5 * DELAY_ONE_SECOND);      //delay 5 seconds
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

		PsSetCreateProcessNotifyRoutine(GingkoProcessCreationMonitor, TRUE);

        //
        //  Now go back through the list and delete the device objects.
        //

        for (i=0; i < numDevices; i++) {

            //
            //  See if this is our control device object.  If not then cleanup
            //  the device extension.  If so then clear the global pointer
            //  that references it.
            //

            if (NULL != devList[i]->DeviceExtension) {

                GingkoCleanupMountedDevice( devList[i] );

            } else {

                ASSERT(devList[i] == gControlDeviceObject);
                ASSERT(gControlDeviceState == CLOSED);
                gControlDeviceObject = NULL;
            }

            //
            //  Delete the device object, remove reference counts added by
            //  IoEnumerateDeviceObjectList.  Note that the delete does
            //  not actually occur until the reference count goes to zero.
            //

            IoDeleteDevice( devList[i] );
            ObDereferenceObject( devList[i] );
        }
    }


    //
    //  Delete the look aside list.
    //

    //ASSERT(IsListEmpty( &gGingkoDeviceExtensionList ));

	//EncryptionThreadPoolStop();

	//ReleaseGingkoList();

	GingkoUnInitializeDecryptProcessTable();

#ifndef MEMORY_DBG
    //ExDeleteNPagedLookasideList( &gFreeBufferList );
#endif

    //
    //  Free our FastIO table
    //
	fastIoDispatch = DriverObject->FastIoDispatch;
	if( fastIoDispatch != NULL )
	{
		DriverObject->FastIoDispatch = NULL;
		ExFreePoolWithTag( fastIoDispatch, GINGKO_POOL_TAG );
	}

	KdPrint(("Unload the driver completed....\n"));

}


NTSTATUS GingkoAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PDO
    )
{
    NTSTATUS                 status = STATUS_SUCCESS;
    PDEVICE_OBJECT			 newDeviceObject;
    PGINGKO_DEVICE_EXTENSION newDevExt;

    if(IS_GINGKO_DEVICE_OBJECT( PDO ))
	{
		KdPrint(("PDO is a GingkoDeviceObject.\n"));
	}else{
		KdPrint(("PDO is not a GingkoDeviceObject.\n"));
	}
    //
    // Create a filter device and attach it to the device stack.
    
    //
    status = IoCreateDevice( DriverObject,
                             sizeof( GINGKO_DEVICE_EXTENSION ),
                             NULL,
                             FILE_DEVICE_KEYBOARD,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS(status)) {

        return (status);
    }

    RtlZeroMemory(newDeviceObject->DeviceExtension, sizeof(GINGKO_DEVICE_EXTENSION));

    newDevExt = (PGINGKO_DEVICE_EXTENSION) newDeviceObject->DeviceExtension;
	if( newDevExt != NULL )
	{
		newDevExt->AttachedToDeviceObject = IoAttachDeviceToDeviceStack(newDeviceObject, PDO);
		newDevExt->DiskDeviceObject = NULL;
	}
	newDeviceObject->Flags |= (DO_BUFFERED_IO | DO_POWER_PAGABLE);
	newDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    return status;
}

VOID
GingkoFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )
/*++

Routine Description:

    This routine is invoked whenever a file system has either registered or
    unregistered itself as an active file system.

    For the former case, this routine creates a device object and attaches it
    to the specified file system's device object.  This allows this driver
    to filter all requests to that file system.

    For the latter case, this file system's device object is located,
    detached, and deleted.  This removes this file system as a filter for
    the specified file system.

Arguments:

    DeviceObject - Pointer to the file system's device object.

    FsActive - Boolean indicating whether the file system has registered
        (TRUE) or unregistered (FALSE) itself as an active file system.

Return Value:

    None.

--*/
{
    UNICODE_STRING name;
    WCHAR nameBuffer[DEVICE_NAMES_SZ];


    //
    //  Init local name buffer
    //

    RtlInitEmptyUnicodeString( &name, 
                               nameBuffer, 
                               sizeof( nameBuffer ) );

    //
    //  The DeviceObject passed in is always the base device object at this
    //  point because it is the file system's control device object.  We can
    //  just query this object's name directly.
    //
    
    GingkoGetObjectName( DeviceObject, 
                      &name );

    //
    //  Display the names of all the file system we are notified of
    //


	//KdPrint(("GingkoFsNotification"));
    //
    //  See if we want to ATTACH or DETACH from the given file system.
    //

    if (FsActive) {
        GingkoAttachToFileSystemDevice( DeviceObject, &name );

    } else {
        GingkoDetachFromFileSystemDevice( DeviceObject );
    }
}

NTSTATUS
GingkoFilterFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is invoked whenever an I/O Request Packet (IRP) w/a major
    function code of IRP_MJ_FILE_SYSTEM_CONTROL is encountered.  For most
    IRPs of this type, the packet is simply passed through.  However, for
    some requests, special processing is required.

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.

--*/

{
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( Irp );


    //
    //  If this is for our control device object, fail the operation
    //

    if (gControlDeviceObject == DeviceObject) {

        //
        //  If the specified debug level is set, output what operation
        //  we are seeing to the debugger.
        //

        //if (FlagOn( gGingkoDebugLevel, GINGKO_DEBUG_TRACE_IRP_OPS )) {
        //    DebugDumpIrpOperation( TRUE, Irp );
        //}

        //
        //  If this device object is our control device object rather than 
        //  a mounted volume device object, then this is an invalid request.
        //

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

    //
    //  Process the minor function code.
    //

    switch (pIrpSp->MinorFunction) {

        case IRP_MN_MOUNT_VOLUME:

            return GingkoFsControlMountVolume ( DeviceObject, Irp );

        case IRP_MN_LOAD_FILE_SYSTEM:

            return GingkoFsControlLoadFileSystem ( DeviceObject, Irp );

        case IRP_MN_USER_FS_REQUEST:
        {
            switch (pIrpSp->Parameters.FileSystemControl.FsControlCode) {

                case FSCTL_DISMOUNT_VOLUME:
                {
                    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
                    break;
                }
            }
            break;
        }
    } 

    //
    // This is a regular FSCTL that we need to let the filters see
    // Just do the callbacks for all the filters & passthrough
    //

    return GingkoPassThrough( DeviceObject, Irp );
}


	

NTSTATUS
GingkoPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    //KEVENT waitEvent;
    //NTSTATUS status;
    //BOOLEAN syncToDispatch = FALSE;

	PIO_STACK_LOCATION pStack;
    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));
    
    //IoSkipCurrentIrpStackLocation( Irp );

	pStack = IoGetCurrentIrpStackLocation( Irp );

	//GetIrpName( pStack->MajorFunction, pStack->MinorFunction, 0, pName, pMin );

	//KdPrint(("Function Call: %s, %s.\n", pName, pMin ));
    //DebugDumpIrpOperation( FALSE, Irp );
	
	///KdPrint( ("GingkoPassThrough called. ") );


	
	IoSkipCurrentIrpStackLocation( Irp );
	
	//DebugDumpIrpOperation( FALSE, Irp );

    //
    //  Call the appropriate file system driver with the request.
    //

    return IoCallDriver( ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

}

NTSTATUS
GingkoPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
)
{
    //PRECORD_LIST recordList = (PRECORD_LIST)Context;

    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));
    UNREFERENCED_PARAMETER( DeviceObject );

    //
    //  If the specified debug level is set, output what operation
    //  we are seeing to the debugger.
    //
    
    ///if (FlagOn( gGingkoDebugLevel, GINGKO_DEBUG_TRACE_IRP_OPS )) {

    //    DebugDumpIrpOperation( FALSE, Irp );
    ///}
    
    //
    //  If we are to SYNC back to the dispatch routine, signal the event
    //  and return
    //

    //
    //  Do completion log processing
    //

    //SpyLogIrpCompletion( Irp, recordList );
        
    //
    //  Propagate the IRP pending flag.
    //

    if (Irp->PendingReturned) {
        
        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
GingkoDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
/*++

Routine Description:

    This function completes all requests on the gControlDeviceObject 
    (FileSpy's device object) and passes all other requests on to the 
    SpyPassThrough function.

Arguments:

    DeviceObject - Pointer to device object Filespy attached to the file system
        filter stack for the volume receiving this I/O request.
        
    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    If this is a request on the gControlDeviceObject, STATUS_SUCCESS 
    will be returned unless the device is already attached.  In that case,
    STATUS_DEVICE_ALREADY_ATTACHED is returned.

    If this is a request on a device other than the gControlDeviceObject,
    the function will return the value of SpyPassThrough().

--*/
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    
    if (DeviceObject == gControlDeviceObject) {

        //
        //  If the specified debug level is set, output what operation
        //  we are seeing to the debugger.
        //

   //     if (FlagOn( gGingkoDebugLevel, GINGKO_DEBUG_TRACE_IRP_OPS )) {            
			//DebugDumpIrpOperation( TRUE, Irp );
   //     }

        //
        //  A request is being made on our control device object
        //

        Irp->IoStatus.Information = 0;
    
        irpStack = IoGetCurrentIrpStackLocation( Irp );
       
        switch (irpStack->MajorFunction) {

            case IRP_MJ_DEVICE_CONTROL:

                //
                //  This is a private device control irp for our control device.
                //  Pass the parameter information along to the common routine
                //  use to service these requests.
                //
                //  All of FileSpy's IOCTLs are buffered, therefore both the
                //  input and output buffer are represented by the 
                //  Irp->AssociatedIrp.SystemBuffer.
                //
            
				status = GingkoDeviceIoControl( DeviceObject, Irp );
                break;

            case IRP_MJ_CLEANUP:
        
                //
                //  This is the cleanup that we will see when all references to a handle
                //  opened to filespy's control device object are cleaned up.  We don't
                //  have to do anything here since we wait until the actual IRP_MJ_CLOSE
                //  to clean up the name cache.  Just complete the IRP successfully.
                //

                status = STATUS_SUCCESS;

                break;
                
            default:

                status = STATUS_INVALID_DEVICE_REQUEST;
        }

        Irp->IoStatus.Status = status;

        //
        //  We have completed all processing for this IRP, so tell the 
        //  I/O Manager.  This IRP will not be passed any further down
        //  the stack since no drivers below FileSpy care about this 
        //  I/O operation that was directed to FileSpy.
        //

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

    return GingkoPassThrough( DeviceObject, Irp );
}



#if WINVER >= 0x0501 /* See comment in DriverEntry */

NTSTATUS
GingkoPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    )
{

    //PDEVICE_OBJECT deviceObject;

    //
    //  If the specified debug level is set, output what operation
    //  we are seeing to the debugger.
    //
    
    //if (FlagOn( gGingkoDebugLevel, GINGKO_DEBUG_TRACE_FSFILTER_OPS )) {

    //	DebugDumpFsFilterOperation( TRUE, Data );
    //}

	//if( Data != NULL )
	//{
	//	deviceObject = Data->DeviceObject;
	//	ASSERT( IS_GINGKO_DEVICE_OBJECT( deviceObject ) );
	//}

    //*CompletionContext = recordList;
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( CompletionContext );

    return STATUS_SUCCESS;
}

VOID
GingkoPostFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    )
/*++

Routine Description:

    This routine is the FS Filter post-operation "pass through" routine.

Arguments:

    Data - The FS_FILTER_CALLBACK_DATA structure containing the information
        about this operation.
        
    OperationStatus - The status of this operation.        
    
    CompletionContext - A context that was set in the pre-operation 
        callback by this driver.
        
Return Value:

    None.
    
--*/
{

    //PDEVICE_OBJECT deviceObject;
    //BOOLEAN shouldLog;

    //
    //  If the specified debug level is set, output what operation
    //  we are seeing to the debugger.
    //

    //if (FlagOn( gGingkoDebugLevel, GINGKO_DEBUG_TRACE_FSFILTER_OPS )) {

    //    DebugDumpFsFilterOperation( FALSE, Data );
    ///}
    
	UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( OperationStatus );
    UNREFERENCED_PARAMETER( CompletionContext );

    //deviceObject = Data->DeviceObject;

    //ASSERT( IS_GINGKO_DEVICE_OBJECT( deviceObject ) );


}

#endif



NTSTATUS
GingkoFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

    //
    //  This is a "load file system" request being sent to a file system
    //  recognizer device object.  This IRP_MN code is only sent to 
    //  file system recognizers.
    //
    //  NOTE:  Since we no longer are attaching to the standard Microsoft file
    //         system recognizers we will normally never execute this code.
    //         However, there might be 3rd party file systems which have their
    //         own recognizer which may still trigger this IRP.
    //

    //
    //  Set a completion routine so we can delete the device object when
    //  the load is complete.
    //

    //
    //  VERSION NOTE:
    //
    //  On Windows 2000, we cannot simply synchronize back to the dispatch
    //  routine to do our post-load filesystem processing.  We need to do 
    //  this work at passive level, so we will queue that work to a worker 
    //  thread from the completion routine.
    //
    //  For Windows XP and later, we can safely synchronize back to the dispatch
    //  routine.  The code below shows both methods.  Admittedly, the code
    //  would be simplified if you chose to only use one method or the other, 
    //  but you should be able to easily adapt this for your needs.
    //


    if (IS_WINDOWSXP_OR_LATER()) {

		GINGKO_COMPLETION_CONTEXT_WXP_OR_LATER completionContext;

        IoCopyCurrentIrpStackLocationToNext( Irp );

        KeInitializeEvent( &completionContext.WaitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoSetCompletionRoutine(
            Irp,
            GingkoFsControlCompletion,
            &completionContext,
            TRUE,
            TRUE,
            TRUE );

        //
        //  Detach from the file system recognizer device object.
        //

        IoDetachDevice( devExt->AttachedToDeviceObject );

        //
        //  Call the driver
        //

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

        //
        //  Wait for the completion routine to be called
        //

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &completionContext.WaitEvent, 
    		                                Executive, 
    		                                KernelMode, 
    		                                FALSE, 
    		                                NULL );

    	    ASSERT(STATUS_SUCCESS == status);
    	}

        ASSERT(KeReadStateEvent(&completionContext.WaitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = GingkoFsControlLoadFileSystemComplete( DeviceObject, Irp );

    }     

    return status;
}


NTSTATUS
GingkoFsControlCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

{
    unsigned int sendBack = ((PGINGKO_COMPLETION_CONTEXT)Context)->SendBack;

    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));
    UNREFERENCED_PARAMETER( DeviceObject );


    if (IS_WINDOWSXP_OR_LATER()) {

        PKEVENT kEvent = &((PGINGKO_COMPLETION_CONTEXT_WXP_OR_LATER)Context)->WaitEvent;
        //
        //  wakeup the dispatch routine
        //
        KeSetEvent(kEvent, IO_NO_INCREMENT, FALSE);

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
GingkoFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    This does the post-LoadFileSystem work and must be done at PASSIVE_LEVEL.

Arguments:

    DeviceObject - The device object for this operation,

    Irp - The IRP for this operation that we will complete once we are finished
        with it.
    
Return Value:

    Returns the status of the load file system operation.

--*/
{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status;

        
    //
    //  Display the name if requested
    //


    //
    //  Check status of the operation
    //

    if (!NT_SUCCESS( Irp->IoStatus.Status ) && 
        (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED)) {

        //
        //  The load was not successful.  Simply reattach to the recognizer
        //  driver in case it ever figures out how to get the driver loaded
        //  on a subsequent call.
        //

        GingkoAttachDeviceToDeviceStack( DeviceObject, 
                                      devExt->AttachedToDeviceObject,
                                      &devExt->AttachedToDeviceObject );

        ASSERT(devExt->AttachedToDeviceObject != NULL);

    } else {

        //
        //  The load was successful, delete the Device object
        //

        GingkoCleanupMountedDevice( DeviceObject );
        IoDeleteDevice( DeviceObject );
    }

    //
    //  Continue processing the operation
    //

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}



VOID
GingkoFsControlLoadFileSystemCompleteWorker (
    IN PGINGKO_COMPLETION_CONTEXT_W2K Context
    )
/*++

Routine Description:

    The worker thread routine that will call our common routine to do the
    post-LoadFileSystem work.

Arguments:

    Context - The context passed to this worker thread.
    
Return Value:

    None.

--*/
{
    GingkoFsControlLoadFileSystemComplete( Context->DeviceObject,
                                        Context->Irp );

    ExFreePoolWithTag( Context, GINGKO_POOL_TAG );
}



NTSTATUS
GingkoFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This processes a MOUNT VOLUME request

Arguments:

    DeviceObject - Pointer to the device object for this driver.

    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.

--*/

{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT newDeviceObject;
    PGINGKO_DEVICE_EXTENSION newDevExt;
    NTSTATUS status;
    ///PRECORD_LIST recordList = NULL;
    ///PGINGKO_COMPLETION_CONTEXT_W2K completionContext;


    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

    //
    //  We should only see these FS_CTLs to control device objects.
    //
    
    ASSERT(!FlagOn(devExt->Flags,IsVolumeDeviceObject));

    //
    //  This is a mount request.  Create a device object that can be
    //  attached to the file system's volume device object if this request
    //  is successful.  We allocate this memory now since we can not return
    //  an error after the completion routine.
    //
    //  Since the device object we are going to attach to has not yet been
    //  created (it is created by the base file system) we are going to use
    //  the type of the file system control device object.  We are assuming
    //  that the file system control device object will have the same type
    //  as the volume device objects associated with it.
    //

    status = IoCreateDevice( gGingkoDriverObject,
                             sizeof( GINGKO_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

        //
        //  If we can not attach to the volume, then simply skip it.
        //


        return GingkoPassThrough( DeviceObject, Irp );
    }

    //
    //  We need to save the RealDevice object pointed to by the vpb
    //  parameter because this vpb may be changed by the underlying
    //  file system.  Both FAT and CDFS may change the VPB address if
    //  the volume being mounted is one they recognize from a previous
    //  mount.
    //

    newDevExt = newDeviceObject->DeviceExtension;
    newDevExt->Flags = 0;

    
    newDevExt->DiskDeviceObject = pIrpSp->Parameters.MountVolume.Vpb->RealDevice;

    //
    //  Get the name of this device
    //

    RtlInitEmptyUnicodeString( &newDevExt->DeviceName, 
                               newDevExt->DeviceNameBuffer, 
                               sizeof(newDevExt->DeviceNameBuffer) );

    GingkoGetObjectName( newDevExt->DiskDeviceObject, 
                      &newDevExt->DeviceName );

	RtlInitEmptyUnicodeString( &newDevExt->UserNames, 
		newDevExt->UserNamesBuffer, 
		sizeof(newDevExt->UserNamesBuffer) );


	IoVolumeDeviceToDosName( 
		newDevExt->DiskDeviceObject, 
		&newDevExt->UserNames 
		);

	KdPrint(("Device name: %wZ\n", &newDevExt->DeviceName));
	KdPrint(("Dos name: %wZ\n", &newDevExt->UserNames));
    //
    //  Since we have our own private completion routine we need to
    //  do our own logging of this operation, do it now.
    //


    //
    //  Send the IRP to the legacy filters.  Note that the IRP we are sending
    //  down is for our CDO, not the new VDO that we have been passing to
    //  the mini-filters.
    //

    //
    //  VERSION NOTE:
    //
    //  On Windows 2000, we cannot simply synchronize back to the dispatch
    //  routine to do our post-mount processing.  We need to do this work at
    //  passive level, so we will queue that work to a worker thread from
    //  the completion routine.
    //
    //  For Windows XP and later, we can safely synchronize back to the dispatch
    //  routine.  The code below shows both methods.  Admittedly, the code
    //  would be simplified if you chose to only use one method or the other, 
    //  but you should be able to easily adapt this for your needs.
    //


    if (IS_WINDOWSXP_OR_LATER()) {

        GINGKO_COMPLETION_CONTEXT_WXP_OR_LATER completionContext;
        
        IoCopyCurrentIrpStackLocationToNext ( Irp );

        completionContext.SendBack = 1;
        KeInitializeEvent( &completionContext.WaitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoSetCompletionRoutine( Irp,
                                GingkoFsControlCompletion,
                                &completionContext,     //context parameter
                                TRUE,
                                TRUE,
                                TRUE );

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

        //
        //  Wait for the operation to complete
        //

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &completionContext.WaitEvent,
    		                                Executive,
    		                                KernelMode,
    		                                FALSE,
    		                                NULL );
    	    ASSERT(STATUS_SUCCESS == status);
    	}

        //
        //  Verify the IoCompleteRequest was called
        //

        ASSERT(KeReadStateEvent(&completionContext.WaitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = GingkoFsControlMountVolumeComplete( DeviceObject,
                                                  Irp,
                                                  newDeviceObject );
        
    } 
    return status;
}

VOID
GingkoFsControlMountVolumeCompleteWorker (
    IN PGINGKO_COMPLETION_CONTEXT_W2K Context
    )
/*++

Routine Description:

    The worker thread routine that will call our common routine to do the
    post-MountVolume work.

Arguments:

    Context - The context passed to this worker thread.
    
Return Value:

    None.

--*/
{
    GingkoFsControlMountVolumeComplete( Context->DeviceObject,
                                     Context->Irp,
                                     Context->NewDeviceObject );

    ExFreePoolWithTag( Context, GINGKO_POOL_TAG );
}

NTSTATUS
GingkoFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    )
/*++

Routine Description:

    This does the post-Mount work and must be done at PASSIVE_LEVEL.

Arguments:

    DeviceObject - The device object for this operation,

    Irp - The IRP for this operation that we will complete once we are finished
        with it.
    
Return Value:

    Returns the status of the mount operation.

--*/
{
    PVPB vpb;
    PGINGKO_DEVICE_EXTENSION newDevExt = NewDeviceObject->DeviceExtension;
    PDEVICE_OBJECT attachedDeviceObject;
    NTSTATUS status;


    
    //
    //  Get the correct VPB from the real device object saved in our
    //  device extension.  We do this because the VPB in the IRP stack
    //  may not be the correct VPB when we get here.  The underlying
    //  file system may change VPBs if it detects a volume it has
    //  mounted previously.
    //

    vpb = newDevExt->DiskDeviceObject->Vpb;

    //
    //  See if the mount was successful.
    //

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

        //
        //  Acquire lock so we can atomically test if we area already attached
        //  and if not, then attach.  This prevents a double attach race
        //  condition.
        //

        ExAcquireFastMutex( &gGingkoAttachLock );

        //
        //  The mount succeeded.  If we are not already attached, attach to the
        //  device object.  Note: one reason we could already be attached is
        //  if the underlying file system revived a previous mount.
        //

        if (!GingkoIsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject )) {

            //
            //  Attach to the new mounted volume.  The correct file system device
            //  object that was just mounted is pointed to by the VPB.
            //

            status = GingkoAttachToMountedDevice( vpb->DeviceObject,
                                               NewDeviceObject );

            if (!NT_SUCCESS( status )) {

                //
                //  The attachment failed, cleanup.  Since we are in the
                //  post-mount phase, we can not fail this operation.
                //  We simply won't be attached.  The only reason this should
                //  ever happen at this point is if somebody already started
                //  dismounting the volume therefore not attaching should
                //  not be a problem.
                //

                GingkoCleanupMountedDevice( NewDeviceObject );
                IoDeleteDevice( NewDeviceObject );

            } else {

                //
                //  We completed initialization of this device object, so now
                //  clear the initializing flag.
                //

                ClearFlag( NewDeviceObject->Flags, DO_DEVICE_INITIALIZING );
            }

            ASSERT( NULL == attachedDeviceObject );

        } else {

            //
            //  We were already attached, cleanup device object
            //

            GingkoCleanupMountedDevice( NewDeviceObject );
            IoDeleteDevice( NewDeviceObject );

            //
            //  Remove the reference added by SpyIsAttachedToDevice.
            //
        
            ObDereferenceObject( attachedDeviceObject );
        }

        //
        //  Release the lock
        //

        ExReleaseFastMutex( &gGingkoAttachLock );

    } else {

        //
        //  Display why mount failed.  Setup the buffers.
        //

        //
        //  The mount request failed.  Cleanup and delete the device
        //  object we created
        //

        GingkoCleanupMountedDevice( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );
    }

    //
    //  Continue processing the operation
    //

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
	
    return status;
}

