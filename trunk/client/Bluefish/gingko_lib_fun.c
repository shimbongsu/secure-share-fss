#include "gingko_filter.h"

//extern PVOID SessionBuffer;
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                     Library support routines                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

LONG ___SharedNotificationReference = 0L;

VOID
GingkoReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This routine tries to read the FileSpy-specific parameters from
    the registry.  These values will be found in the registry location
    indicated by the RegistryPath passed in.

Arguments:

    RegistryPath - the path key which contains the values that are
        the FileSpy parameters

Return Value:

    None.

--*/
{
    OBJECT_ATTRIBUTES attributes;
    HANDLE driverRegKey;
    NTSTATUS status;
    PVOID buffer = NULL;

    //
    //  All the global values are already set to default values.  Any
    //  values we read from the registry will override these defaults.
    //
    
    //
    //  Do the initial setup to start reading from the registry.
    //

    InitializeObjectAttributes( &attributes,
								RegistryPath,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);

    status = ZwOpenKey( &driverRegKey,
						KEY_READ,
						&attributes);

    if (!NT_SUCCESS(status)) {

        driverRegKey = NULL;
        goto GingkoReadDriverParameters_Exit;
    }

    goto GingkoReadDriverParameters_Exit;

GingkoReadDriverParameters_Exit:

    if (NULL != buffer) {

        ExFreePoolWithTag( buffer, GINGKO_POOL_TAG );
    }

    if (NULL != driverRegKey) {

        ZwClose(driverRegKey);
    }

    return;
}

VOID
GingkoInitNamingEnvironment(
    VOID
    )
{

}


#if WINVER >= 0x0501
VOID
GingkoLoadDynamicFunctions (
    )
/*++

Routine Description:

    This routine tries to load the function pointers for the routines that
    are not supported on all versions of the OS.  These function pointers are
    then stored in the global structure gGingkoDynamicFunctions.

    This support allows for one driver to be built that will run on all 
    versions of the OS Windows 2000 and greater.  Note that on Windows 2000, 
    the functionality may be limited.
    
Arguments:

    None.
    
Return Value:

    None.

--*/
{
    UNICODE_STRING functionName;

    RtlZeroMemory( &gGingkoDynamicFunctions, sizeof( gGingkoDynamicFunctions ) );

	

    //
    //  For each routine that we would want to use, lookup its address in the
    //  kernel or hal.  If it is not present, that field in our global
    //  gGingkoDynamicFunctions structure will be set to NULL.
    //

    RtlInitUnicodeString( &functionName, L"FsRtlRegisterFileSystemFilterCallbacks" );
    //KdPrint( ("Load System Routine %wZ\n", &functionName) );
	gGingkoDynamicFunctions.RegisterFileSystemFilterCallbacks = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoAttachDeviceToDeviceStackSafe" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe = MmGetSystemRoutineAddress( &functionName );
    
    RtlInitUnicodeString( &functionName, L"IoEnumerateDeviceObjectList" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.EnumerateDeviceObjectList = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetLowerDeviceObject" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.GetLowerDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDeviceAttachmentBaseRef" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.GetDeviceAttachmentBaseRef = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDiskDeviceObject" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.GetDiskDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetAttachedDeviceReference" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.GetAttachedDeviceReference = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"RtlGetVersion" );
	//KdPrint( ("Load System Routine %wZ\n", &functionName) );
    gGingkoDynamicFunctions.GetVersion = MmGetSystemRoutineAddress( &functionName );
}

VOID
GingkoGetCurrentVersion (
    )
/*++

Routine Description:

    This routine reads the current OS version using the correct routine based
    on what routine is available.

Arguments:

    None.
    
Return Value:

    None.

--*/
{
    if (NULL != gGingkoDynamicFunctions.GetVersion) {

        RTL_OSVERSIONINFOW versionInfo;
        NTSTATUS status;

        //
        //  VERSION NOTE: RtlGetVersion does a bit more than we need, but
        //  we are using it if it is available to show how to use it.  It
        //  is available on Windows XP and later.  RtlGetVersion and
        //  RtlVerifyVersionInfo (both documented in the IFS Kit docs) allow
        //  you to make correct choices when you need to change logic based
        //  on the current OS executing your code.
        //

        versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );

        status = (gGingkoDynamicFunctions.GetVersion)( &versionInfo );

        ASSERT( NT_SUCCESS( status ) );

        gGingkoOsMajorVersion = versionInfo.dwMajorVersion;
        gGingkoOsMinorVersion = versionInfo.dwMinorVersion;
        
    } else {
		RTL_OSVERSIONINFOW OsVersion;
		OsVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

		RtlGetVersion( &OsVersion );
        //PsGetVersion( &gGingkoOsMajorVersion,
        //              &gGingkoOsMinorVersion,
        //              NULL,
        //              NULL );

		gGingkoOsMajorVersion = OsVersion.dwMajorVersion;
		gGingkoOsMinorVersion = OsVersion.dwMinorVersion;
    }
	KdPrint( ("This driver is running on Windows Version: %d.%d \n ", gGingkoOsMajorVersion, gGingkoOsMinorVersion) );
}

////////////////////////////////////////////////////////////////////////
//                                                                    //
//               Device name tracking helper routines                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

VOID
GingkoGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
/*++

Routine Description:

    This routine will return the name of the given object.
    If a name can not be found an empty string will be returned.

Arguments:

    Object - The object whose name we want

    Name - A unicode string that is already initialized with a buffer

Return Value:

    None

--*/
{
    NTSTATUS status;
    CHAR nibuf[512];        //buffer that receives NAME information and name
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, 
                                nameInfo, 
                                sizeof(nibuf), 
                                &retLength );

    //
    //  Init current length, if we have an error a NULL string will be returned
    //

    Name->Length = 0;

    if (NT_SUCCESS( status )) {

        //
        //  Copy what we can of the name string
        //
        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}

SharedNotificationPtr GingkoReferenceSharedNotification()
{
	if( gControlDeviceState != CLOSED )
	{
		___SharedNotificationReference ++;
		return gGingkoNotification;
	}
	return NULL;
}

LONG GingkoDereferenceSharedNotification(SharedNotificationPtr pg)
{
	if( pg == gGingkoNotification )
	{
		___SharedNotificationReference --;
	}
	return ___SharedNotificationReference;
}


VOID
GingkoCloseControlDevice ()
{
    ///KIRQL oldIrql;
	//KdPrint( ("GingkoCloseControlDevice called.") );
	if( gControlDeviceState != CLOSED )
	{
		//KeAcquireSpinLock( &gControlDeviceStateLock, &oldIrql );
		if( gControlDeviceState != CLOSED )
		{
			gControlDeviceState = CLOSED;
			if( gGingkoNotification != NULL )
			{
				SharedNotificationPtr mLocal = gGingkoNotification;
				gGingkoNotification = NULL;
				gGingkoServerStarted = FALSE;
				if( mLocal->ServerThread != NULL )
				{
					//ObDereferenceObject( mLocal->ServerThread );
				}
				if( mLocal->ReadEvent != NULL )
				{
					//KeSetEvent( mLocal->ReadEvent, IO_NO_INCREAME );
					//ObDereferenceObject( mLocal->ReadEvent );
				}
				if( mLocal->WriteEvent != NULL )
				{
					//ObDereferenceObject( mLocal->WriteEvent );
				}

				RemoveAllFileObjectQueue();

				ExFreePoolWithTag( mLocal, GINGKO_POOL_TAG );
			}

			gGingkoNotification  = NULL; /// It must be cleard.
			gGingkoServerStarted = FALSE;
			gControlDeviceState = CLOSED;
		}
		//KeReleaseSpinLock( &gControlDeviceStateLock, oldIrql );
	}
	return;
}

#endif


////////////////////////////////////////////////////////////////////////
//                                                                    //
//       Attaching/detaching to all volumes in system routines        //
//                                                                    //
////////////////////////////////////////////////////////////////////////

NTSTATUS
GingkoAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    )
{
    PDEVICE_OBJECT filespyDeviceObject;
    PGINGKO_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;
    UNICODE_STRING tempName;
    WCHAR tempNameBuffer[DEVICE_NAMES_SZ];


	//KdPrint( ("During AttachToFileSystemDevice.\n") );
    //
    //  See if this is a file system we care about.  If not, return.
    //

    if (!IS_SUPPORTED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

    //
    //  See if this is Microsoft's file system recognizer device (see if the name of the
    //  driver is the FS_REC driver).  If so skip it.  We don't need to 
    //  attach to file system recognizer devices since we can just wait for the
    //  real file system driver to load.  Therefore, if we can identify them, we won't
    //  attach to them.
    //

    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );

    RtlInitEmptyUnicodeString( &tempName,
                               tempNameBuffer,
                               sizeof(tempNameBuffer) );

    GingkoGetObjectName( DeviceObject->DriverObject, &tempName );
    
    if (RtlCompareUnicodeString( &tempName, &fsrecName, TRUE ) == 0) {
        return STATUS_SUCCESS;
    }

	KdPrint( ("Temp name: %wZ\n", &tempName) );

    //
    //  Create a new device object we can attach with
    //
    status = IoCreateDevice( gGingkoDriverObject,
                             sizeof( GINGKO_DEVICE_EXTENSION ),
                             (PUNICODE_STRING) NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &filespyDeviceObject );

    if (!NT_SUCCESS( status )) {

        return status;
    }

    //
    //  Load extension, set device object associated with extension
    //

    devExt = filespyDeviceObject->DeviceExtension;

    devExt->Flags = 0;

    devExt->ThisDeviceObject = filespyDeviceObject;

    //
    //  Propagate flags from Device Object we are trying to attach to.
    //  Note that we do this before the actual attachment to make sure
    //  the flags are properly set once we are attached (since an IRP
    //  can come in immediately after attachment but before the flags would
    //  be set).
    //

    //if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

    //    SetFlag( filespyDeviceObject->Flags, DO_BUFFERED_IO );
    //}

    if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( filespyDeviceObject->Flags, DO_DIRECT_IO );
    }

    if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) ) {

        SetFlag( filespyDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );
    }

	//KdPrint( ("Before AttachDeviceToDeviceStack.\n") );

    //
    //  Do the attachment
    //

    status = GingkoAttachDeviceToDeviceStack( filespyDeviceObject, 
                                           DeviceObject, 
                                           &devExt->AttachedToDeviceObject );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupDevice;
    }

    //
    //  Since this is an attachment to a file system control device object
    //  we are not going to log anything, but properly initialize our
    //  extension.
    //

    RtlInitEmptyUnicodeString( &devExt->DeviceName,
                               devExt->DeviceNameBuffer,
                               sizeof(devExt->DeviceNameBuffer) );

    RtlCopyUnicodeString( &devExt->DeviceName, DeviceName );        //Save Name

    RtlInitEmptyUnicodeString( &devExt->UserNames,
                               devExt->UserNamesBuffer,
                               sizeof(devExt->UserNamesBuffer) );

	//KdPrint( ("Before INIT DEVICE NAMING ENVIRONMENT.\n") );

	

    GingkoInitDeviceNamingEnvironment( filespyDeviceObject );

	//KdPrint( ("AFTER INIT DEVICE NAMING ENVIRONMENT.\n") );
    //
    //  The NETWORK device objects function as both CDOs (control device object)
    //  and VDOs (volume device object) so insert the NETWORK CDO devices into
    //  the list of attached device so we will properly enumerate it.
    //

	devExt->Flags |= DeviceObject->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);
	//devExt->Flags &= ~DO_DEVICE_INITIALIZING;

    if (FILE_DEVICE_NETWORK_FILE_SYSTEM == DeviceObject->DeviceType) {

        ExAcquireFastMutex( &gGingkoDeviceExtensionListLock );
        InsertTailList( &gGingkoDeviceExtensionList, &devExt->NextFileSpyDeviceLink );
        ExReleaseFastMutex( &gGingkoDeviceExtensionListLock );
        SetFlag(devExt->Flags, ExtensionIsLinked);
    }

    //
    //  Flag we are no longer initializing this device object
    //

    ClearFlag( filespyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    //
    //  Display who we have attached to
    //

    //
    //  VERSION NOTE:
    //
    //  In Windows XP, the IO Manager provided APIs to safely enumerate all the
    //  device objects for a given driver.  This allows filters to attach to 
    //  all mounted volumes for a given file system at some time after the
    //  volume has been mounted.  There is no support for this functionality
    //  in Windows 2000.
    //
    //  MULTIVERSION NOTE:
    //
    //  If built for Windows XP or later, this driver is built to run on 
    //  multiple versions.  When this is the case, we will test
    //  for the presence of the new IO Manager routines that allow for volume 
    //  enumeration.  If they are not present, we will not enumerate the volumes
    //  when we attach to a new file system.
    //
    
#if WINVER >= 0x0501

    if (IS_WINDOWSXP_OR_LATER()) {

		

        ASSERT( NULL != gGingkoDynamicFunctions.EnumerateDeviceObjectList &&
                NULL != gGingkoDynamicFunctions.GetDiskDeviceObject &&
                NULL != gGingkoDynamicFunctions.GetDeviceAttachmentBaseRef &&
                NULL != gGingkoDynamicFunctions.GetLowerDeviceObject );


		//KdPrint( ("Xp special process for GingkoEnumerateFileSystemVolumes.\n") );
        //
        //  Enumerate all the mounted devices that currently
        //  exist for this file system and attach to them.
        //

        status = GingkoEnumerateFileSystemVolumes( DeviceObject, &tempName );

        if (!NT_SUCCESS( status )) {

            IoDetachDevice( devExt->AttachedToDeviceObject );
            goto ErrorCleanupDevice;
        }
    }
    
#endif

	//GingkoStartReadWriteThread( DeviceObject, devExt );
	
    return STATUS_SUCCESS;

    /////////////////////////////////////////////////////////////////////
    //                  Cleanup error handling
    /////////////////////////////////////////////////////////////////////

    ErrorCleanupDevice:
        GingkoCleanupMountedDevice( filespyDeviceObject );
        IoDeleteDevice( filespyDeviceObject );

    return status;
}

VOID
GingkoDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    Given a base file system device object, this will scan up the attachment
    chain looking for our attached device object.  If found it will detach
    us from the chain.

Arguments:

    DeviceObject - The file system device to detach from.

Return Value:

--*/ 
{
    PDEVICE_OBJECT ourAttachedDevice;
    PGINGKO_DEVICE_EXTENSION devExt;

    //
    //  We have to iterate through the device objects in the filter stack
    //  attached to DeviceObject.  If we are attached to this filesystem device
    //  object, We should be at the top of the stack, but there is no guarantee.
    //  If we are in the stack and not at the top, we can safely call IoDetachDevice
    //  at this time because the IO Manager will only really detach our device
    //  object from the stack at a safe time.
    //

    //
    //  Skip the base file system device object (since it can't be us)
    //

    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_GINGKO_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

            //
            //  Display who we detached from
            //
                                
            //
            //  Unlink from global list
            //

            if (FlagOn(devExt->Flags,ExtensionIsLinked)) {

                ExAcquireFastMutex( &gGingkoDeviceExtensionListLock );
                RemoveEntryList( &devExt->NextFileSpyDeviceLink );
                ExReleaseFastMutex( &gGingkoDeviceExtensionListLock );
                ClearFlag(devExt->Flags,ExtensionIsLinked);
            }

            //
            //  Detach us from the object just below us
            //  Cleanup and delete the object
            //

            GingkoCleanupMountedDevice( ourAttachedDevice );
            IoDetachDevice( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            return;
        }

        //
        //  Look at the next device up in the attachment chain
        //

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
}



NTSTATUS
GingkoEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
/*++

Routine Description:

    Enumerate all the mounted devices that currently exist for the given file
    system and attach to them.  We do this because this filter could be loaded
    at any time and there might already be mounted volumes for this file system.

Arguments:

    FSDeviceObject - The device object for the file system we want to enumerate

    Name - An already initialized unicode string used to retrieve names

Return Value:

    The status of the operation

--*/
{
    PDEVICE_OBJECT newDeviceObject;
    PGINGKO_DEVICE_EXTENSION newDevExt;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT diskDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;

    //
    //  Find out how big of an array we need to allocate for the
    //  mounted device list.
    //

    ASSERT( NULL != gGingkoDynamicFunctions.EnumerateDeviceObjectList );
    status = (gGingkoDynamicFunctions.EnumerateDeviceObjectList)( FSDeviceObject->DriverObject,
                                                              NULL,
                                                              0,
                                                              &numDevices);

    //
    //  We only need to get this list of there are devices.  If we
    //  don't get an error there are no devices so go on.
    //

    if (!NT_SUCCESS( status )) {

        ASSERT(STATUS_BUFFER_TOO_SMALL == status);

        //
        //  Allocate memory for the list of known devices
        //

        numDevices += 8;        //grab a few extra slots

        devList = ExAllocatePoolWithTag( NonPagedPool, 
                                         (numDevices * sizeof(PDEVICE_OBJECT)), 
                                         GINGKO_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        //  Now get the list of devices.  If we get an error again
        //  something is wrong, so just fail.
        //

        status = (gGingkoDynamicFunctions.EnumerateDeviceObjectList)(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePoolWithTag( devList, GINGKO_POOL_TAG );
            return status;
        }

		//KdPrint( ("After Call EnumerateDeviceObjectList. We got the %d devices. \n", numDevices ) );

        //
        //  Walk the given list of devices and attach to them if we should.
        //

        for (i=0; i < numDevices; i++) {

            //
            //  Do not attach if:
            //      - This is the control device object (the one passed in)
            //      - The device type does not match
            //      - We are already attached to it
            //

            if ((devList[i] != FSDeviceObject) && 
                (devList[i]->DeviceType == FSDeviceObject->DeviceType) &&
                !GingkoIsAttachedToDevice( devList[i], NULL )) {

                //
                //  See if this device has a name.  If so, then it must
                //  be a control device so don't attach to it.  This handles
                //  drivers with more then one control device.
                //

                GingkoGetBaseDeviceObjectName( devList[i], Name );

				KdPrint(("Name: %wZ.\n", Name));

                if (Name->Length <= 0) {

                    //
                    //  Get the disk device object associated with this
                    //  file  system device object.  Only try to attach if we
                    //  have a disk device object.
                    //

                    ASSERT( NULL != gGingkoDynamicFunctions.GetDiskDeviceObject );
                    status = (gGingkoDynamicFunctions.GetDiskDeviceObject)( devList[i], &diskDeviceObject );

					//KdPrint(("After call GetDiskDeviceObject.\n"));

                    if ( status == STATUS_SUCCESS) {

                        //
                        //  Allocate a new device object to attach with
                        //

                        status = IoCreateDevice( gGingkoDriverObject,
                                                 sizeof( GINGKO_DEVICE_EXTENSION ),
                                                 (PUNICODE_STRING) NULL,
                                                 devList[i]->DeviceType,
                                                 0,
                                                 FALSE,
                                                 &newDeviceObject );
						//KdPrint(("After call IoCreateDevice.\n"));
						

                        if (NT_SUCCESS( status )) {

                            //
                            //  Set disk device object
                            //

                            newDevExt = newDeviceObject->DeviceExtension;
                            newDevExt->Flags = 0;

                            newDevExt->DiskDeviceObject = diskDeviceObject;

							//KdPrint(("Initialize the DeviceName of  New Device.\n"));
                    
                            //
                            //  Set Device Name
                            //

                            RtlInitEmptyUnicodeString( &newDevExt->DeviceName,
                                                       newDevExt->DeviceNameBuffer,
                                                       sizeof(newDevExt->DeviceNameBuffer) );

							RtlInitEmptyUnicodeString( &newDevExt->UserNames,
													   newDevExt->UserNamesBuffer,
                                                       sizeof(newDevExt->UserNamesBuffer) );

							//KdPrint(("Initialized the Buffer successfull.\n"));

							

                            GingkoGetObjectName( diskDeviceObject, 
                                              &(newDevExt->DeviceName) );

							//RtlVolumeDeviceToDosName
							IoVolumeDeviceToDosName( 
								diskDeviceObject, 
								&(newDevExt->UserNames) );
							

							//KdPrint(("After GingkoGetObjectName=%wZ.\n"));
							
                            //
                            //  We have done a lot of work since the last time
                            //  we tested to see if we were already attached
                            //  to this device object.  Test again, this time
                            //  with a lock, and attach if we are not attached.
                            //  The lock is used to atomically test if we are
                            //  attached, and then do the attach.
                            //

                            ExAcquireFastMutex( &gGingkoAttachLock );

							KdPrint(("New Device Name: %wZ.Dos name: %wZ\n", &newDevExt->DeviceName, &newDevExt->UserNames ));

                            if (!GingkoIsAttachedToDevice( devList[i], NULL )) {

                                //
                                //  Attach to this device object
                                //


								KdPrint(("GingkoIsAttachedToDevice return to FALSE;"));

                                status = GingkoAttachToMountedDevice( devList[i], 
                                                                   newDeviceObject );

								KdPrint(("GingkoAttachToMountedDevice;"));

                                //
                                //  Handle normal vs error cases, but keep going
                                //

                                if (NT_SUCCESS( status )) {

                                    //
                                    //  Finished all initialization of the new
                                    //  device object,  so clear the initializing
                                    //  flag now.  This allows other filters to
                                    //  now attach to our device object.
                                    //

                                    ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );

                                } else {

                                    //
                                    //  The attachment failed, cleanup.  Note that
                                    //  we continue processing so we will cleanup
                                    //  the reference counts and try to attach to
                                    //  the rest of the volumes.
                                    //
                                    //  One of the reasons this could have failed
                                    //  is because this volume is just being
                                    //  mounted as we are attaching and the
                                    //  DO_DEVICE_INITIALIZING flag has not yet
                                    //  been cleared.  A filter could handle
                                    //  this situation by pausing for a short
                                    //  period of time and retrying the attachment.
                                    //

                                    GingkoCleanupMountedDevice( newDeviceObject );
                                    IoDeleteDevice( newDeviceObject );
                                }

                            } else {

                                //
                                //  We were already attached, cleanup this
                                //  device object.
                                //

                                GingkoCleanupMountedDevice( newDeviceObject );
                                IoDeleteDevice( newDeviceObject );
                            }

                            //
                            //  Release the lock
                            //

                            ExReleaseFastMutex( &gGingkoAttachLock );

							//KdPrint(("Attached to Mounted Device.\n"));

                        } else {

                        }
                        
                        //
                        //  Remove reference added by IoGetDiskDeviceObject.
                        //  We only need to hold this reference until we are
                        //  successfully attached to the current volume.  Once
                        //  we are successfully attached to devList[i], the
                        //  IO Manager will make sure that the underlying
                        //  diskDeviceObject will not go away until the file
                        //  system stack is torn down.
                        //

                        ObDereferenceObject( diskDeviceObject );
                    }
                }
            }

            //
            //  Dereference the object (reference added by 
            //  IoEnumerateDeviceObjectList)
            //

            ObDereferenceObject( devList[i] );
        }

        //
        //  We are going to ignore any errors received while loading.  We
        //  simply won't be attached to those volumes if we get an error
        //

        status = STATUS_SUCCESS;

        //
        //  Free the memory we allocated for the list
        //

        ExFreePoolWithTag( devList, GINGKO_POOL_TAG );
    }

    return status;
}


VOID
GingkoGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
/*++

Routine Description:

    This locates the base device object in the given attachment chain and then
    returns the name of that object.

    If no name can be found, an empty string is returned.

Arguments:

    Object - The object whose name we want

    Name - A unicode string that is already initialized with a buffer

Return Value:

    None

--*/
{
    //
    //  Get the base file system device object
    //

    ASSERT( NULL != gGingkoDynamicFunctions.GetDeviceAttachmentBaseRef );
    DeviceObject = (gGingkoDynamicFunctions.GetDeviceAttachmentBaseRef)( DeviceObject );

    //
    //  Get the name of that object
    //

    GingkoGetObjectName( DeviceObject, Name );

    //
    //  Remove the reference added by IoGetDeviceAttachmentBaseRef
    //

    ObDereferenceObject( DeviceObject );
}



BOOLEAN
GingkoIsAttachedToDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
{    
        ASSERT( NULL != gGingkoDynamicFunctions.GetLowerDeviceObject &&
                NULL != gGingkoDynamicFunctions.GetDeviceAttachmentBaseRef );
        
        return GingkoIsAttachedToDeviceWXPAndLater( DeviceObject, AttachedDeviceObject );

}



BOOLEAN
GingkoIsAttachedToDeviceWXPAndLater (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    
    //
    //  Get the device object at the TOP of the attachment chain
    //

    ASSERT( NULL != gGingkoDynamicFunctions.GetAttachedDeviceReference );
    currentDevObj = (gGingkoDynamicFunctions.GetAttachedDeviceReference)( DeviceObject );

	KdPrint(("Get the Top of the attachment chain is OK."));

    //
    //  Scan down the list to find our device object.
    //

    do {
    
        if (IS_GINGKO_DEVICE_OBJECT( currentDevObj )) {

            //
            //  We have found that we are already attached.  If we are
            //  returning the device object, leave it referenced else remove
            //  the reference.
            //

            if (NULL != AttachedDeviceObject) {

                *AttachedDeviceObject = currentDevObj;

            } else {

                ObDereferenceObject( currentDevObj );
            }

            return TRUE;
        }

        //
        //  Get the next attached object.  This puts a reference on 
        //  the device object.
        //

        ASSERT( NULL != gGingkoDynamicFunctions.GetLowerDeviceObject );
        nextDevObj = (gGingkoDynamicFunctions.GetLowerDeviceObject)( currentDevObj );

        //
        //  Dereference our current device object, before
        //  moving to the next one.
        //

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
    //
    //  Mark no device returned
    //

    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}


NTSTATUS
GingkoAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT FilespyDeviceObject
    )
{
    PGINGKO_DEVICE_EXTENSION devExt;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    ASSERT( IS_GINGKO_DEVICE_OBJECT( FilespyDeviceObject ) );
    ASSERT( !GingkoIsAttachedToDevice( DeviceObject, NULL ) );
    
    //
    //  Insert pointer from extension back to owning device object
    //
    devExt = FilespyDeviceObject->DeviceExtension;

    devExt->ThisDeviceObject = FilespyDeviceObject;

	devExt->Flags = 0;

	//devExt->AttachedToDeviceObject = NULL;

    //
    //  Propagate flags from Device Object we are trying to attach to.
    //  Note that we do this before the actual attachment to make sure
    //  the flags are properly set once we are attached (since an IRP
    //  can come in immediately after attachment but before the flags would
    //  be set).
    //

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( FilespyDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( FilespyDeviceObject->Flags, DO_DIRECT_IO );
    }

	//if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) ) {

    //    SetFlag( FilespyDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );
    //}

    //
    //  It is possible for this attachment request to fail because this device
    //  object has not finished initializing.  This can occur if this filter
    //  loaded just as this volume was being mounted.
    //	
//////ERROR TRACKING BEGIN HERE.

    for (i=0; i < 8; i++) {
        LARGE_INTEGER interval;

        //
        //  Attach our device object to the given device object
        //  The only reason this can fail is if someone is trying to dismount
        //  this volume while we are attaching to it.
        //
		KdPrint(("AttachToMountedDevice........"));
        status = GingkoAttachDeviceToDeviceStack( FilespyDeviceObject,
												  DeviceObject,
												  &(devExt->AttachedToDeviceObject) );
		KdPrint(("AttachToMountedDevice........finished."));
        if (NT_SUCCESS(status) ) {

            //
            //  Do all common initializing of the device extension
            //

            SetFlag(devExt->Flags,IsVolumeDeviceObject);

            //RtlInitEmptyUnicodeString( &devExt->UserNames,
            //                           devExt->UserNamesBuffer,
            //                           sizeof(devExt->UserNamesBuffer) );

            GingkoInitDeviceNamingEnvironment( FilespyDeviceObject );

			//IoVolumeDeviceToDosName( DeviceObject, &devExt->UserNames );

			//KdPrint( ("Dos name: %wZ\n", &devExt->UserNames) );


            //
            //  Add this device to our attachment list
            //

            ExAcquireFastMutex( &gGingkoDeviceExtensionListLock );
            InsertTailList( &gGingkoDeviceExtensionList, &devExt->NextFileSpyDeviceLink );
            ExReleaseFastMutex( &gGingkoDeviceExtensionListLock );
            SetFlag(devExt->Flags,ExtensionIsLinked);

			//GingkoStartReadWriteThread( FilespyDeviceObject, devExt );

            return STATUS_SUCCESS;
        }

        //
        //  Delay, giving the device object a chance to finish its
        //  initialization so we can try again
        //

        interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);      //delay 1/2 second
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

    return status;
}


VOID
GingkoCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    
    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

    GingkoCleanupDeviceNamingEnvironment( DeviceObject );

    //
    //  Unlink from global list
    //

    if (FlagOn(devExt->Flags,ExtensionIsLinked)) {

        ExAcquireFastMutex( &gGingkoDeviceExtensionListLock );
        RemoveEntryList( &devExt->NextFileSpyDeviceLink );
        ExReleaseFastMutex( &gGingkoDeviceExtensionListLock );
        ClearFlag(devExt->Flags,ExtensionIsLinked);
    }
}


NTSTATUS
GingkoAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    )
{

	NTSTATUS status = STATUS_SUCCESS;

	//UNICODE_STRING Name;

	if( AttachedToDeviceObject == NULL )
	{
		return STATUS_SUCCESS;
	}

	//RtlInitUnicodeString( &Name, L"" );

	///KdPrint( ("Enter AttachDeviceToDeviceStackSafe. %d\n", gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe) );

	///GingkoGetObjectName( SourceDevice, &Name );

	KdPrint( ("Source Device Type: %s. Target Device Type: %s\n", GET_DEVICE_TYPE_NAME(SourceDevice->DeviceType), GET_DEVICE_TYPE_NAME(TargetDevice->DeviceType) ) );

	//return STATUS_SUCCESS;
    if (IS_WINDOWSXP_OR_LATER()) {
        ASSERT( NULL != gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe );
        //KdPrint( ("Calling XP AttachDeviceToDeviceStackSafe. %x\n", gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe) );
		status = STATUS_SUCCESS;
		//KdPrint( ("Success status code is: %d. Source Device: %x, Target: %x, AttachedToDeviceObject: %x \n", 
		//	status, SourceDevice, TargetDevice, *AttachedToDeviceObject ) );

		if( (*AttachedToDeviceObject) != NULL )
		{
			KdPrint( ("AttachedToDeviceObject: %d, %d.\n", (*AttachedToDeviceObject)->ActiveThreadCount, (*AttachedToDeviceObject)->AlignmentRequirement) );
			status = (gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe)( SourceDevice,
                                                                     TargetDevice,
                                                                     AttachedToDeviceObject );
		}
		else
		{
			KdPrint( ("AttachedToDeviceObject Is NULL.\n") );
			status = (gGingkoDynamicFunctions.AttachDeviceToDeviceStackSafe)( SourceDevice,
                                                                     TargetDevice,
                                                                     AttachedToDeviceObject );
			//if( SourceDevice != NULL && TargetDevice != NULL ){
			//	(*AttachedToDeviceObject) = IoAttachDeviceToDeviceStack( SourceDevice, TargetDevice );
			//}

			if( (*AttachedToDeviceObject) != NULL ){
				KdPrint( ("AttachedToDeviceObject: ActiveThreadCount: %d, AlignmentRequirement: %d.\n", (*AttachedToDeviceObject)->ActiveThreadCount, (*AttachedToDeviceObject)->AlignmentRequirement) );
			}else{
				KdPrint( ("AttachedToDeviceObject: return NULL.\n" ));
			}

		}
		KdPrint( ("After call the  Dynamic function, the return code is: %d. \n", status ) );

		

    }

	return status;

}



VOID
GingkoInitDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    Initializes context information for a given device

Arguments:

    DeviceObject - Device to init

Return Value:

    None.

--*/
{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    ASSERT(IS_GINGKO_DEVICE_OBJECT(DeviceObject));

    InitializeListHead( &devExt->CtxList );

    ExInitializeResourceLite( &devExt->CtxLock );

	InitializeListHead( &devExt->ListEntry );

	KeInitializeSpinLock( &devExt->ListSpinLock );

	InitializeListHead( &devExt->ProcessListEntry );

	KeInitializeSpinLock( &devExt->ProcessListSpinLock );

	KeInitializeSemaphore( &devExt->RequestSemaphore, 0L, MAXLONG );

	KeInitializeEvent( &devExt->CreateEvent, SynchronizationEvent, FALSE );

    SetFlag( devExt->Flags, ContextsInitialized );
}


VOID
GingkoCleanupDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    Cleans up the context information for a given device

Arguments:

    DeviceObject - Device to cleanup

Return Value:

    None.

--*/
{
    PGINGKO_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    ASSERT(IS_GINGKO_DEVICE_OBJECT(DeviceObject));

    //
    //  Cleanup if initialized
    //

    if (FlagOn(devExt->Flags,ContextsInitialized)) {

        //
        //  Delete all existing contexts
        //

        //SpyDeleteAllContexts( DeviceObject );
        ASSERT(IsListEmpty( &devExt->CtxList ));

        //
        //  Release resource
        //

        ExDeleteResourceLite( &devExt->CtxLock );

        //
        //  Flag not initialized
        //

        ClearFlag( devExt->Flags, ContextsInitialized );
    }
}

size_t GetCpuCount ()
{
	KAFFINITY activeCpuMap = KeQueryActiveProcessors();
	size_t mapSize = sizeof (activeCpuMap) * 8;
	size_t cpuCount = 0;

	while (mapSize--)
	{
		if (activeCpuMap & 1)
			++cpuCount;

		activeCpuMap >>= 1;
	}

	if (cpuCount == 0)
		return 1;

	return cpuCount;
}
