#include "gingko_filter.h"
#include "gingko_apc.h"
#include "gingko_interact.h"
#include "gingko_ioqueue.h"

KSPIN_LOCK  gGingkoObjectSpinLock;

GINGKO_OBJECT gGingkoObjectList;

PGINGKO_PROCESS_HOOK_LIST gGingkoProcessHookRoot = NULL;

BOOLEAN			gGingkoServerStarted = FALSE;

LIST_ENTRY		gGingkoWriteFileListEntry;
KSPIN_LOCK		gGingkoWriteFileSpinLock;

SharedNotificationPtr gGingkoNotification = NULL;

PUNICODE_STRING
GingkoGetFileName(
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS CreateStatus,
    IN OUT PGET_NAME_CONTROL NameControl
    );

VOID
GingkoGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl
    );

IO_COMPLETION_ROUTINE PagingIoCompletedReadRoutine;
IO_COMPLETION_ROUTINE PagingIoCompletedReadRoutine;
IO_COMPLETION_ROUTINE PagingIoCompletedReadRoutine;
IO_COMPLETION_ROUTINE PagingIoCompletedReadRoutine;


PUNICODE_STRING GetFileObjectFullName( PFILE_OBJECT FileObject, PUNICODE_STRING pUnicode )
{
	NTSTATUS Status = STATUS_SUCCESS;
	CHAR nibuf[512];
	POBJECT_NAME_INFORMATION pName;
	//PUNICODE_STRING pUnicode = NULL;
	PUNICODE_STRING pRetr = NULL;
	PWSTR bmf = NULL;
	ULONG MaxLength = 0;

	PDEVICE_OBJECT pFirst = IoGetRelatedDeviceObject( FileObject );

	pName = (POBJECT_NAME_INFORMATION) nibuf;

	while( pFirst != NULL )
	{
		if( IS_GINGKO_DEVICE_OBJECT( pFirst ) )
		{
			//KdPrint(("%wZ%wZ\n", &(((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->DeviceName), &FileObject->FileName ));
			//pUnicode = ExAllocatePoolWithTag( NonPagedPool, sizeof(UNICODE_STRING), GINGKO_POOL_TAG );
			MaxLength = FileObject->FileName.Length + ((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->UserNames.Length ;

			bmf = ExAllocatePoolWithTag( NonPagedPool, MaxLength + sizeof(WCHAR), GINGKO_POOL_TAG );
			if( bmf == NULL )
			{
				KdPrint(("Can not allocate for temp buffer.\n"));
				return NULL;
			}

			//KdPrint(("Device Name: %wZ: %d.\n", &(((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->DeviceName), 
			//	(((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->DeviceName).Length));
			//KdPrint(("FileObject: %wZ: %d.\n", &FileObject->FileName, FileObject->FileName.Length ));
			//wsprintf( bmf, L"%wZ%wZ", (((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->DeviceName).Buffer, FileObject->FileName.Buffer );
			RtlZeroMemory( bmf, MaxLength + sizeof(WCHAR) );
			RtlCopyMemory( (char*)bmf, 
				(char*)(((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->UserNames).Buffer, 
				(((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->UserNames).Length );

			RtlCopyMemory( (char*)bmf + (((PGINGKO_DEVICE_EXTENSION)pFirst->DeviceExtension)->UserNames).Length,
				(char*)FileObject->FileName.Buffer, 
				FileObject->FileName.Length );

			//pUnicode = ExAllocatePoolWithTag( NonPagedPool, sizeof(UNICODE_STRING), GINGKO_POOL_TAG );
			if( pUnicode != NULL )
			{
				//UNICODE_STRING TempUnicode;
				//RtlInitUnicodeString( pUnicode, bmf );
				RtlCreateUnicodeString( pUnicode, bmf );
				//RtlCopyUnicodeString( pUnicode, &TempUnicode );
				//KdPrint(("Combined: %wZ\n", pUnicode ));
				//::RtlFreeUnicodeString( &TempUnicode );
			}
			
			ExFreePoolWithTag( bmf, GINGKO_POOL_TAG );

			return NULL;
			
		}
		pFirst =  pFirst->AttachedDevice;
	}

	return NULL;
}

/****
 ** Find The FileObject is under the procecess, so that this will
 ****/
#pragma optimize( "", off )
BOOLEAN FindWriteFileObject( PFILE_OBJECT FileObject, ULONG dwProcessId, PGINGKO_OBJECT *pRevObject, BOOLEAN Remove )
{
	PLIST_ENTRY pEntry = NULL;
	PFILE_OBJECT pFirstFileObject = NULL;
	ULONG processId = 0;
	
	while( pEntry = ExInterlockedRemoveHeadList( &gGingkoWriteFileListEntry, &gGingkoWriteFileSpinLock ) )
	{
		PGINGKO_OBJECT pObject = CONTAINING_RECORD(pEntry, GINGKO_OBJECT, ListEntry);
		if( pObject != NULL )
		{
			if( pFirstFileObject == NULL && processId == 0)
			{
				pFirstFileObject = pObject->FileObject;
				//processId = pObject->dwProcessId;
			}else{
				if( pFirstFileObject == pObject->FileObject ) // && processId == pObject->dwProcessId )
				{
					ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );
					return FALSE;
				}
			}

			if( pObject->FileObject == FileObject ) //&& dwProcessId == pObject->dwProcessId )
			{
				if( pRevObject != NULL )
				{
					*pRevObject = pObject;
				}

				if( !Remove )
				{
					ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );
				}else{
					if( pRevObject == NULL )
					{
						TCfree( pObject );
					}
				}
				return TRUE;
			}else
			{
				///Restore into again
				ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );
			}
		}
	}

	return FALSE;
}
#pragma optimize( "", on )

/****
 ** Fetch all of the GINGKO_OBJECT, and raise the event of requestKey
 ****/
#pragma optimize( "", off )
BOOLEAN RemoveAllFileObjectQueue()
{
	PLIST_ENTRY pEntry = NULL;
	PGINGKO_OBJECT pFirstObject = NULL;
	ULONG processId = 0;
	
	while( pEntry = ExInterlockedRemoveHeadList( &gGingkoWriteFileListEntry, &gGingkoWriteFileSpinLock ) )
	{
		PGINGKO_OBJECT pObject = CONTAINING_RECORD(pEntry, GINGKO_OBJECT, ListEntry);
		//PKTHREAD kThread = NULL;
		if( pObject != NULL )
		{
			if( pFirstObject == NULL )
			{
				pFirstObject = pObject;
				//processId = pObject->dwProcessId;
			}else if( pFirstObject == pObject ){
				return FALSE;
			}
			pObject->Queue.ThreadExitRequested = TRUE;
			
			//kThread = pObject->Queue.MainThread;
			//KeSetEvent( pObject->Queue.RequestKeyEvent, );
			pObject->Queue.ThreadExitRequested = TRUE;
			KeSetEvent( &(pObject->Queue.RequestKeyEvent), IO_NO_INCREMENT, FALSE );
			TCStopThreadBySemaphore( pObject->Queue.MainThread, &(pObject->Queue.MainThreadQueueNotEmptyEvent));
			//KeWaitForSingleObject( kThread, Executive, KernelMode, TRUE, NULL );
		}
	}

	return TRUE;
}
#pragma optimize( "", on )

#pragma optimize( "", off )
BOOLEAN FindGingkoObjectByFileName( WCHAR *pFileName, ULONG FileNameLength, ULONG dwProcessId, PGINGKO_OBJECT *pRevObject )
{
	PLIST_ENTRY pEntry = NULL;
	PFILE_OBJECT pFirstFileObject = NULL;
	ULONG processId = 0;
	
	while( pEntry = ExInterlockedRemoveHeadList( &gGingkoWriteFileListEntry, &gGingkoWriteFileSpinLock ) )
	{
		PGINGKO_OBJECT pObject = CONTAINING_RECORD(pEntry, GINGKO_OBJECT, ListEntry);
		if( pObject != NULL )
		{
			//UNICODE_STRING fname;
			//UNICODE_STRING gname;
			WCHAR *gkoName = NULL;
			WCHAR *fileName = NULL;
			
			if( pFirstFileObject == NULL && processId == 0)
			{
				pFirstFileObject = pObject->FileObject;
				//processId = pObject->dwProcessId;
			}else{
				if( pFirstFileObject == pObject->FileObject ) // && processId == pObject->dwProcessId )
				{
					ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );
					return FALSE;
				}
			}

			KdPrint( ( "Filename is %s.\n", pFileName ) );

			if( (FileNameLength + sizeof(WCHAR) * 2 ) == pObject->FileName.Length )
			{
				if( RtlCompareMemory( pFileName, pObject->FileName.Buffer + 2, FileNameLength ) == FileNameLength )
				{
					if( pRevObject != NULL )
					{
						*pRevObject = pObject;
					}
					return TRUE;
				}
			}
			ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );			
		}
	}

	return FALSE;
}
#pragma optimize( "", on )

#pragma optimize( "", off )
BOOLEAN HasGingkoIdentifier(
			PGINGKO_HEADER Header
		)
{
	PGINGKO_HEADER pHeader = Header;

	//KdPrint(("Checking Gingko Identifier.Length: %d. FileObject: %08x Name: %wZ\n", Length, FileObject, &FileObject->FileName));
	if( pHeader->SizeOfHeader == sizeof( GINGKO_HEADER ) )
	{
		if( pHeader->Identifier[0] == 'G' && 
			pHeader->Identifier[1] == 'K' &&
			pHeader->Identifier[2] == 'T' &&
			pHeader->Identifier[3] == 'F')
		{
			return TRUE;
		}
	}

	return FALSE;
}
#pragma optimize( "", on )
#pragma optimize( "", off )
BOOLEAN IsGingkoServerProcess()
{
	SharedNotificationPtr snf = GingkoReferenceSharedNotification();

	if( gGingkoServerStarted == TRUE && snf != NULL )
	{
		 
		if( PsGetCurrentProcessId() == snf->ServerProcess || PsGetCurrentProcessId() == snf->ClientProcess )
		{
			GingkoDereferenceSharedNotification(snf);
			return TRUE;
		}
	}
	GingkoDereferenceSharedNotification(snf);
	return FALSE;
}
#pragma optimize( "", on )


/**
 * The Process Creation Monitor that will be used to inject the hook.
 *
 */
VOID GingkoProcessCreationMonitor(
	IN HANDLE ParentId,
	IN HANDLE ProcessId,
	IN BOOLEAN Create
	)
{
	if( Create )
	{
		KdPrint(("%08x created a new proecess %08x\n", ParentId, ProcessId));
	}else{
		KdPrint(("%08x destroy a the process %08x\n", ParentId, ProcessId));
		RemoveProcessUnderDecryption( (ULONG)ProcessId );
	}
}

#pragma optimize( "", off )
BOOLEAN FindCryptoInfo( PGINGKO_HEADER Header, PCRYPTO_INFO *pRevCryptInfo )
{
	PLIST_ENTRY pEntry = NULL;
	PLIST_ENTRY pFirstObject = NULL;
	BOOLEAN Found = FALSE;
	
	while( pEntry = ExInterlockedRemoveHeadList( &gGingkoWriteFileListEntry, &gGingkoWriteFileSpinLock ) )
	{
		PGINGKO_OBJECT pObject = CONTAINING_RECORD(pEntry, GINGKO_OBJECT, ListEntry);
		if( pFirstObject == NULL )
		{
			pFirstObject = pEntry;
		}else if( pFirstObject == pEntry ) {
			return Found;
		}

		if( pObject != NULL )
		{
			if( RtlCompareMemory( pObject->GingkoHeader.Company, Header->Company, COMPANY_LENGTH ) == COMPANY_LENGTH 
				&& RtlCompareMemory( pObject->GingkoHeader.Md5, Header->Md5, MD5_DIGEST_LENGTH ) == MD5_DIGEST_LENGTH  )
			{
				if( pObject->Queue.CryptoInfo != NULL )
				{
					if( pRevCryptInfo != NULL )
					{
						*pRevCryptInfo = pObject->Queue.CryptoInfo;
					}
					Found = TRUE;
				}
			}
			ExInterlockedInsertTailList( &gGingkoWriteFileListEntry, &pObject->ListEntry, &gGingkoWriteFileSpinLock );
			if( Found == TRUE )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
#pragma optimize( "", on )


/**
 * Clear all cache when the file is opened as a GingkoObject. 
 * Which is will base on the system request.
 */
NTSTATUS GingkoFileFlushCache( PFILE_OBJECT FileObject )
{
	KeEnterCriticalRegion();

	if(ExAcquireResourceExclusiveLite( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Resource, FALSE))
	{
		//if(ExAcquireResourceExclusiveLite( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->PagingIoResource, FALSE))
		{

			if(FileObject->SectionObjectPointer)
			{
				 IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP );

				 CcFlushCache( FileObject->SectionObjectPointer, NULL, 0, NULL );

				 CcGetFileSizePointer( FileObject );

				 if(FileObject->SectionObjectPointer->ImageSectionObject)
				 {
						MmFlushImageSection(
								FileObject->SectionObjectPointer,
								MmFlushForWrite//MmFlushForDelete// 
							);
				 }                      
				 if(FileObject->SectionObjectPointer->DataSectionObject)
				 { 
						CcPurgeCacheSection( FileObject->SectionObjectPointer,
								 NULL,
								 0,
								 FALSE ); //为TRUE，会强迫所有的FileObject重新初始化Cache                                                         
				 }	         
				 if(FileObject->PrivateCacheMap)
				 {
						CcUninitializeCacheMap(FileObject, NULL, NULL);;  
				 }
	         

				 if(FileObject->SectionObjectPointer->DataSectionObject)
				 {
					//Interval.QuadPart = DELAY_ONE_MILLISECOND * 500;//500ms
					//KeDelayExecutionThread(KernelMode, FALSE, &Interval); 
		                                             
					MmForceSectionClosed( FileObject->SectionObjectPointer,
								 TRUE//改为TRUE,彻底刷新缓存!!!
								 ); 
				 }

				IoSetTopLevelIrp(NULL);                                    
			}
			//ExReleaseResourceLite( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->PagingIoResource );                     
		}                
		ExReleaseResourceLite( ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->Resource );                     
	}       
	KeLeaveCriticalRegion();

	return STATUS_SUCCESS;
}
