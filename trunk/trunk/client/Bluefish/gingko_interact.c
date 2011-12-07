#include "gingko_filter.h"
#include "gingko_interact.h"

PGINGKO_APC_PARAMETER gRequestBuffer = NULL;
LIST_ENTRY gGingkoPublicKeyRequestQueue;
KSPIN_LOCK gGingkoPublicKeyRequestLock;
KMUTEX	   gAcquireRequestKeyMutex; 	

LIST_ENTRY gGingkoPublicKeyResponseQueue;
KSPIN_LOCK gGingkoPublicKeyResponseLock;
KMUTEX	   gAcquireResponseQueryMutex; 	
KSPIN_LOCK gDecryptedProcessesLock;

DecryptedProcessesTable *gDecryptedProcessesTable = NULL;
KMUTEX				     gDecryptedProcessesMutex = {0};

extern SharedNotificationPtr gGingkoNotification;

int ReadVolumeHeader (BOOL bBoot, char *encryptedHeader, Password *password, PCRYPTO_INFO *retInfo, CRYPTO_INFO *retHeaderCryptoInfo);
int ReadPrimaryKey(	char *prmKeys, PCRYPTO_INFO *retInfo );
void DecrementOutstandingIoCount (EncryptedIoQueue *queue);

void ClearRequestBuffer()
{
	gRequestBuffer = NULL;
}

void SetRequestBuffer(PGINGKO_APC_PARAMETER req)
{
	if( gRequestBuffer == NULL )
	{
		gRequestBuffer = req;
	}
}

NTSTATUS GingkoRetrievRequestBuffer(PIRP Irp, PVOID InputBuffer, LONG InputLength,  PVOID OutputBuffer)
{
	__try{
		RtlCopyMemory( gRequestBuffer, OutputBuffer, sizeof(GINGKO_APC_PARAMETER) );
	}__except(EXCEPTION_EXECUTE_HANDLER){
		return STATUS_UNSUCCESSFUL;
	}
	return STATUS_SUCCESS;
}

NTSTATUS WriteDecryptBuffer(PIRP Irp, PVOID InputBuffer, LONG InputLength)
{
	PGINGKO_EVENT pGingkoEvent = (PGINGKO_EVENT) InputBuffer;

	if( pGingkoEvent->OutputLength > 0 )
	{
		RtlCopyMemory(  gRequestBuffer->OutputBuffer, &(pGingkoEvent->Buffer), pGingkoEvent->OutputLength );
	}

	gRequestBuffer->OutputLength = pGingkoEvent->OutputLength;
	gRequestBuffer->Status = pGingkoEvent->Status;
	
	KeSetEvent( (PRKEVENT)pGingkoEvent->Handler, IO_NO_INCREMENT, FALSE );
	return STATUS_SUCCESS;
}

NTSTATUS GingkoWaitForDecryptionEvent()
{
	SharedNotificationPtr snf = GingkoReferenceSharedNotification();


	if( snf != NULL && snf->WriteEvent )
	{
		NTSTATUS status = KeWaitForSingleObject( snf->WriteEvent, Executive, KernelMode, FALSE, NULL );
		GingkoDereferenceSharedNotification(snf);
		return status;
	}

	GingkoDereferenceSharedNotification(snf);
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS GingkoSetEventForRequest()
{
	SharedNotificationPtr snf = GingkoReferenceSharedNotification();

	if( snf != NULL )
	{
		if(  snf->ReadEvent != NULL )
		{
			NTSTATUS status = KeSetEvent( snf->ReadEvent, IO_NO_INCREMENT, FALSE );
			GingkoDereferenceSharedNotification(snf);
			return status;
		}
	}
	GingkoDereferenceSharedNotification(snf);
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS GingkoRetrievPublicKeyRequest(PIRP Irp, PVOID InputBuffer, LONG InputLength,  PVOID OutputBuffer, ULONG OutputLength)
{
	PLIST_ENTRY pListEntry = NULL;
	
	//KeWaitForSingleObject( &gAcquireRequestKeyEvent, Executive, KernelMode, FALSE, NULL );
	pListEntry = ExInterlockedRemoveHeadList( &gGingkoPublicKeyRequestQueue, &gGingkoPublicKeyRequestLock );
	//KeSetEvent( &gAcquireRequestKeyEvent, IO_NO_INCREMENT, FALSE );

	while( pListEntry != NULL )
	{
		EncryptedIoQueue* request = CONTAINING_RECORD (pListEntry, EncryptedIoQueue, RequestKeyList);
		if( request != NULL )
		{
			RequestKeyResponseQueue* responseQueue = (RequestKeyResponseQueue*) TCalloc(sizeof(RequestKeyResponseQueue));
			
			if( responseQueue == NULL )
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			__try{

				ULONG dwFilenameLength = 0L;
				PrivateKeyRequest* pkItem = (PrivateKeyRequest*) OutputBuffer;//TCalloc(sizeof(PrivateKeyRequestItem));
				pkItem->FileObject = request->FileObject;
				pkItem->RequestKeyEvent = &(request->RequestKeyEvent);
				pkItem->dwProcessId = request->GingkoObject->dwProcessId;
				//pkItem->Identifier[GINGKO_IDENTIFIER_LENGTH];		///the file format identfied,, should be GKTF
				//pkItem->Version[GINGKO_VERSION_LENGTH];				///first char is the major version, second char is the minor version. the version struct is major.minor
				//pkItem->SizeOfHeader = request->GingkoHeader->SizeOfHeader;								///the size of the file header.
				//pkItem->SizeOfFile = request->GingkoHeader->SizeOfFile;									///the file size of file
				RtlCopyMemory( pkItem->Company, request->GingkoHeader->Company, COMPANY_LENGTH);
				RtlCopyMemory( pkItem->Md5, request->GingkoHeader->Md5, MD5_DIGEST_LENGTH);
				RtlCopyMemory( pkItem->Method, request->GingkoHeader->Method, GINGKO_METHOD_LENGTH);
				RtlCopyMemory( pkItem->Identifier, request->GingkoHeader->Identifier, GINGKO_IDENTIFIER_LENGTH);
				RtlCopyMemory( pkItem->Version, request->GingkoHeader->Version, GINGKO_VERSION_LENGTH);
				RtlCopyMemory( pkItem->FileReserved, request->GingkoHeader->ReservedBytes, FILE_RESERVED_BYTES);
				pkItem->SizeOfBlock = request->GingkoHeader->SizeOfBlock;
				pkItem->SizeOfHeader = request->GingkoHeader->SizeOfHeader;
				pkItem->SizeOfFile = request->GingkoHeader->SizeOfFile;
				
				///one block of size should to take to decrypt. the size should be depend to the method of crypto
				//RtlCopyMemory( &(pkItem->HeaderOfFile), request->GingkoHeader, sizeof(GINGKO_HEADER));
				dwFilenameLength = OutputLength - sizeof(PrivateKeyRequest);
				
				//pkItem->FilenameLength = request->GingkoObject->FileName.Length;
				if( dwFilenameLength > request->GingkoObject->FileName.Length )
				{
					pkItem->FilenameLength = request->GingkoObject->FileName.Length;
					RtlCopyMemory( &(pkItem->Filename[0]), 
						request->GingkoObject->FileName.Buffer, 
						request->GingkoObject->FileName.Length );
				}else{
					pkItem->FilenameLength = request->GingkoObject->FileName.Length;
					pkItem->Filename[0] = L'\0';
				}

				//InterlockedIncrement( &(request->RequestKeyCount) );
				//Add into the Response Queue, this queue will be used to check the response exists 
				responseQueue->FileObject = (LONG_PTR) request->FileObject;
				responseQueue->RequestKeyEvent =(LONG_PTR) &(request->RequestKeyEvent);
				responseQueue->dwProcessId = request->GingkoObject->dwProcessId;
				responseQueue->Queue = (LONG_PTR) request;
				
				ExInterlockedInsertTailList(&gGingkoPublicKeyResponseQueue, &(responseQueue->ListEntry), &gGingkoPublicKeyResponseLock);
				Irp->IoStatus.Status		=	STATUS_SUCCESS;
				Irp->IoStatus.Information	=	sizeof(PrivateKeyRequest);
			}__except(EXCEPTION_EXECUTE_HANDLER){
				//TCfree(request);
				///Add to this list again.
				ExInterlockedInsertTailList(&gGingkoPublicKeyRequestQueue, &(request->RequestKeyList), &gGingkoPublicKeyRequestLock);
				return STATUS_UNSUCCESSFUL;
			}
		}
		return STATUS_SUCCESS;
	}
	
	Irp->IoStatus.Status = STATUS_NO_SUCH_FILE;
	Irp->IoStatus.Information = 0L;
	return STATUS_NO_SUCH_FILE;
}


BOOL FindEncryptionIoQueueFromResponseKeyList(LONG_PTR FileObject, LONG_PTR RequestKeyEvent, RequestKeyResponseQueue** pQueue)
{
	PLIST_ENTRY Entry = NULL;
	RequestKeyResponseQueue* First = NULL;
	NTSTATUS Status = 0L;
	Status = KeWaitForSingleObject( &gAcquireResponseQueryMutex, Executive, KernelMode, FALSE, NULL );
	while( (Entry = ExInterlockedRemoveHeadList( &gGingkoPublicKeyResponseQueue, &gGingkoPublicKeyResponseLock )) )
	{
		RequestKeyResponseQueue* queue = CONTAINING_RECORD (Entry, RequestKeyResponseQueue, ListEntry);

		if( queue != NULL )
		{
			if( First == NULL )
			{
				First = queue;
			}else if( First == queue ){
				ExInterlockedInsertTailList(&gGingkoPublicKeyResponseQueue, &(queue->ListEntry), &gGingkoPublicKeyResponseLock);
				break;
			}

			if( (queue->FileObject == FileObject) && (queue->RequestKeyEvent == RequestKeyEvent ) )
			{
				if( pQueue != NULL )
				{
					*pQueue = queue;
				}else
				{
					TCfree( queue );
				}
				KeReleaseMutex( &gAcquireResponseQueryMutex, FALSE);
				return TRUE;
			}

			ExInterlockedInsertTailList(&gGingkoPublicKeyResponseQueue, &(queue->ListEntry), &gGingkoPublicKeyResponseLock);
		}
	}

	KeReleaseMutex( &gAcquireResponseQueryMutex, FALSE);
	return FALSE;

}


NTSTATUS GingkoPublicKeyResponse(PIRP Irp, PVOID InputBuffer, LONG InputLength,  PVOID OutputBuffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	KIRQL irql = 0;
	if( InputLength == sizeof(PrivateKeyResponse) )
	{
		PrivateKeyResponse* response = (PrivateKeyResponse*) InputBuffer;
		if( response != NULL )
		{
			RequestKeyResponseQueue *queue = NULL;

			if( response->SizeOfResponse != sizeof( PrivateKeyResponse ) )
			{
				return STATUS_UNSUCCESSFUL;
			}

			if( FindEncryptionIoQueueFromResponseKeyList( response->FileObject, response->RequestKeyEvent, &queue ) )
			{
				EncryptedIoQueue* eq = (EncryptedIoQueue*) queue->Queue;
				if( eq->StopPending == FALSE && response->Permission & 0x80000000L )  //Was Set the certernly permsission
				{
					InterlockedIncrement (&(eq->OutstandingIoCount));
					//ReadVolumeHeader( FALSE, response->CryptoKey, &passwd, &(eq->CryptoInfo), NULL);
					ReadPrimaryKey( response->CryptoKey, &(eq->CryptoInfo) );
					UpdateProcessPermission( response->ProcessId, response->Permission, response->Md5, response->CryptoKey, &irql );
					if( eq->CryptoInfo != NULL )
					{
						__try{
							RtlCopyMemory( eq->GingkoHeader->Identifier, response->Identifier,		GINGKO_IDENTIFIER_LENGTH );
							RtlCopyMemory( eq->GingkoHeader->Version, response->Version,			GINGKO_VERSION_LENGTH );
							RtlCopyMemory( eq->GingkoHeader->Company, response->Company,			COMPANY_LENGTH );
							RtlCopyMemory( eq->GingkoHeader->Md5, response->Md5,					MD5_DIGEST_LENGTH );
							RtlCopyMemory( eq->GingkoHeader->Method, response->Method,				GINGKO_METHOD_LENGTH );
							eq->GingkoHeader->SizeOfHeader = response->SizeOfHeader;
							eq->GingkoHeader->SizeOfFile = response->SizeOfFile;
							eq->GingkoHeader->SizeOfBlock = response->SizeOfBlock;
							eq->GingkoHeader->Reserved = response->Reserved;
						}__except(EXCEPTION_EXECUTE_HANDLER){
							Status = STATUS_UNSUCCESSFUL;
						}
					}
				}
				DecrementOutstandingIoCount(eq);
				KeSetEvent( &(eq->RequestKeyEvent), IO_NO_INCREMENT, FALSE );
				TCfree( queue );
				Irp->IoStatus.Status		=	Status;
				Irp->IoStatus.Information	=	0;
			}
		}
	}
	return STATUS_SUCCESS;
}


NTSTATUS GingkoInitializePublicKeyRequestQueue()
{
	InitializeListHead(&gGingkoPublicKeyRequestQueue );
	KeInitializeSpinLock(&gGingkoPublicKeyRequestLock);
	InitializeListHead(&gGingkoPublicKeyResponseQueue);
	KeInitializeSpinLock(&gGingkoPublicKeyResponseLock);
	KeInitializeMutex( &gAcquireResponseQueryMutex, 0);
	KeInitializeMutex( &gAcquireRequestKeyMutex, 0);
	KeInitializeSpinLock( &gGingkoWriteFileSpinLock );
	InitializeListHead( &gGingkoWriteFileListEntry );
	
	return STATUS_SUCCESS;
}
//
//NTSTATUS GingkoAddPublicKeyRequest( 
//	ULONG		ProcessId,				///ProcessId;
//	UCHAR*		Company,				///The Company Id
//	UCHAR*		Md5				///md5 length
//)
//{
//	PrivateKeyRequest* request = TCalloc(sizeof(PrivateKeyRequest));
//	if( request == NULL )
//	{
//		return STATUS_UNSUCCESSFUL;
//	}
//
//	return STATUS_SUCCESS;
//}

NTSTATUS GingkoSetEncryptionProcess(PIRP Irp, LONG_PTR ProcessId, BOOLEAN Action)
{
	KIRQL irql;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information	= 0;

	if( Action )
	{
		UpdateProcessPermission( (ULONG)ProcessId, 0x800FFFFF, NULL, NULL, &irql ); ///Default will 
	}else{
		RemoveProcessUnderDecryption( (ULONG)ProcessId );
	}

	return STATUS_SUCCESS;
}

static NTSTATUS FreeCryptoBlock( FileCryptoBlock *pBlock )
{
	FileCryptoBlock *pNextBlock = pBlock->Next;
	while( pNextBlock != NULL )
	{
		FileCryptoBlock *pThisBlock = pNextBlock;
		pNextBlock = pThisBlock->Next;
		ExFreePoolWithTag(pThisBlock, GINGKO_POOL_TAG);
	}
	ExFreePoolWithTag(pBlock, GINGKO_POOL_TAG);
	return STATUS_SUCCESS;
}


NTSTATUS GingkoInitializeDecryptProcessTable()
{
	if( gDecryptedProcessesTable != NULL )
	{
		GingkoUnInitializeDecryptProcessTable();
		gDecryptedProcessesTable = NULL;
	}

	gDecryptedProcessesTable = (DecryptedProcessesTable*) TCalloc( sizeof(DecryptedProcessesTable) );
	RtlZeroMemory( gDecryptedProcessesTable, sizeof(DecryptedProcessesTable));
	KeInitializeMutex( &gDecryptedProcessesMutex, 0 );
	KeInitializeSpinLock( &gDecryptedProcessesLock );

	return STATUS_SUCCESS;
}

NTSTATUS GingkoUnInitializeDecryptProcessTable()
{
	KIRQL irql;

	KeAcquireSpinLock( &gDecryptedProcessesLock, &irql );
	//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
	//TCSleep( 100 );

	if( gDecryptedProcessesTable != NULL )
	{
		DecryptedProcessesTable *FreeTable = gDecryptedProcessesTable;
		gDecryptedProcessesTable = NULL;
		do{
			DecryptedProcessesTable *FreeTableNext = FreeTable->Next;
			FreeCryptoBlock( FreeTableNext->FileCryptoBlock );
			TCfree( FreeTable );
			FreeTable = FreeTableNext;
		}while( FreeTable != NULL );
		gDecryptedProcessesTable = NULL;
	}
	
	KeReleaseSpinLock( &gDecryptedProcessesLock, irql );

	return STATUS_SUCCESS;
}

//Check the ProcessId whether is under in decryption process.
ULONG GetPermissionUnderDecryptionByProcessId( ULONG ProcessId, KIRQL *irql )
{
	int i = 0;
	BOOL bWaitable = FALSE;
	ULONG uret = 0x00000000;
	DecryptedProcessesTable* pTable = NULL;
	
	if( irql != NULL )
	{
		KeAcquireSpinLock( &gDecryptedProcessesLock, irql );
	}

	pTable = gDecryptedProcessesTable;

	while( pTable != NULL )
	{
		if( pTable->ProcessesId[0] == ProcessId )
		{
			uret = pTable->ProcessesId[1];
			break;
		}
		pTable = pTable->Next;
	}

	if( irql != NULL )
	{
		KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
	}
	return uret;  
}



// Remove the ProcessId
NTSTATUS RemoveProcessUnderDecryption( ULONG ProcessId )
{
	int i = 0;
	KIRQL irql;
	DecryptedProcessesTable* pTable = NULL;
	DecryptedProcessesTable* pPrevTable = NULL;

	
	KeAcquireSpinLock( &gDecryptedProcessesLock, &irql );

	//KeWaitForSingleObject( &gDecryptedProcessesMutex, Executive, KernelMode, FALSE, NULL );
	pTable = gDecryptedProcessesTable;
	pPrevTable = pTable;
	
	while( pTable != NULL )
	{
		if( pTable->ProcessesId[0] == ProcessId )
		{
			pTable->ProcessesId[0] = 0L;
			pTable->ProcessesId[1] = 0L;
			FreeCryptoBlock( pTable->FileCryptoBlock );
			pPrevTable->Next = pTable->Next;
			TCfree( pTable );
			break;
		}

		pPrevTable = pTable;
		pTable = pTable->Next;
	}

	KeReleaseSpinLock( &gDecryptedProcessesLock, irql );

	return STATUS_SUCCESS;
}

static void UpdateFileCryptoBlock( FileCryptoBlock **Block, char* fileHash, char* cryptoKeys )
{
	if( Block == NULL )
	{
		return;
	}

	if( *Block != NULL )
	{
		FileCryptoBlock *pCurr = *Block;
		FileCryptoBlock *pBefore = NULL;
		while( pCurr != NULL )
		{
			if( RtlCompareMemory( pCurr->FileHash, fileHash, MD5_DIGEST_LENGTH ) == MD5_DIGEST_LENGTH )
			{
				RtlCopyMemory( pCurr->CryptoInfo, cryptoKeys, MAXSIZE_PRIVATE_KEY );
				return;
			}
			pBefore = pCurr;
			pCurr = pCurr->Next;
		}

		if( pBefore != NULL )
		{
			pCurr = ExAllocatePoolWithTag( NonPagedPool, sizeof(FileCryptoBlock), GINGKO_POOL_TAG );
			if( pCurr != NULL )
			{
				RtlCopyMemory( pCurr->FileHash, fileHash, MD5_DIGEST_LENGTH );
				RtlCopyMemory( pCurr->CryptoInfo, cryptoKeys, MAXSIZE_PRIVATE_KEY );
				pCurr->Next = NULL;
				pBefore->Next = pCurr;
			}
		}

	}else{
		*Block = ExAllocatePoolWithTag( NonPagedPool, sizeof(FileCryptoBlock), GINGKO_POOL_TAG );
		if( *Block != NULL )
		{
			RtlCopyMemory( (*Block)->FileHash, fileHash, MD5_DIGEST_LENGTH );
			RtlCopyMemory( (*Block)->CryptoInfo, cryptoKeys, MAXSIZE_PRIVATE_KEY );
			(*Block)->Next = NULL;
		}
	}
}


//
// Update Process's permission
//
ULONG UpdateProcessPermission(  ULONG ProcessId, ULONG Permission, char* fileHash, char* cryptoKeys, KIRQL *irql )
{
	DecryptedProcessesTable* pTable = NULL;
	int i = 0;
	int FindEmptyId = -1;
	ULONG OldPermission = 0L;
	BOOL bWaitable = FALSE;
	DecryptedProcessesTable* pHasSpaceTable = NULL;

	//if (KeGetCurrentIrql() < DISPATCH_LEVEL) {	
	//	KeWaitForSingleObject( &gDecryptedProcessesMutex, Executive, KernelMode, FALSE, NULL );
	//	bWaitable = TRUE;
	//}
	if( irql != NULL )
	{
		KeAcquireSpinLock( &gDecryptedProcessesLock, irql );
	}

	pTable = gDecryptedProcessesTable;
	
	while( pTable != NULL )
	{
		if( pTable->ProcessesId[0] == ProcessId )
		{
			OldPermission = pTable->ProcessesId[1];
			pTable->ProcessesId[1] |= Permission;
			if( fileHash != NULL && cryptoKeys != NULL )
			{
				UpdateFileCryptoBlock( &(pTable->FileCryptoBlock), fileHash, cryptoKeys );
			}
			//if( bWaitable )
			//{
				//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
			//}
			//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
			if( irql != NULL )
			{
				KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
			}
			return OldPermission;
		}
		pTable = pTable->Next;
	}

	///i can not find the process, so I will add it as a new entry.
	pTable = gDecryptedProcessesTable;
	pHasSpaceTable = TCalloc(sizeof(DecryptedProcessesTable));
	if( pHasSpaceTable != NULL )
	{
		pHasSpaceTable->ProcessesId[0] = ProcessId;
		pHasSpaceTable->ProcessesId[1] = Permission;
		pHasSpaceTable->FileCryptoBlock = NULL;

		if( fileHash != NULL && cryptoKeys != NULL )
		{
			UpdateFileCryptoBlock( &(pHasSpaceTable->FileCryptoBlock), fileHash, cryptoKeys );
		}

		if( pTable != NULL )
		{
			pHasSpaceTable->Next = pTable->Next;
			pTable->Next = pHasSpaceTable;
		}else{
			gDecryptedProcessesTable = TCalloc(sizeof(DecryptedProcessesTable));
			if( gDecryptedProcessesTable == NULL )
			{
				TCfree( pHasSpaceTable );
				if( irql != NULL )
				{
					KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
				}
				return STATUS_UNSUCCESSFUL;
			}
			pHasSpaceTable->Next = NULL;
			gDecryptedProcessesTable->ProcessesId[0] = 0L;
			gDecryptedProcessesTable->ProcessesId[1] = 0L;
			gDecryptedProcessesTable->Next = pHasSpaceTable;
			gDecryptedProcessesTable->FileCryptoBlock = NULL;
		}
	}
	if( irql != NULL )
	{
		KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
	}
	//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
	return OldPermission;
}


//
// Update Process's permission
//
BOOL FindCryptoPermission(  ULONG ProcessId, char* fileHash, ULONG *Permission, unsigned char* CryptoKeysBuf, KIRQL *irql )
{
	DecryptedProcessesTable* pTable = NULL;
	int i = 0;
	int FindEmptyId = -1;
	BOOL bWaitable = FALSE;
	ULONG OldPermission = 0L;
	DecryptedProcessesTable* pHasSpaceTable = NULL;
	//if (KeGetCurrentIrql() < DISPATCH_LEVEL) {	
	//	KeWaitForSingleObject( &gDecryptedProcessesMutex, Executive, KernelMode, FALSE, NULL );
	//	bWaitable = TRUE;
	//}
	if( irql != NULL )
	{
		KeAcquireSpinLock( &gDecryptedProcessesLock, irql );
	}
	//KeWaitForSingleObject( &gDecryptedProcessesMutex, Executive, KernelMode, FALSE, NULL );
	pTable = gDecryptedProcessesTable;
	
	while( pTable != NULL )
	{
		if( pTable->ProcessesId[0] == ProcessId )
		{
			if( Permission != NULL )
			{
				*Permission = pTable->ProcessesId[1];
				if( fileHash == NULL || CryptoKeysBuf == NULL )
				{
					//if( bWaitable )
					//{
					//	KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
					//}
					if( irql != NULL )
					{
						KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
					}
					return TRUE;
				}
			}

			if( fileHash != NULL && CryptoKeysBuf != NULL )
			{
				FileCryptoBlock *pCurr = pTable->FileCryptoBlock;
				while( pCurr != NULL )
				{
					if( RtlCompareMemory( pCurr->FileHash, fileHash, MD5_DIGEST_LENGTH ) == MD5_DIGEST_LENGTH )
					{
						RtlCopyMemory( CryptoKeysBuf, pCurr->CryptoInfo, MAXSIZE_PRIVATE_KEY );
						//if( bWaitable )
						//{
						//	KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
						//}
						if( irql != NULL )
						{
							KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
						}
						//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
						return TRUE;
					}
					pCurr = pCurr->Next;
				}
			}
		}
		pTable = pTable->Next;
	}
	//if( bWaitable )
	//{
	//	KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
	//}
	if( irql != NULL )
	{
		KeReleaseSpinLock( &gDecryptedProcessesLock, *irql );
	}
	//KeReleaseMutex( &gDecryptedProcessesMutex, FALSE );
	return FALSE;
}
