#include "gingko_fastio.h"

/////////////////////////////////////////////////////////////////////////////
//
//                      FastIO Handling routines
//
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SfFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

		//KdPrint( ( "FastIOCheckIfPossible: %wZ\n", &FileObject->FileName ) );

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible )) {

			//if( !IsGingkoServerProcess() )
			//{
			
				if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				{
					return FALSE;
				}
			//}

            return (fastIoDispatch->FastIoCheckIfPossible)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        CheckForReadOperation,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
	BOOLEAN GingkoFile = FALSE;
	PGINGKO_OBJECT pGingkoObject = NULL;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        
		ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

		//KdPrint( ( "Read File: %wZ\n", &FileObject->FileName ) );

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {
			BOOLEAN FioReadStatus = FALSE;
			LONGLONG QuadPart = FileOffset->QuadPart;
			//UNICODE_STRING TempName;
			//RtlInitUnicodeString( &TempName, L"\\GingkoDebug\\dbgview.chm" );

			//if( RtlCompareUnicodeString( &TempName, &FileObject->FileName, TRUE ) == 0 )
			//{
			//	LARGE_INTEGER OriginalOffset = *FileOffset;
			//	long		  OriginalLength = Length;
			//	KdPrint(("FASTIO: Read DbgView: %I64d (L: %08x, H: %08x) Lenght: %08x\n", OriginalOffset.QuadPart, OriginalOffset.LowPart, 
			//		OriginalOffset.HighPart, OriginalLength));
			//	return FALSE;
			//}

			if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
			{
				return FALSE;
			}

            FioReadStatus = (fastIoDispatch->FastIoRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );

			return FioReadStatus;
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    
    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {

			if( IsProcessUnderDecryption( (ULONG) PsGetCurrentProcessId() ) )
			{
				return FALSE;
			}

            return (fastIoDispatch->FastIoWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				//if( FindWriteFileObject( FileObject,(ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				//{
				//	KdPrint(("FastIo QueryBasicInfo.\n"));
				//	return FALSE;
				//}
			//}
            return (fastIoDispatch->FastIoQueryBasicInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
	BOOLEAN ret;
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo )) {
			
			//PGINGKO_OBJECT pObject = NULL;
			if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
			{
				
				return FALSE;
			}

            ret = (fastIoDispatch->FastIoQueryStandardInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
			return ret;
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {

            return (fastIoDispatch->FastIoLock)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				//if( FindWriteFileObject( FileObject, NULL, FALSE ) )
				//{
				//	return FALSE;
				//}
			//}
            return (fastIoDispatch->FastIoUnlockSingle)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )


{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;

        if (nextDeviceObject) {

            fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {
				//if( IsGingkoServerProcess() == FALSE )
				//{
					//if( FindWriteFileObject( FileObject, NULL, FALSE ) )
					//{
					//	return FALSE;
					//}
				//}
                return (fastIoDispatch->FastIoUnlockAll)(
                            FileObject,
                            ProcessId,
                            IoStatus,
                            nextDeviceObject );
            }
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				//if( FindWriteFileObject( FileObject, NULL, FALSE ) )
				//{
				//	return FALSE;
				//}
			//}
            return (fastIoDispatch->FastIoUnlockAllByKey)(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) {

			//if( !IsGingkoServerProcess() )
			//{

				//if( FindWriteFileObject( FileObject, NULL, FALSE ) )
				//{
				//	KdPrint(("IoDeviceControl on an encrypted files.\n"));
				//	return FALSE;
				//}
			///}

            return (fastIoDispatch->FastIoDeviceControl)(
                        FileObject,
                        Wait,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        IoControlCode,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


VOID
SfFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )


{
    PGINGKO_DEVICE_EXTENSION devExt;
    ASSERT(IS_GINGKO_DEVICE_OBJECT( SourceDevice ));

    devExt = SourceDevice->DeviceExtension;

    //
    //  Display name information
    //


    //
    //  Detach from the file system's volume device object.
    //

    //SfCleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );
}


BOOLEAN
SfFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )


{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {
			//if( FileObject != NULL )
			//{
			//	KdPrint(("FileObject FastIoQueryNetworkOpenInfo: %wZ.\n", &FileObject->FileName));
			//}
			//if( IsGingkoServerProcess() == FALSE )
			//{
				if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				{
					return FALSE;
				}
			//}
            return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
	BOOLEAN Status = FALSE;
	
    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

		KdPrint( ("FastIoMdlRead: %wZ.\n", &FileObject->FileName ) );

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {
			BOOLEAN IsServerProcess = IsGingkoServerProcess();

			//if( IsServerProcess == FALSE )
			//{
				if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				{
					KdPrint(("This is a Gingko Encrypted File FastIOReadMdl. \n"));
					return FALSE;
				}	
			//}
			
            Status = (fastIoDispatch->MdlRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
			return Status;
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				//if( FindWriteFileObject( FileObject, NULL, FALSE ) )
				//{
					//return FALSE;
				//}
			//}
            return (fastIoDispatch->MdlReadComplete)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
			if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
			{
				return FALSE;
			}
			//}
            return (fastIoDispatch->PrepareMdlWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )


{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {
            return (fastIoDispatch->MdlWriteComplete)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


/*********************************************************************************
        UNIMPLEMENTED FAST IO ROUTINES
        
        The following four Fast IO routines are for compression on the wire
        which is not yet implemented in NT.  
        
        NOTE:  It is highly recommended that you include these routines (which
               do a pass-through call) so your filter will not need to be
               modified in the future when this functionality is implemented in
               the OS.
        
        FastIoReadCompressed, FastIoWriteCompressed, 
        FastIoMdlReadCompleteCompressed, FastIoMdlWriteCompleteCompressed
**********************************************************************************/


BOOLEAN
SfFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    )


{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				{
					return FALSE;
				}
			//}
            return (fastIoDispatch->FastIoReadCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {
			//if( IsGingkoServerProcess() == FALSE )
			//{
				//if( FindWriteFileObject( FileObject, (ULONG)PsGetCurrentProcessId(), NULL, FALSE ) )
				//{
				//	//return FALSE;
				//}
			//}
            return (fastIoDispatch->FastIoWriteCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed )) {
            return (fastIoDispatch->MdlReadCompleteCompressed)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )


{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {
            return (fastIoDispatch->MdlWriteCompleteCompressed)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    )

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

	if( DeviceObject == gControlDeviceObject )
	{
		KdPrint(("QueryOpen for Gingko Control Device Object.\n"));
		RtlZeroMemory(NetworkInformation, sizeof(FILE_NETWORK_OPEN_INFORMATION) );
		return TRUE;
	}

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

        //
        //  Pass through logic for this type of Fast I/O
        //

        nextDeviceObject = ((PGINGKO_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
			PDEVICE_OBJECT oldDev = irpSp->DeviceObject;
            //
            //  Before calling the next filter, we must make sure their device
            //  object is in the current stack entry for the given IRP
            //
            irpSp->DeviceObject = nextDeviceObject;

			//KdPrint(("FastIoQueryOpen: %p==%p==%p\n", DeviceObject, oldDev, nextDeviceObject));


			result = (fastIoDispatch->FastIoQueryOpen)(
							Irp,
							NetworkInformation,
							nextDeviceObject );

            //
            //  Always restore the IRP back to our device object
            //
            irpSp->DeviceObject = oldDev;
            return result;
        }
    }
    return FALSE;
}
