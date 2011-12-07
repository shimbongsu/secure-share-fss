#include "gingko_readwrite.h"
#include "gingko_interact.h"
#include "gingko_ioqueue.h"
#include "gingko_redfish.h"

extern KSPIN_LOCK gGingkoReadSpinLock;

WCHAR SessionBuffer[64];

static LONG GingkoControlDeviceReferenct = 0;

IO_COMPLETION_ROUTINE GingkoCompletedReadRoutine;

NTSTATUS
GingkoCompletedReadRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
	PKEVENT Event = Context;
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( Context );
    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

	///KdPrint(("Gingko Completed Read from the physical driver.\n"));

	if( Irp->PendingReturned == TRUE )
	{
		IoMarkIrpPending( Irp );
	}

	KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
GingkoCreateCompleteRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
	PKEVENT Event = Context;
	ULONG CreateOptions = 0L;
    PIO_STACK_LOCATION pCurrStack = NULL;
	ULONG dwRequestProcessId = 0L;
	PFILE_OBJECT FileObject = NULL;
	ACCESS_MASK DesiredAccess = 0L;
	ULONG Information = 0L;
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( Context );
    ASSERT(IS_GINGKO_DEVICE_OBJECT( DeviceObject ));

	///KdPrint(("Gingko Completed Read from the physical driver.\n"));

	if( Irp->PendingReturned == TRUE )
	{
		IoMarkIrpPending( Irp );
		return STATUS_MORE_PROCESSING_REQUIRED;
	}

	pCurrStack = IoGetCurrentIrpStackLocation( Irp );

	FileObject = pCurrStack->FileObject;

	dwRequestProcessId = IoGetRequestorProcessId( Irp );

	IoCopyCurrentIrpStackLocationToNext( Irp );

	//ShareAccess = pCurrStack->Parameters.Create.ShareAccess;
	//FileAttributes = pCurrStack->Parameters.Create.FileAttributes;
	CreateOptions = pCurrStack->Parameters.Create.Options;
	DesiredAccess = pCurrStack->Parameters.Create.SecurityContext ? pCurrStack->Parameters.Create.SecurityContext->DesiredAccess : 0xFFFFFFFF;
	
	Information = (ULONG)Irp->IoStatus.Information;

	if( Irp->IoStatus.Status == STATUS_SUCCESS 
		//&& !(FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) 
		//&& !(FileObject->Flags & FO_STREAM_FILE) 
		&& !(CreateOptions & FILE_OPEN_REPARSE_POINT)
		&& !(CreateOptions & FILE_DELETE_ON_CLOSE)
		&& !(CreateOptions & FILE_DIRECTORY_FILE)
		&& !(DesiredAccess & FILE_EXECUTE)) 
	{
		PGINGKO_OBJECT pRelatedGingko = NULL;
		GINGKO_HEADER Header = {0};
		IO_STATUS_BLOCK IoStatusBlock = {0};
		LARGE_INTEGER offset = {0};
		ULONG		   ulHeaderSize = sizeof(GINGKO_HEADER);
		ULONG FileFlags = FileObject->Flags;

		//Header = (PGINGKO_HEADER) TCalloc( 8192 );

		//if( Header == NULL )
		//{
		//	return status;
		//}

		if( Information == FILE_OPENED && FileObject->ReadAccess )
		{
			//KdPrint(("Create FileObject->Flags: %08x, Irp->Flags: %08x.\n", FileObject->Flags, IrpFlags ));
			//FileObject->Flags = FO_HANDLE_CREATED | FO_CLEANUP_COMPLETE | FO_CACHE_SUPPORTED;
			
			if( TRUE ) // Header != NULL )
			{
				
				//status = TCReadDeviceDirect( //
				status = TCReadDeviceEx(  
					((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject,
					FileObject, &Header, &offset, sizeof(GINGKO_HEADER), &IoStatusBlock, 
					IRP_PAGING_IO | IRP_NOCACHE );
									//IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE ); 
			}else
			{
				IoStatusBlock.Status = STATUS_NO_MEMORY;
				status = STATUS_NO_MEMORY;
			}

			FileObject->Flags = FileFlags;

			FileObject->CurrentByteOffset.QuadPart = 0L;

			if( NT_SUCCESS(IoStatusBlock.Status) && IoStatusBlock.Information == sizeof(GINGKO_HEADER) )
			{
				//IoCheckShareAccess( DesiredAccess, DesiredShareAccess, FileObject, SHARE_ACCESS hs, FALSE );
				if( HasGingkoIdentifier( &Header ) )
				{
					GingkoFileFlushCache( FileObject );

					if( !FindWriteFileObject( FileObject,dwRequestProcessId, NULL, FALSE ) )
					{
						PGINGKO_OBJECT pObj = NULL;
						pObj = (PGINGKO_OBJECT)TCalloc(sizeof(GINGKO_OBJECT));
						if( pObj != NULL )
						{
							RtlZeroMemory( pObj, sizeof(GINGKO_OBJECT) );
							RtlCopyMemory( &(pObj->GingkoHeader), &Header, sizeof(GINGKO_HEADER) );
							pObj->FileObject = FileObject;						
							pObj->Permission = GINGKO_PERMISSION_EMPTY;
							pObj->dwProcessId = dwRequestProcessId;
							pObj->Queue.LowerDeviceObject = ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
							pObj->Queue.DeviceObject = ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
							pObj->Queue.IsFilterDevice = TRUE;
							pObj->Queue.SecurityClientContext = NULL;
							//pObj->Queue.CompletionThread = NULL;
							pObj->Queue.CryptoInfo = NULL;
							pObj->Queue.StopPending = FALSE;
							pObj->Queue.ShouldPassthrough = FALSE;
							pObj->Queue.FileObject = FileObject;
							pObj->Queue.GingkoHeader = &(pObj->GingkoHeader);
							pObj->Queue.TotalBytesRead = 0;
							pObj->Queue.TotalBytesWritten = 0;
							pObj->Queue.GingkoObject = pObj;
							pObj->Queue.dwProcessId = dwRequestProcessId;
							GetFileObjectFullName( FileObject, &(pObj->FileName) );
							pObj->Queue.AbstractOffset = sizeof(GINGKO_HEADER);
							ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &(pObj->ListEntry), &gGingkoWriteFileSpinLock );
							EncryptedIoQueueStart( &pObj->Queue );
						}
					}
					//KdPrint(("CREATE: Has Gingko Identifier from Create.\n"));
				}
				//reset the offset
			}

			//if( Header != NULL )
			//{
			//	//ExFreePoolWithTag(Header, GINGKO_POOL_TAG);
			//	TCfree( Header );
			//	Header = NULL;
			//}
		}
	}

	if(Event)
		KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
IoCompletionRoutine(
	IN PDEVICE_OBJECT  DeviceObject,
	IN PIRP  Irp,
	IN PVOID  Context
	)
{
	//DbgPrint(("IoCompletionRoutine!\n"));
	*Irp->UserIosb = Irp->IoStatus;


	if (Irp->MdlAddress)
	{
		//MmUnmapLockedPages( Irp->MdlAddress, 
		//	MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority ) );
		MmUnlockPages( Irp->MdlAddress );
		IoFreeMdl(Irp->MdlAddress);
		Irp->MdlAddress = NULL;
	}

	if (Irp->UserEvent)
		KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
	//return STATUS_SUCCESS;
}



PUNICODE_STRING
GingkoGetFileName(
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS CreateStatus,
    IN OUT PGET_NAME_CONTROL NameControl
    )
{
    POBJECT_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    ULONG size;
    ULONG bufferSize;
	ULONG InitSize = sizeof(OBJECT_NAME_INFORMATION) + 512 * sizeof(WCHAR);

	if( NameControl == NULL )
	{
		return NULL;
	}

    //
    //  Mark we have not allocated the buffer
    //

    NameControl->AllocatedBuffer = ExAllocatePoolWithTag( NonPagedPool, InitSize, GINGKO_POOL_TAG );

    //
    //  Use the small buffer in the structure (that will handle most cases)
    //  for the name
    //

	if( NameControl->AllocatedBuffer == NULL )
	{
		return NULL;
	}


	RtlZeroMemory( NameControl->AllocatedBuffer, InitSize );
	nameInfo = (POBJECT_NAME_INFORMATION)NameControl->AllocatedBuffer;
    bufferSize = InitSize;
	nameInfo->Name.Length = 0;
	nameInfo->Name.MaximumLength = (USHORT)InitSize - sizeof(OBJECT_NAME_INFORMATION);


    //
    //  If the open succeeded, get the name of the file, if it
    //  failed, get the name of the device.
    //
        
    status = ObQueryNameString(
                  FileObject ,
                  nameInfo,
                  (USHORT)InitSize - sizeof(OBJECT_NAME_INFORMATION),
                  &size );

    //
    //  See if the buffer was to small
    //

    if (status == STATUS_INFO_LENGTH_MISMATCH) {

        //
        //  The buffer was too small, allocate one big enough
        //

		if( NameControl->AllocatedBuffer != NULL )
		{
			ExFreePoolWithTag( NameControl->AllocatedBuffer, GINGKO_POOL_TAG );
		}

        bufferSize = sizeof(OBJECT_NAME_INFORMATION) + size + sizeof(WCHAR);

        NameControl->AllocatedBuffer = ExAllocatePoolWithTag( 
                                            NonPagedPool,
                                            bufferSize,
                                            GINGKO_POOL_TAG );

        if (NULL == NameControl->AllocatedBuffer) {

            //
            //  Failed allocating a buffer, return an empty string for the name
            //
			return NULL;
        }

        //
        //  Set the allocated buffer and get the name again
        //

        nameInfo = (POBJECT_NAME_INFORMATION)NameControl->AllocatedBuffer;
		nameInfo->Name.Length = 0;
		nameInfo->Name.MaximumLength = (USHORT)bufferSize - sizeof(OBJECT_NAME_INFORMATION);

        status = ObQueryNameString(
                      FileObject,
                      nameInfo,
                      (USHORT)bufferSize - sizeof(OBJECT_NAME_INFORMATION),
                      &size );
    }

    return &nameInfo->Name;
}


VOID
GingkoGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl
    )
{

    if (NULL != NameControl->AllocatedBuffer) {

        ExFreePool( NameControl->AllocatedBuffer);
        NameControl->AllocatedBuffer = NULL;
    }
}

VOID
GingkoDisplayCreateFileName (
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;
    PUNICODE_STRING name;
    GET_NAME_CONTROL nameControl;

    //
    //  Get current IRP stack
    //

    irpSp = IoGetCurrentIrpStackLocation( Irp );
	
    //
    //  Get the name of this file object
    //

	__try{
		name = GingkoGetFileName( irpSp->FileObject, 
                          Irp->IoStatus.Status, 
                          &nameControl );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Get the Fullname failed..\n"));
		return;
	}

	if( name != NULL ){

		KdPrint( ("Gingko!GingkoDisplayCreateFileName: Read From %wZ\n", 
					   name) );
	}

    //
    //  Cleanup from getting the name
    //

    GingkoGetFileNameCleanup( &nameControl );
}


NTSTATUS
GingkoRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuff = NULL;
	BOOLEAN GingkoFile = FALSE;
	PGINGKO_OBJECT pGingkoObject = NULL;
	PIO_STACK_LOCATION irpsp = NULL;
	PFILE_OBJECT FileObject = NULL;
	SharedNotificationPtr snf = NULL;
	PGINGKO_PROCESS_LIST pProcessList = NULL;
	PGINGKO_DEVICE_EXTENSION DevExt = DeviceObject->DeviceExtension;

	if ( IS_GINGKO_CONTROL_DEVICE_OBJECT(DeviceObject) ) {
		//KdPrint( ("READ: Read Gingko Control Device...") );
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

	if( IsGingkoServerProcess()  || PsIsSystemThread(KeGetCurrentThread()) )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	snf = GingkoReferenceSharedNotification();

	if( gGingkoServerStarted == FALSE || snf == NULL )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	GingkoDereferenceSharedNotification( snf );

	irpsp = IoGetCurrentIrpStackLocation(Irp);

	FileObject = irpsp->FileObject;
	
	if( FsRtlIsPagingFile( FileObject ) )
	{
		KdPrint(("This is a Paging file: %wZ.\n", &(FileObject->FileName)));
	}

	//RtlInitUnicodeString( &TempName, L"\\GingkoDebug\\dbgview.chm" );

	//if( RtlCompareUnicodeString( &TempName, &FileObject->FileName, TRUE ) == 0 )
	//{
	//	LARGE_INTEGER OriginalOffset = irpsp->Parameters.Read.ByteOffset;
	//	long		  OriginalLength = irpsp->Parameters.Read.Length;
	//	KdPrint(("Read DbgView: %I64d (L: %08x, H: %08x) Lenght: %08x\n", OriginalOffset.QuadPart, OriginalOffset.LowPart, 
	//		OriginalOffset.HighPart, OriginalLength));
	//}

	if( FindWriteFileObject( FileObject, IoGetRequestorProcessId(Irp), &pGingkoObject, FALSE ) )
	{
	    NTSTATUS status = 0L;
		if( pGingkoObject != NULL ) //&& pGingkoObject->Queue.CryptoInfo != NULL && ( pGingkoObject->Permission & 0x800F0000 | pGingkoObject->Permission & 0x8000F000 ) )
		{
			if( irpsp->Parameters.Read.Length > 0L )
			{
				if( !Irp->MdlAddress )
				{
					__try{
						ProbeForRead( Irp->UserBuffer, irpsp->Parameters.Read.Length, 1);
						IoAllocateMdl( Irp->UserBuffer, irpsp->Parameters.Read.Length, FALSE, FALSE, Irp );
						if( Irp->MdlAddress )
						{
							MmProbeAndLockPages( Irp->MdlAddress, UserMode, IoReadAccess );
						}
					}__except(EXCEPTION_EXECUTE_HANDLER)
					{
						KdPrint(("Exception Code: %08x. PassThrough. The Write Length is %d, Offset: %I64d.\n", GetExceptionCode(), irpsp->Parameters.Read.Length, irpsp->Parameters.Read.ByteOffset ));
						if( Irp->MdlAddress )
						{
							IoFreeMdl( Irp->MdlAddress );
							Irp->MdlAddress = NULL;
						}
					}
				}
			}

			status = EncryptedIoQueueAddIrp (&pGingkoObject->Queue, Irp);
			
			if( status != STATUS_PENDING )
				TCCompleteDiskIrp ( Irp, status, 0 );

			return status;
		}
	}

	return GingkoPassThrough( DeviceObject, Irp );
}




NTSTATUS
GingkoWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuff = NULL;
	BOOLEAN GingkoFile = FALSE;
	ULONG	dwPermission = 0L;
	PIO_STACK_LOCATION irpsp = NULL;
	PFILE_OBJECT FileObject = NULL;
	PGINGKO_OBJECT pGingkoObject = NULL;
	SharedNotificationPtr snf = NULL;
	KIRQL	irql;
	ULONG	RequestorProcessId = IoGetRequestorProcessId( Irp );
	PGINGKO_DEVICE_EXTENSION DevExt = DeviceObject->DeviceExtension;

	if ( IS_GINGKO_CONTROL_DEVICE_OBJECT(DeviceObject) ) 
	{
		//KdPrint( ("READ: Read Gingko Control Device...") );
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

	if( IsGingkoServerProcess()  || PsIsSystemThread(KeGetCurrentThread()) )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	snf = GingkoReferenceSharedNotification();

	if( gGingkoServerStarted == FALSE || snf == NULL )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	GingkoDereferenceSharedNotification( snf );
	

	dwPermission = GetPermissionUnderDecryptionByProcessId( RequestorProcessId, &irql );

	if( dwPermission != 0 )
	{
		if( (dwPermission & 0x8FF0F000) != 0x8FF0F000 )
		{
			Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest( Irp, IO_NO_INCREMENT );
			return STATUS_INVALID_DEVICE_REQUEST;		
		}
	}

	return GingkoPassThrough( DeviceObject, Irp );
}


PVOID GetBufferOfIrp(PIRP Irp){

	if( Irp->MdlAddress )
	{
		return MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
	}else if( Irp->AssociatedIrp.SystemBuffer ){
		return Irp->AssociatedIrp.SystemBuffer;
	}else if( Irp->UserBuffer )
	{
		return Irp->UserBuffer;
	}
	return NULL;
}

NTSTATUS
GingkoQueryDirectoryInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	ULONG dwRequestProcessId = IoGetRequestorProcessId(Irp);
	PIO_STACK_LOCATION irpsp = NULL;
	KEVENT WaitEvent;
	KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

	irpsp = IoGetCurrentIrpStackLocation(Irp);
	//KdPrint(("ProcessId: %d query directory information. Class: %d, Filename: %wZ\n", dwRequestProcessId, 
	//	irpsp->Parameters.QueryDirectory.FileInformationClass, irpsp->Parameters.QueryDirectory.FileName ));

	if( IsProcessUnderDecryption( dwRequestProcessId ) )
	{
		WCHAR *pFileName = NULL;
		ULONG dwFileNameLength = 0;
		PVOID InputBuffer;
		NTSTATUS status = STATUS_SUCCESS;

		IoCopyCurrentIrpStackLocationToNext( Irp );

		IoSetCompletionRoutine( 
			Irp, 
			GingkoCompletedReadRoutine,
			&WaitEvent, 
			TRUE,
			TRUE,
			TRUE
			);

		status = IoCallDriver( ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

		if( STATUS_PENDING == status )
		{
			KdPrint(("WaitForSingleObject IRP STATUS_PENDING 2\n"));
			KeWaitForSingleObject( &WaitEvent, Executive, KernelMode, FALSE, NULL );
		}
		status = Irp->IoStatus.Status;

		InputBuffer = GetBufferOfIrp( Irp );
		
		if( NT_SUCCESS( status ) && InputBuffer != NULL )
		{
			switch( irpsp->Parameters.QueryDirectory.FileInformationClass )
			{
			case FileDirectoryInformation:
				dwFileNameLength = ((PFILE_DIRECTORY_INFORMATION)InputBuffer)->FileNameLength;
				pFileName = ((PFILE_DIRECTORY_INFORMATION)InputBuffer)->FileName;
				break;
			case FileFullDirectoryInformation:
				dwFileNameLength = ((PFILE_FULL_DIR_INFORMATION)InputBuffer)->FileNameLength;
				pFileName = ((PFILE_FULL_DIR_INFORMATION)InputBuffer)->FileName;
				break;
			case FileBothDirectoryInformation:
				dwFileNameLength = ((PFILE_BOTH_DIR_INFORMATION)InputBuffer)->FileNameLength;
				pFileName = ((PFILE_BOTH_DIR_INFORMATION)InputBuffer)->FileName;
				break;
			}

			if( pFileName != NULL && dwFileNameLength > 0 )
			{
				PGINGKO_OBJECT pObject = NULL;
				///try to find the file from the linkedlist
				if( FindGingkoObjectByFileName( pFileName, dwFileNameLength, dwRequestProcessId, &pObject ) )
				{
					switch( irpsp->Parameters.QueryDirectory.FileInformationClass )
					{
					case FileDirectoryInformation:
						((PFILE_DIRECTORY_INFORMATION)InputBuffer)->EndOfFile.QuadPart = pObject->GingkoHeader.SizeOfFile;
						break;
					case FileFullDirectoryInformation:
						((PFILE_FULL_DIR_INFORMATION)InputBuffer)->EndOfFile.QuadPart = pObject->GingkoHeader.SizeOfFile;
						break;
					case FileBothDirectoryInformation:
						((PFILE_BOTH_DIR_INFORMATION)InputBuffer)->EndOfFile.QuadPart = pObject->GingkoHeader.SizeOfFile;
						break;
					}
				}
			}
		}

		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return status;
	}

	return GingkoPassThrough( DeviceObject, Irp );
}

NTSTATUS
GingkoQueryInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuff = NULL;
	BOOLEAN GingkoFile = FALSE;
	PGINGKO_OBJECT pGingkoObject = NULL;
	PIO_STACK_LOCATION irpsp = NULL;
	PFILE_OBJECT FileObject = NULL;
	PGINGKO_DEVICE_EXTENSION DevExt = DeviceObject->DeviceExtension;

	if ( IS_GINGKO_CONTROL_DEVICE_OBJECT(DeviceObject) ) {
		//KdPrint( ("READ: Read Gingko Control Device...") );
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

	if( IsGingkoServerProcess() )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	irpsp = IoGetCurrentIrpStackLocation(Irp);

	FileObject = irpsp->FileObject;

	//KdPrint(("ProcessId: %d query file information: %d. File: %wZ\n", IoGetRequestorProcessId(Irp), irpsp->Parameters.QueryFile.FileInformationClass, &(FileObject->FileName) ));

	if( FindWriteFileObject( FileObject, IoGetRequestorProcessId(Irp), &pGingkoObject, FALSE ) )
	{

			KEVENT WaitEvent;
			PFILE_STANDARD_INFORMATION FileStdInformation;

			KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );

			IoCopyCurrentIrpStackLocationToNext( Irp );

			IoSetCompletionRoutine( 
				Irp, 
				GingkoCompletedReadRoutine,
				&WaitEvent, 
				TRUE,
				TRUE,
				TRUE
				);

			status = IoCallDriver( ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

			if( STATUS_PENDING == status )
			{
				KdPrint(("WaitForSingleObject IRP STATUS_PENDING 2\n"));
				KeWaitForSingleObject( &WaitEvent, Executive, KernelMode, FALSE, NULL );
			}

			status = Irp->IoStatus.Status;

			if( !NT_SUCCESS(status) )
			{
				IoCompleteRequest( Irp, IO_NO_INCREMENT );
				return status;
			}

			switch( irpsp->Parameters.QueryFile.FileInformationClass )
			{
			case FileStandardInformation:
				FileStdInformation = (PFILE_STANDARD_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
				if( FileStdInformation != NULL )
				{
					//KdPrint(("Get the FileSize of QueryInformation.Information: %08x.Status: %08x. Size: %I64d==%I64d\n", 
					//	Irp->IoStatus.Information, Irp->IoStatus.Status, FileStdInformation->EndOfFile.QuadPart,
					//	pGingkoObject->GingkoHeader.SizeOfFile
					//	));
					__try
					{
						FileStdInformation->EndOfFile.QuadPart = (LONGLONG) pGingkoObject->GingkoHeader.SizeOfFile;
					}__except(EXCEPTION_EXECUTE_HANDLER)
					{
						KdPrint(("Handle an exception while set the EndOfFile.\n"));
					}
				}
				break;
			case FilePositionInformation:
				KdPrint(("Query FilePosition Information.\n"));
				//PFILE_POSITION_INFORMATION FilePosInformation = (PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
				//if( FilePosInformation != NULL )
				//{
				//	KdPrint(("Query FilePosition Information.\n"));
				//	if( FilePosInformation->CurrentByteOffset.QuadPart > pGingkoObject->OffsetBytes.QuadPart )
				//	{
				//		FilePosInformation->CurrentByteOffset.QuadPart = pGingkoObject->OffsetBytes.QuadPart;
				//	}
				//}
				break;
			case FileEndOfFileInformation:
				 ((PFILE_END_OF_FILE_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->EndOfFile.QuadPart = (LONGLONG) pGingkoObject->GingkoHeader.SizeOfFile;
				 break;
			case FileAllocationInformation:
				//((PFILE_ALLOCATION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->AllocationSize.QuadPart = (LONGLONG) pGingkoObject->GingkoHeader.SizeOfFile;
				break;
			case FileAllInformation:
				//((PFILE_ALL_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->PositionInformation.CurrentByteOffset.QuadPart += InvisiblePartSize;
				((PFILE_ALL_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->StandardInformation.EndOfFile.QuadPart += (LONGLONG) pGingkoObject->GingkoHeader.SizeOfFile;
				break;
			}
			IoCompleteRequest( Irp, NT_SUCCESS(status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT );
			return status;
	}

	return GingkoPassThrough( DeviceObject, Irp );
}

NTSTATUS
GingkoSetInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID pBuff = NULL;
	BOOLEAN GingkoFile = FALSE;
	PGINGKO_OBJECT pGingkoObject = NULL;
	PIO_STACK_LOCATION irpsp = NULL;
	ULONG dwProcessId = 0L;
	PFILE_OBJECT FileObject = NULL;
	SharedNotificationPtr snf = NULL;
	PGINGKO_DEVICE_EXTENSION DevExt = DeviceObject->DeviceExtension;

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

///problem is come from the basic wait for get the crypto.
	if ( IS_GINGKO_CONTROL_DEVICE_OBJECT(DeviceObject) ) {
		//KdPrint( ("READ: Read Gingko Control Device...") );
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return STATUS_INVALID_DEVICE_REQUEST;
	}

	snf = GingkoReferenceSharedNotification();

	if( gGingkoServerStarted == FALSE || snf == NULL )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	GingkoDereferenceSharedNotification( snf );

	if( IsGingkoServerProcess() )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}


	irpsp = IoGetCurrentIrpStackLocation(Irp);

	FileObject = irpsp->FileObject;

	dwProcessId = IoGetRequestorProcessId(Irp);
	//KdPrint(("ProcessId: %d set Information.Parameter FIC: %d, File: %wZ.\n", dwProcessId, irpsp->Parameters.SetFile.FileInformationClass, &(FileObject->FileName) ));

	if( FindWriteFileObject( FileObject, dwProcessId, &pGingkoObject, FALSE ) )
	{
		KEVENT WaitEvent;

		KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );

		IoCopyCurrentIrpStackLocationToNext( Irp );

		IoSetCompletionRoutine( 
			Irp, 
			GingkoCompletedReadRoutine,
			&WaitEvent, 
			TRUE,
			TRUE,
			TRUE
			);

		status = IoCallDriver( ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

		if( STATUS_PENDING == status )
		{
			KeWaitForSingleObject( &WaitEvent, Executive, KernelMode, FALSE, NULL );
		}

		status = Irp->IoStatus.Status;
		switch( irpsp->Parameters.SetFile.FileInformationClass )
		{
		case FilePositionInformation:
			{
				PFILE_POSITION_INFORMATION FilePositionInformation = (PFILE_POSITION_INFORMATION) GetBufferOfIrp( Irp );
			KdPrint(("SetFileInformation Position: %I64d. Pwd: %wZ\n", FilePositionInformation->CurrentByteOffset,&FileObject->FileName));
			}
			break;
		case FileEndOfFileInformation:
			{
			PFILE_END_OF_FILE_INFORMATION FileEndInformation = (PFILE_END_OF_FILE_INFORMATION) GetBufferOfIrp( Irp );
			KdPrint(("SetFileInformation End of file: %I64d. Pwd: %wZ\n", FileEndInformation->EndOfFile,&(FileObject->FileName)));
			}
			break;
		}
		IoCompleteRequest( Irp, IO_NO_INCREMENT );
		return status;

	}

	return GingkoPassThrough( DeviceObject, Irp );
}


NTSTATUS
GingkoCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
	KEVENT WaitEvent = {0};
    NTSTATUS status = 0;
	PFILE_OBJECT FileObject = NULL;
	ULONG_PTR Information = 0L;
	ULONG IrpFlags = 0L;
	ULONG dwPermission = 0L;
	KIRQL irql;
	ULONG	dwRequestProcessId;
    ULONG CreateOptions = 0L;
	ACCESS_MASK DesiredAccess = 0L;
	SharedNotificationPtr snf = NULL;
	PIO_STACK_LOCATION pCurrStack = NULL;
	BOOLEAN isSynchronous = TRUE;
	//UNICODE_STRING FileOpenName;


    if (DeviceObject == gControlDeviceObject) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;
		GingkoControlDeviceReferenct ++;
        gControlDeviceState = OPENED;
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

    ASSERT( IS_GINGKO_DEVICE_OBJECT( DeviceObject ) );

	if( ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->DiskDeviceObject == NULL )
	{
		//KdPrint(("CREATE: DiskDeviceObject is NULL. Pass.\n"));
		return GingkoPassThrough( DeviceObject, Irp );
	}

	if( IsGingkoServerProcess()  || PsIsSystemThread(KeGetCurrentThread()) )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	snf = GingkoReferenceSharedNotification();

	if( gGingkoServerStarted == FALSE || snf == NULL )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	GingkoDereferenceSharedNotification( snf );
	
	pCurrStack = IoGetCurrentIrpStackLocation( Irp );

	FileObject = pCurrStack->FileObject;

	//RelatedFileObject = FileObject->RelatedFileObject;
	//if( RelatedFileObject != NULL )
	//{
	//	KdPrint(("Related FileObject: %wZ\n", &(RelatedFileObject->FileName)));
	//	if( RtlCompareUnicodeString( &(RelatedFileObject->FileName), &(FileObject->FileName), TRUE ) != 0 )
	//	{
	//		RelatedFileObject = NULL;
	//	}
	//}

	dwRequestProcessId = IoGetRequestorProcessId( Irp );

	dwPermission = GetPermissionUnderDecryptionByProcessId( dwRequestProcessId, &irql );


	IoCopyCurrentIrpStackLocationToNext( Irp );

	//ShareAccess = pCurrStack->Parameters.Create.ShareAccess;
	//FileAttributes = pCurrStack->Parameters.Create.FileAttributes;
	CreateOptions = pCurrStack->Parameters.Create.Options;
	DesiredAccess = pCurrStack->Parameters.Create.SecurityContext ? pCurrStack->Parameters.Create.SecurityContext->DesiredAccess : 0xFFFFFFFF;

	if( dwPermission != 0 )
	{
		///Assert the CreateOption to Open this file only. 
		ULONG dwCreateOption = pCurrStack->Parameters.Create.Options;
		pCurrStack->Parameters.Create.Options = (dwCreateOption & 0x00FFFFFF) | FILE_OPEN << 24;
		//pCurrStack->Parameters.Create.SecurityContext ? pCurrStack->Parameters.Create.SecurityContext->DesiredAccess : 0xFFFFFFFF;
	}


	IoSetCompletionRoutine( 
		Irp, 
		GingkoCreateCompleteRoutine,
		&WaitEvent, 
		TRUE,
		TRUE,
		TRUE
		);

	KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );

	status = IoCallDriver( ((PGINGKO_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

	if( STATUS_PENDING == status )
	{
		//KdPrint(("WaitForSingleObject IRP STATUS_PENDING 4\n"));
		KeWaitForSingleObject( &WaitEvent, Executive, KernelMode, FALSE, NULL );
	}
	
	status = Irp->IoStatus.Status;
	Information = Irp->IoStatus.Information;
	IrpFlags = Irp->Flags;

	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return status;
}


NTSTATUS
GingkoClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	PFILE_OBJECT FileObject = NULL;
	PIO_STACK_LOCATION pCurrStack;
    //
    //  See if they are closing the control device object for the filter.
    //

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

    if (DeviceObject == gControlDeviceObject) {

        //
        //  If the specified debug level is set, output what operation
        //  we are seeing to the debugger.

		GingkoControlDeviceReferenct --;

		if( GingkoControlDeviceReferenct == 0 )
		{
			GingkoCloseControlDevice();
			//GingkoUnInitializeDecryptProcessTable();
		}

        //
        //  We have completed all processing for this IRP, so tell the 
        //  I/O Manager.  This IRP will not be passed any further down
        //  the stack since no drivers below FileSpy care about this 
        //  I/O operation that was directed to FileSpy.
        //

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT( IS_GINGKO_DEVICE_OBJECT( DeviceObject ) );
 
	if( IsGingkoServerProcess() )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	pCurrStack = IoGetCurrentIrpStackLocation( Irp );

	if( pCurrStack->FileObject != NULL )
	{
		PGINGKO_OBJECT pObject = NULL;

		__try{
			
			if( FindWriteFileObject( pCurrStack->FileObject, IoGetRequestorProcessId(Irp), &pObject, TRUE ) )
			{
				KdPrint(("Close file.\n"));
				//GingkoFileFlushCache( pCurrStack->FileObject );
				//status = EncryptedIoQueueAddIrp( &(pObject->Queue), Irp );
				//if( status != STATUS_PENDING )
				//	TCCompleteDiskIrp ( Irp, status, 0 );
				EncryptedIoQueueStop(&(pObject->Queue));
				TCfree( pObject );
			}
		}__except(EXCEPTION_EXECUTE_HANDLER){
			KdPrint(("Exception when close file.\n"));
		}

		//DeleteGingkoObject( pCurrStack->FileObject ); ////Attemp to delete the Gingko Object.
	}

    return GingkoPassThrough( DeviceObject, Irp );
}

NTSTATUS
GingkoCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
	PFILE_OBJECT FileObject = NULL;
	PIO_STACK_LOCATION pCurrStack;


    //
    //  See if they are closing the control device object for the filter.
    //

    if (DeviceObject == gControlDeviceObject) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

    ASSERT( IS_GINGKO_DEVICE_OBJECT( DeviceObject ) );

	
 
	if( IsGingkoServerProcess() )
	{
		return GingkoPassThrough( DeviceObject, Irp );
	}

	pCurrStack = IoGetCurrentIrpStackLocation( Irp );

	FileObject = pCurrStack->FileObject;

	if( FileObject != NULL )
	{
		PGINGKO_OBJECT pGingkoObject = NULL;

		__try{
		if( FindWriteFileObject( FileObject, IoGetRequestorProcessId(Irp), &pGingkoObject, FALSE ) )
		{		
			if( pGingkoObject != NULL )
			{
				NTSTATUS status = 0L;
				KdPrint(("Cleanup: Cleanup the current OffsetBytes of this FileObject.\n"));
				FileObject->CurrentByteOffset.QuadPart = 0L;
				//FileObject->FsContext2 = NULL;
				//GingkoFileFlushCache( FileObject );

				///EncryptedIoQueueCleanup( &(pGingkoObject->Queue) );
				status = EncryptedIoQueueAddIrp (&pGingkoObject->Queue, Irp);

				if( status != STATUS_PENDING )
					TCCompleteDiskIrp ( Irp, status, 0 );
				return status;
				//EncryptedIoQueueStop( &(pGingkoObject->Queue) );
				//TCfree( pGingkoObject );
				//return status;
			}
		}
		}__except(EXCEPTION_EXECUTE_HANDLER){}
	}

    return GingkoPassThrough( DeviceObject, Irp );
}

NTSTATUS
GingkoDeviceIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

	REDFISH_NETWORK_DISPATCH( DeviceObject, Irp );

	IrpSp = IoGetCurrentIrpStackLocation( Irp );

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;

	switch( IrpSp->Parameters.DeviceIoControl.IoControlCode )
	{
	case GINGKO_DEVICE_START:     ///The Service of Decrypt and Encrypt will be started.
		{
			PRKPROCESS Process = NULL;
			if( gGingkoServerStarted == FALSE )
			{
				if( IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(SharedNotification) )
			 	{
					gGingkoNotification = ExAllocatePoolWithTag( NonPagedPool, sizeof(SharedNotification), GINGKO_POOL_TAG );// Irp->AssociatedIrp.SystemBuffer;
					if( gGingkoNotification )
					{
						PETHREAD pThread = NULL;
						PKEVENT  pEventForRead;
						PKEVENT  pEventForWrite;
						RtlCopyMemory( gGingkoNotification, Irp->AssociatedIrp.SystemBuffer, sizeof(SharedNotification) );
						
						if( gGingkoNotification->cbStruct != sizeof(SharedNotification) )
						{
							ExFreePoolWithTag( gGingkoNotification, GINGKO_POOL_TAG );
							gGingkoNotification = NULL;
							Irp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
							break;
						}

						gGingkoNotification->ServerProcess = (HANDLE) IoGetCurrentProcess();
						if( gGingkoNotification->ServerThread == NULL )
						{
							gGingkoNotification->ServerThread = (HANDLE) KeGetCurrentThread();
						}else{
							if( NT_SUCCESS(ObReferenceObjectByHandle( 
								gGingkoNotification->ServerThread, 
								THREAD_SET_CONTEXT, 
								*PsThreadType,
								KernelMode, 
								(PVOID *)&pThread, 
								NULL )))
							{
								gGingkoNotification->ServerThread = (HANDLE) pThread;
							}
						}

						if( gGingkoNotification->WriteEvent != NULL )
						{
							if( NT_SUCCESS( ObReferenceObjectByHandle( gGingkoNotification->WriteEvent, 
								EVENT_ALL_ACCESS, *ExEventObjectType, KernelMode, &pEventForWrite, NULL ))){
								gGingkoNotification->WriteEvent = (HANDLE) pEventForWrite;
							}else{
								gGingkoNotification->WriteEvent = NULL;
							}
						}

						if( gGingkoNotification->ReadEvent != NULL )
						{
							if( NT_SUCCESS( ObReferenceObjectByHandle( gGingkoNotification->ReadEvent, 
								EVENT_ALL_ACCESS, *ExEventObjectType, KernelMode, &pEventForRead, NULL ))){
								gGingkoNotification->ReadEvent = (HANDLE) pEventForRead;
							}else{
								gGingkoNotification->ReadEvent  = NULL;
							}
						}

						gGingkoServerStarted = TRUE;
						gControlDeviceState =  OPENED;
					}
				}
			}
		}
		break;
	case GINGKO_DEVICE_STOP:      ///The service of decrypt and encrypt will be stoped.
		{
			KdPrint(("IoControl Of this Device will be stoped.\n"));
			//ExReleaseFastMutex( &gGingkoFastMutex );
			GingkoCloseControlDevice();
			//GingkoUnInitializeDecryptProcessTable();
			KdPrint(("GingkoCloseControlDevice called.\n"));
		}
		break;
	case GINGKO_GET_SESSION:
		{
			PVOID TargBuff = Irp->AssociatedIrp.SystemBuffer; // != NULL ? Irp->AssociatedIrp.SystemBuffer : Irp->UserBuffer;//IrpSp->Parameters.DeviceIoControl.Type3InputBuffer == NULL ? Irp->AssociatedIrp.SystemBuffer : IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
			ULONG BuffLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
			if( SessionBuffer == NULL )
			{
				Irp->IoStatus.Information = 0;
			}else if( TargBuff == NULL )
			{
				Irp->IoStatus.Information = 0;
			}else{
				PMDL pMdl = NULL;
				if( Irp->MdlAddress != NULL )
				{
					TargBuff = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
					RtlCopyMemory( TargBuff, SessionBuffer, BuffLength );
				}else{
					PVOID pSysBuf = TargBuff;
					PVOID pTemp = NULL;
					RtlCopyMemory( pSysBuf, SessionBuffer, BuffLength );
					Irp->IoStatus.Information = BuffLength;
				}
				
			}
		}
		break;
	case GINGKO_PUT_SESSION:
		{
			if( IrpSp->Parameters.DeviceIoControl.InputBufferLength == 0 
				&& IrpSp->Parameters.DeviceIoControl.OutputBufferLength == 0 )
			{
				if( SessionBuffer != NULL )
				{
					RtlZeroMemory( SessionBuffer, 128 );
				}
				Irp->IoStatus.Information = 0;
			}else if( IrpSp->Parameters.DeviceIoControl.InputBufferLength != 0 )
			{
				if( SessionBuffer == NULL )
				{
					Irp->IoStatus.Information = 0;
				}else{
					RtlZeroMemory( SessionBuffer, GINGKO_SESSION_MAX_BUFFER );					
					if( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer != NULL )
					{
						RtlCopyMemory( SessionBuffer, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, 
							IrpSp->Parameters.DeviceIoControl.InputBufferLength );
					}else if( Irp->AssociatedIrp.SystemBuffer != NULL )
					{
						RtlCopyMemory( SessionBuffer, Irp->AssociatedIrp.SystemBuffer, 
							IrpSp->Parameters.DeviceIoControl.InputBufferLength );
					}
					Irp->IoStatus.Information = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
				}
			}
		}
		break;
	case GINGKO_GET_REQUEST:
		GingkoRetrievPublicKeyRequest( Irp, 
			NULL,
			IrpSp->Parameters.DeviceIoControl.InputBufferLength,
			Irp->MdlAddress ? MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority ) : Irp->UserBuffer,
			IrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case GINGKO_PUT_RESPONSE:
		GingkoPublicKeyResponse( Irp, 
			Irp->AssociatedIrp.SystemBuffer,
			IrpSp->Parameters.DeviceIoControl.InputBufferLength,
			NULL );
		break;
	case GINGKO_SET_ENCRYPT_START:
		GingkoSetEncryptionProcess( Irp, (LONG_PTR)PsGetCurrentProcessId(), TRUE );
		break;
	case GINGKO_SET_ENCRYPT_END:
		GingkoSetEncryptionProcess( Irp, (LONG_PTR)PsGetCurrentProcessId(), FALSE );
		break;
	case GINGKO_QUERY_PROCESS_PERMISSION:
		//GingkoSetEncryptionProcess( Irp, (LONG_PTR)PsGetCurrentProcessId(), FALSE );
		{
			ULONG dwProcessId = 0;
			ULONG dwPermission = 0;
			KIRQL irql;
			if( IrpSp->Parameters.DeviceIoControl.Type3InputBuffer != NULL )
			{
				RtlCopyMemory( &dwProcessId, IrpSp->Parameters.DeviceIoControl.Type3InputBuffer, 
					IrpSp->Parameters.DeviceIoControl.InputBufferLength );
			}else if( Irp->AssociatedIrp.SystemBuffer != NULL )
			{
				RtlCopyMemory( &dwProcessId, Irp->AssociatedIrp.SystemBuffer, 
					IrpSp->Parameters.DeviceIoControl.InputBufferLength );
			}
			if( dwProcessId == 0 )
			{
				Irp->IoStatus.Information = sizeof(ULONG);
				break;
			}

			dwPermission = GetPermissionUnderDecryptionByProcessId( dwProcessId, &irql );

			if( dwPermission != 0x0FFFFFFF )
			{
				PVOID pTargetBuf = Irp->MdlAddress ? MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority ) : Irp->UserBuffer;
				RtlCopyMemory(  pTargetBuf, &dwPermission, sizeof(ULONG) );
				Irp->IoStatus.Information = sizeof(ULONG);
			}else{
				Irp->IoStatus.Information = 0L;
			}


		}
		break;
	}

	return STATUS_SUCCESS;
}
