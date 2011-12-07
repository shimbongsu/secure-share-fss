/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/
#include "gingko_filter.h"
#include "gingko_lib.h"
#include "gingko_ioqueue.h"
#include "gingko_interact.h"

#define __INTERNAL_ASYNC_IO__

IO_COMPLETION_ROUTINE GingkoCompletedReadWriteRoutine;
IO_COMPLETION_ROUTINE GingkoAsynchronousRequestCompletion;
IO_COMPLETION_ROUTINE IoCompletionRoutine;
IO_COMPLETION_ROUTINE IoCleanupCompletionRoutine;
KSTART_ROUTINE		  MainThreadProc;


NTSTATUS
IoCleanupCompletionRoutine(
	IN PDEVICE_OBJECT  DeviceObject,
	IN PIRP  Irp,
	IN PVOID  Context
	)
{
	//DbgPrint(("IoCompletionRoutine!\n"));
	*Irp->UserIosb = Irp->IoStatus;

	if (Irp->UserEvent)
		KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);

	if (Irp->MdlAddress)
	{
		IoFreeMdl(Irp->MdlAddress);
		Irp->MdlAddress = NULL;
	}

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

LONGLONG ResetFileObjectOffset( PFILE_OBJECT FileObject, LONGLONG CurrentByteOffset )
{
	#if 0
		return InterlockedExchange64( &(FileObject->CurrentByteOffset.QuadPart), CurrentByteOffset );
	#else
		LONGLONG OldValue = FileObject->CurrentByteOffset.QuadPart;
		FileObject->CurrentByteOffset.QuadPart = CurrentByteOffset;
		return OldValue;
	#endif
}

NTSTATUS DirectIrpCleanup(
	IN PFILE_OBJECT FileObject,
	IN PDEVICE_OBJECT Device,
	IN PETHREAD CurrentThread,
	OUT PIO_STATUS_BLOCK IoStatusBlock
	)
{
	NTSTATUS status;
	KEVENT Event;
	PIRP irp;
	PIO_STACK_LOCATION irpSp;
	PDEVICE_OBJECT deviceObject = Device;


	if (FileObject->Vpb == 0 || FileObject->Vpb->RealDevice == NULL)
		return STATUS_UNSUCCESSFUL;

	if( deviceObject == NULL )
	{
		deviceObject = FileObject->Vpb->DeviceObject;
	}

	irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

	if (irp == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

//	irp->Flags = IRP_READ_OPERATION;
	irp->RequestorMode = KernelMode;
	irp->UserIosb = IoStatusBlock;
	irp->UserEvent = &Event;
	irp->Tail.Overlay.Thread = CurrentThread;
	irp->Tail.Overlay.OriginalFileObject = FileObject;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_CLEANUP;
	irpSp->MinorFunction = 0;
	irpSp->DeviceObject = deviceObject;
	irpSp->FileObject = FileObject;

	KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
	IoSetCompletionRoutine(irp, IoCleanupCompletionRoutine, NULL, TRUE, TRUE, TRUE);
	status = IoCallDriver(deviceObject, irp);

	if (status == STATUS_PENDING)
	{
		status = KeWaitForSingleObject(&Event, Executive, KernelMode, TRUE, NULL);
	}

	return status;
}

NTSTATUS TCCompleteIrp (PIRP irp, NTSTATUS status, ULONG_PTR information)
{
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = information;
	//IoCompleteRequest (irp, NT_SUCCESS (status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT);
	IoCompleteRequest (irp, IO_NO_INCREMENT);
	return status;
}


NTSTATUS TCCompleteDiskIrp (PIRP irp, NTSTATUS status, ULONG_PTR information)
{
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = information;
	IoCompleteRequest (irp, NT_SUCCESS (status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT);
	//IoCompleteRequest (irp, IO_NO_INCREMENT);
	return status;
}


void DecrementOutstandingIoCount (EncryptedIoQueue *queue)
{
	if (InterlockedDecrement (&queue->OutstandingIoCount) == 0 && (queue->SuspendPending || queue->StopPending))
		KeSetEvent (&queue->NoOutstandingIoEvent, IO_DISK_INCREMENT, FALSE);
}


void OnItemCompleted (EncryptedIoQueueItem *item, BOOL freeItem)
{
	DecrementOutstandingIoCount (item->Queue);

	if(freeItem)
	{
		//if (item->Queue->IsFilterDevice)
		//	IoReleaseRemoveLock (&item->Queue->RemoveLock, item->OriginalIrp);
	}

	if (NT_SUCCESS (item->Status))
	{
		if (item->Write)
			item->Queue->TotalBytesWritten += item->OriginalLength;
		else
			item->Queue->TotalBytesRead += item->OriginalLength;
	}

	if (freeItem) 
		TCfree (item);
}


NTSTATUS CompleteOriginalIrp (EncryptedIoQueueItem *item, NTSTATUS status, ULONG_PTR information)
{
	//Dump ("Queue comp  offset=%I64d  status=%x  info=%p  out=%d\n", item->OriginalOffset, status, information, item->Queue->OutstandingIoCount - 1);
	if( item->ShouldReleaseMdl )
	{
		PIRP Irp = item->OriginalIrp;
		if( Irp->MdlAddress )
		{
			MmUnlockPages( Irp->MdlAddress );
			IoFreeMdl( Irp->MdlAddress );
			Irp->MdlAddress = NULL;
		}
	}

	TCCompleteDiskIrp (item->OriginalIrp, status, information);

	item->Status = status;
	
	OnItemCompleted (item, TRUE);

	return status;
}


 void AcquireFragmentBuffer (EncryptedIoQueue *queue, byte *buffer)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	if (buffer == queue->FragmentBufferA)
	{
		status = KeWaitForSingleObject (&queue->FragmentBufferAFreeEvent, Executive, KernelMode, FALSE, NULL);
	}
	//else if (buffer == queue->FragmentBufferB)
	//{
	//	KdPrint(("WaitForSingleObject IRP AcquireFrameBufferB\n"));
	//	status = KeWaitForSingleObject (&queue->FragmentBufferBFreeEvent, Executive, KernelMode, FALSE, NULL);
	//	KdPrint(("After WaitForSingleObject IRP AcquireFrameBufferB\n"));
	//}

	//if (!NT_SUCCESS (status))
	//	TC_BUG_CHECK (status);
}


 void ReleaseFragmentBuffer (EncryptedIoQueue *queue, byte *buffer)
{
	if (buffer == queue->FragmentBufferA)
	{
		KeSetEvent (&queue->FragmentBufferAFreeEvent, IO_DISK_INCREMENT, FALSE);
	}
	//else if (buffer == queue->FragmentBufferB)
	//{
	//	KeSetEvent (&queue->FragmentBufferBFreeEvent, IO_DISK_INCREMENT, FALSE);
	//}
	else
	{
		//TC_BUG_CHECK (STATUS_INVALID_PARAMETER);
	}
}


static NTSTATUS ProcessWrite (EncryptedIoQueue *queue, byte* Buffer, __int64 _offset, ULONG length, ULONG IrpFlagsx, BOOLEAN Final)
{
	UINT64_STRUCT dataUnit;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG dataRemaining = length;
	LARGE_INTEGER FileOffset;
	IO_STATUS_BLOCK IoStatusBlock;
	ULONG CurrentOffset = 0L;
	ULONG GingkoHeaderOffset = sizeof(GINGKO_HEADER);
	ULONG CurrentLength;
	__int64 actualOffset;
	byte *TempReadBuffer;
	LARGE_INTEGER FileObjectOffset;
	ULONG IrpFlags;

	if( dataRemaining <= 0 && queue->CurrentWriteLength > 0 )
	{
		ULONG dataParts = ((queue->CurrentWriteLength / ENCRYPTION_DATA_UNIT_SIZE) + ((queue->CurrentWriteLength % ENCRYPTION_DATA_UNIT_SIZE) ? 1 : 0));
		///Just write the Fragement Buffer to disk 
		FileOffset.QuadPart = queue->CurrentWriteOffset + GingkoHeaderOffset;
		dataUnit.Value = 0; //  queue->CurrentWriteOffset / ENCRYPTION_DATA_UNIT_SIZE;
		
		EncryptDataUnits( queue->FragmentBufferA, &dataUnit, 
							dataParts, queue->CryptoInfo);

		FileObjectOffset = queue->FileObject->CurrentByteOffset;
		queue->FileObject->CurrentByteOffset = FileOffset;
		IrpFlags = queue->GingkoObject->OffsetBytes.QuadPart >=  (FileOffset.QuadPart+(dataParts*ENCRYPTION_DATA_UNIT_SIZE)) ? 
					IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE: IRP_WRITE_OPERATION | IRP_NOCACHE;

		status = TCWriteDeviceEx( queue->LowerDeviceObject, queue->FileObject, 
							queue->FragmentBufferA, &FileOffset, 
							(dataParts * ENCRYPTION_DATA_UNIT_SIZE) , &IoStatusBlock, 
							IrpFlags );

		queue->FileObject->CurrentByteOffset = FileObjectOffset;
		if( !NT_SUCCESS( status ) )
		{
			return status;
		}

		queue->CurrentWriteLength = 0;

		queue->GingkoHeader->SizeOfFile = _offset;

		FileOffset.QuadPart = 0;

		FileObjectOffset = queue->FileObject->CurrentByteOffset;
		queue->FileObject->CurrentByteOffset = FileOffset;
		
		status = TCWriteDeviceEx( queue->LowerDeviceObject, queue->FileObject, 
							queue->GingkoHeader, &FileOffset, 
							sizeof(GINGKO_HEADER) , &IoStatusBlock, 
							IrpFlags );
		queue->FileObject->CurrentByteOffset = FileObjectOffset;
		if( !NT_SUCCESS( status ) )
		{
			return status;
		}
		status = STATUS_END_OF_FILE;
	}else if(dataRemaining <= 0 && queue->CurrentWriteLength == 0){
		status = STATUS_END_OF_FILE;
	}

	actualOffset = _offset;
	while( dataRemaining > 0 )
	{
		
		if( actualOffset > (queue->CurrentWriteOffset + TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE) || actualOffset < queue->CurrentWriteOffset )
		{
			///The write buffer is not in the buffer range, I will persist the buffer into harddisk.
			FileOffset.QuadPart = queue->CurrentWriteOffset + GingkoHeaderOffset;
			dataUnit.Value = 0; //  queue->CurrentWriteOffset / ENCRYPTION_DATA_UNIT_SIZE;
			
			EncryptDataUnits( queue->FragmentBufferA, &dataUnit, TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE / ENCRYPTION_DATA_UNIT_SIZE, queue->CryptoInfo);

			FileObjectOffset = queue->FileObject->CurrentByteOffset;
			queue->FileObject->CurrentByteOffset = FileOffset;
	
			IrpFlags = queue->GingkoObject->OffsetBytes.QuadPart >=  (FileOffset.QuadPart+TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE) ? 
					IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE: IRP_WRITE_OPERATION | IRP_NOCACHE;

			KdPrint(("Write to disk if the request offset is out of the space.\n"));
			status = TCWriteDeviceEx( queue->LowerDeviceObject, queue->FileObject, 
										queue->FragmentBufferA, 
										&FileOffset,
										TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE, 
										&IoStatusBlock, 
										IrpFlags );

			queue->FileObject->CurrentByteOffset = FileObjectOffset;
			queue->GingkoObject->OffsetBytes.QuadPart = MAX( queue->GingkoObject->OffsetBytes.QuadPart, queue->FileObject->CurrentByteOffset.QuadPart );
			///Clear the Buffer
			if( !NT_SUCCESS( status ) )
			{
				return status;
			}
			if( NT_SUCCESS( status ) )
			{
				queue->FileObject->CurrentByteOffset.QuadPart -=  GingkoHeaderOffset;
				RtlZeroMemory( queue->FragmentBufferA, TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE );
				queue->CurrentWriteOffset += queue->CurrentWriteLength;
				queue->CurrentWriteLength = 0;
			}

			///Reset the WriteOffset
			TempReadBuffer = TCalloc( TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE );
			
			if( TempReadBuffer == NULL )
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			queue->CurrentWriteOffset = (actualOffset / TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE) * TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE;
			FileOffset.QuadPart = queue->CurrentWriteOffset  + GingkoHeaderOffset;

			FileObjectOffset = queue->FileObject->CurrentByteOffset;
			queue->FileObject->CurrentByteOffset = FileOffset;
			status = TCReadDeviceEx( queue->LowerDeviceObject, queue->FileObject, 
										TempReadBuffer, 
										&FileOffset,
										TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE, 
										&IoStatusBlock, 
										IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE );
			queue->FileObject->CurrentByteOffset = FileObjectOffset;

			if( !NT_SUCCESS( status ) && status != STATUS_END_OF_FILE )
			{
				TCfree( TempReadBuffer );
				return status;
			}

			if( NT_SUCCESS( status ) )
			{
				dataUnit.Value = 0;
				DecryptDataUnits( TempReadBuffer, &dataUnit, TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE/ENCRYPTION_DATA_UNIT_SIZE, queue->CryptoInfo );
				RtlCopyMemory( queue->FragmentBufferA, TempReadBuffer, TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
			}

			TCfree( TempReadBuffer );
		}

		///Copy the buffer into the memory
		CurrentOffset = (ULONG)(actualOffset - (queue->CurrentWriteOffset)); // + queue->CurrentWriteLength));
		CurrentLength = 0;

		if( CurrentOffset + dataRemaining > TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE )
		{
			CurrentLength = TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE - CurrentOffset;
		}else{
			CurrentLength = dataRemaining;
		}
		
		RtlCopyMemory( queue->FragmentBufferA + queue->CurrentWriteLength, Buffer + (length - dataRemaining), CurrentLength );
		actualOffset += CurrentLength;
		dataRemaining = dataRemaining - CurrentLength;
		queue->GingkoObject->OffsetBytes.QuadPart = MAX( queue->GingkoObject->OffsetBytes.QuadPart, actualOffset );
		queue->CurrentWriteLength += CurrentLength;

		if( queue->CurrentWriteLength == TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE )
		{
			FileOffset.QuadPart = queue->CurrentWriteOffset  + GingkoHeaderOffset;
			dataUnit.Value = 0; //  queue->CurrentWriteOffset / ENCRYPTION_DATA_UNIT_SIZE;
			
			EncryptDataUnits( queue->FragmentBufferA, &dataUnit, 
				TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE / ENCRYPTION_DATA_UNIT_SIZE, 
				queue->CryptoInfo);

			FileObjectOffset = queue->FileObject->CurrentByteOffset;
			queue->FileObject->CurrentByteOffset = FileOffset;
			IrpFlags = queue->GingkoObject->OffsetBytes.QuadPart >=  (FileOffset.QuadPart+(TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE)) ? 
						IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE: IRP_WRITE_OPERATION | IRP_NOCACHE;

			status = TCWriteDeviceEx(   queue->LowerDeviceObject, queue->FileObject, 
										queue->FragmentBufferA, &FileOffset, 
										TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE, &IoStatusBlock, 
										IrpFlags );
			///Restore it because this value will be notified to the operation.
			queue->FileObject->CurrentByteOffset = FileObjectOffset;

			if( !NT_SUCCESS( status ) )
			{
				//queue->FileObject->CurrentByteOffset = FileObjectOffset;
				KdPrint(("Write 256K buffer with error status: %08x\n", status));
				return status;
			}

			if( NT_SUCCESS( status ) )
			{
				//queue->FileObject->CurrentByteOffset.QuadPart -= sizeof(GINGKO_HEADER);
				RtlZeroMemory( queue->FragmentBufferA, TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE );
				queue->CurrentWriteOffset += queue->CurrentWriteLength;
				queue->CurrentWriteLength = 0;
			}
		}
	}

	return status;
}

static NTSTATUS ProcessRead( EncryptedIoQueue *queue, byte* dataBuffer, __int64 offset, ULONG length, ULONG* pCommitLength )
{
	LONG dataRemaining = length;
	ULONG  commitLength = 0;
	NTSTATUS status = 0;
	LARGE_INTEGER LoadOffset = {0};
	byte *fragmentBuffer;
	ULONG alignedLength = 0L; 
	LARGE_INTEGER alignedOffset;
	IO_STATUS_BLOCK IoStatusBlock;
	ULONG intersectLength = 0;
	ULONG NextCopyOffset = 0L;

	fragmentBuffer = queue->FragmentBufferA;
	alignedOffset.QuadPart = offset;

	while ( dataRemaining > 0L )
	{
		if( queue->CurrentOffset == 0L && queue->CurrentLength == 0L && queue->CurrentStatus == STATUS_SUCCESS )
		{
				LoadOffset.QuadPart = (alignedOffset.QuadPart / TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE ) * (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
		}else{
			if( alignedOffset.QuadPart >= queue->CurrentOffset )
			{
				if( alignedOffset.QuadPart <= queue->CurrentOffset + queue->CurrentLength ){
					///The request length was in buffer, so, copy the current byte to buffer.
					long BufferOffset = (long)(alignedOffset.QuadPart - queue->CurrentOffset);
					if( BufferOffset + dataRemaining <= queue->CurrentLength )
					{
						//Copy the memory directly.
						RtlCopyMemory( dataBuffer + NextCopyOffset, queue->FragmentBufferA + BufferOffset, dataRemaining );
						//////Finished all request.
						commitLength += dataRemaining;
						status = STATUS_SUCCESS;
						break;
					}else{
						RtlCopyMemory( dataBuffer + NextCopyOffset, fragmentBuffer + BufferOffset, queue->CurrentLength - BufferOffset );
						NextCopyOffset += (queue->CurrentLength - BufferOffset);
						commitLength   += (queue->CurrentLength - BufferOffset);
						dataRemaining  -= (queue->CurrentLength - BufferOffset);
						alignedOffset.QuadPart += (queue->CurrentLength - BufferOffset);
						if( queue->CurrentStatus == STATUS_END_OF_FILE )
						{
							status = STATUS_SUCCESS;
							break;
						}else{
							LoadOffset.QuadPart = queue->CurrentOffset + queue->CurrentLength;
						}
					}
				}else{
					if( queue->CurrentStatus == STATUS_END_OF_FILE )
					{
						status = commitLength ? STATUS_SUCCESS : STATUS_END_OF_FILE;
						break;
					}
					LoadOffset.QuadPart = (alignedOffset.QuadPart / (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE)) * (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
				}
			}else{
				//if( queue->CurrentStatus == STATUS_END_OF_FILE )
				//{
				//	item->Status = commitLength ? STATUS_SUCCESS : STATUS_END_OF_FILE;
				//	break;
				//}
				LoadOffset.QuadPart = (alignedOffset.QuadPart / (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE)) * (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
			}
		}

		if(TRUE){
			LARGE_INTEGER  ActualOffset = {0};
			ActualOffset.QuadPart = LoadOffset.QuadPart + sizeof(GINGKO_HEADER);
			status = TCReadDeviceEx ( queue->LowerDeviceObject, 
									  (PFILE_OBJECT)queue->FileObject, 
									  fragmentBuffer, 
									  &ActualOffset, 
									  TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE, 
									  &IoStatusBlock,
									  IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE);
			if( NT_SUCCESS( status ) )
			{
				UINT64_STRUCT dataUnit;
				TC_LARGEST_COMPILER_UINT unit = (IoStatusBlock.Information / ENCRYPTION_DATA_UNIT_SIZE) + ((IoStatusBlock.Information % ENCRYPTION_DATA_UNIT_SIZE) ? 1 : 0);
				TC_LARGEST_COMPILER_UINT perunit = 0L;

				dataUnit.Value = 0; //LoadOffset.QuadPart / ENCRYPTION_DATA_UNIT_SIZE;
				//while( perunit <= unit )
				//{
				//	TC_LARGEST_COMPILER_UINT dounit = 0L;
				//	if( unit - perunit > 16 )
				//	{
				//		dounit = 16L;
				//	}else
				//	{
				//		dounit = unit - perunit;
				//	}
				//	DecryptDataUnits ( fragmentBuffer, &dataUnit, 
				//	dounit, //TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE / ENCRYPTION_DATA_UNIT_SIZE, 
				//	queue->CryptoInfo);
				//	perunit += 16L;
				//	dataUnit.Value = perunit;
				//}

					DecryptDataUnits ( fragmentBuffer, &dataUnit, 
					unit, //TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE / ENCRYPTION_DATA_UNIT_SIZE, 
					queue->CryptoInfo);

				queue->CurrentStatus = status;
				queue->CurrentOffset = LoadOffset.QuadPart;
				queue->CurrentLength = (ULONG)IoStatusBlock.Information; //FileIsEnded.
				
				if( IoStatusBlock.Information < TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE )
				{
					queue->CurrentLength = (LONG)(queue->GingkoHeader->SizeOfFile - queue->CurrentOffset);
					queue->CurrentStatus = STATUS_END_OF_FILE;
				}

			}else
			{
				queue->CurrentStatus = STATUS_END_OF_FILE;
				queue->CurrentOffset = LoadOffset.QuadPart;
				queue->CurrentLength = 0L;
			}
		}
	}
	
	*pCommitLength = commitLength;
	
	return status;
}



NTSTATUS OnPassedIrpCompleted (PDEVICE_OBJECT filterDeviceObject, PIRP irp, EncryptedIoQueueItem *item)
{
	//KdPrint(("OnPassedIrpCompleted.\n"));
	if (irp->PendingReturned)
		IoMarkIrpPending (irp);

	OnItemCompleted (item, TRUE);
	return STATUS_CONTINUE_COMPLETION;
}


VOID MainThreadProc (PVOID threadArg)
{
	EncryptedIoQueue *queue = (EncryptedIoQueue *) threadArg;
	PLIST_ENTRY listEntry;
	EncryptedIoQueueItem *item = NULL;
	IO_STATUS_BLOCK IoStatusBlock;
	PUCHAR activeFragmentBuffer = queue->FragmentBufferA;
	PUCHAR dataBuffer = NULL;
	EncryptedIoRequest *request = NULL;
	uint64 intersectStart = 0;
	//uint32 intersectLength = 0;

	int mdlWaitTime;
	LARGE_INTEGER mdlWaitInterval;
	mdlWaitInterval.QuadPart = TC_ENC_IO_QUEUE_MEM_ALLOC_RETRY_DELAY * -10000;

	while (!queue->ThreadExitRequested)
	{
		if( queue->CryptoInfo == NULL && queue->ShouldPassthrough == FALSE )
		{
			if (!NT_SUCCESS(KeWaitForSingleObject(&queue->RequestKeyEvent, Executive, KernelMode, FALSE, NULL)))
			{	
				continue;
			}

			if( queue->CryptoInfo == NULL )
			{
				queue->ShouldPassthrough = TRUE;
				queue->AbstractOffset = 0L;
				queue->FileObject->CurrentByteOffset.QuadPart = 0L;
			}else{
				if( queue->GingkoHeader->SizeOfBlock == 0 )
				{
					queue->GingkoHeader->SizeOfBlock = CipherGetBlockSize( queue->CryptoInfo->ea );
				}
			}
		}

		if (!NT_SUCCESS (KeWaitForSingleObject (&queue->MainThreadQueueNotEmptyEvent, Executive, KernelMode, FALSE, NULL)))
		{
			continue;
		}

		if( queue->ShouldPassthrough == FALSE )
		{
			if( queue->GingkoHeader != NULL )
			{
				if( queue->GingkoHeader->SizeOfHeader == 0 )
				{
					LARGE_INTEGER Offset = {0};
					LARGE_INTEGER OldOffset = {0};
					IO_STATUS_BLOCK IoStatusBlock;
					queue->GingkoHeader->SizeOfHeader = sizeof(GINGKO_HEADER);
					queue->GingkoHeader->SizeOfFile = 0L;
					queue->GingkoHeader->SizeOfBlock = 0L;
					OldOffset = queue->FileObject->CurrentByteOffset;
					queue->FileObject->CurrentByteOffset = Offset;
					TCWriteDeviceEx( queue->LowerDeviceObject, queue->FileObject, 
						queue->GingkoHeader, &Offset, sizeof(GINGKO_HEADER), &IoStatusBlock,
						//IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE);
						IRP_WRITE_OPERATION | IRP_NOCACHE);
					queue->FileObject->CurrentByteOffset = OldOffset;
					queue->AbstractOffset = sizeof(GINGKO_HEADER);
				}
			}
		}
		

		while ((listEntry = ExInterlockedRemoveHeadList (&queue->MainThreadQueue, &queue->MainThreadQueueLock)))
		{
			PIRP irp = CONTAINING_RECORD (listEntry, IRP, Tail.Overlay.ListEntry);
			PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (irp);

			if (queue->Suspended)
			{
				KeWaitForSingleObject (&queue->QueueResumedEvent, Executive, KernelMode, FALSE, NULL);
			}

			item = AllocateMemoryWithTimeout (sizeof (EncryptedIoQueueItem), TC_ENC_IO_QUEUE_MEM_ALLOC_RETRY_DELAY, TC_ENC_IO_QUEUE_MEM_ALLOC_TIMEOUT);
			
			if (!item)
			{
				EncryptedIoQueueItem stackItem;
				stackItem.Queue = queue;
				stackItem.OriginalIrp = irp;
				stackItem.Status = STATUS_INSUFFICIENT_RESOURCES;
				TCCompleteDiskIrp (irp, STATUS_INSUFFICIENT_RESOURCES, 0);
				OnItemCompleted (&stackItem, FALSE);
				continue;
			}

			item->Queue = queue;
			item->OriginalIrp = irp;
			item->ShouldReleaseMdl = FALSE;
			item->OutstandingRequestCount = 0;
			item->Status = STATUS_SUCCESS;

			IoSetCancelRoutine (irp, NULL);
			if (irp->Cancel)
			{
				CompleteOriginalIrp (item, STATUS_CANCELLED, 0);
				continue;
			}

			switch (irpSp->MajorFunction)
			{
			case IRP_MJ_READ:
				item->Write = FALSE;
				item->OriginalOffset = irpSp->Parameters.Read.ByteOffset;
				item->OriginalLength = irpSp->Parameters.Read.Length;
				KdPrint(("Read File %wZ From offset %I64d (L: %08x, H: %08x) length %08x.\n", &(item->Queue->GingkoObject->FileName), 
					item->OriginalOffset.QuadPart,item->OriginalOffset.LowPart,item->OriginalOffset.HighPart, item->OriginalLength ) );
				break;

			case IRP_MJ_WRITE:
				item->Write = TRUE;
				item->OriginalOffset = irpSp->Parameters.Write.ByteOffset;
				item->OriginalLength = irpSp->Parameters.Write.Length;
				break;
			case IRP_MJ_CLEANUP:
				{
					IoStatusBlock.Status = 0;
					IoStatusBlock.Information = 0;
					DirectIrpCleanup( (PFILE_OBJECT)queue->FileObject, queue->LowerDeviceObject, PsGetCurrentThread(), &IoStatusBlock );
					CompleteOriginalIrp (item, IoStatusBlock.Status, (ULONG_PTR) item->Queue->FileObject);
				}
				continue;
			default:
				CompleteOriginalIrp (item, STATUS_INVALID_PARAMETER, 0);
				continue;
			}

			// Original IRP data buffer
			mdlWaitTime = 0;
			while (TRUE)
			{
				if( irp->MdlAddress != NULL )
				{
					//KdPrint(("DataBuffer from 2.\n"));
					dataBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe (irp->MdlAddress, HighPagePriority);

					if (dataBuffer || mdlWaitTime >= TC_ENC_IO_QUEUE_MEM_ALLOC_TIMEOUT)
					{
						if( irp->UserBuffer != NULL )
						{
							item->ShouldReleaseMdl = TRUE;
						}
						break;
					}

					KeDelayExecutionThread (KernelMode, FALSE, &mdlWaitInterval);
					mdlWaitTime += TC_ENC_IO_QUEUE_MEM_ALLOC_RETRY_DELAY;
				}else
				{
					dataBuffer = NULL;
					break;
				}
			}

			// Pass the IRP if the drive is not encrypted
			if (queue->IsFilterDevice && queue->ShouldPassthrough )  ///at here, if it is the read operation, the operation will be passthrough.
			{
				if( !item->Write )
				{
					//IoCopyCurrentIrpStackLocationToNext (irp);
					//IoSkipCurrentIrpStackLocationToNext (irp);
					//IoSetCompletionRoutine (irp, OnPassedIrpCompleted, item, TRUE, TRUE, TRUE);
					//IoCallDriver (queue->LowerDeviceObject, irp);
					LARGE_INTEGER  ActualOffset = {0};
					NTSTATUS ReadStatus = STATUS_SUCCESS;
					
					KdPrint(("Because we can not get the crypto key for decrytion.\n"));
					ActualOffset.QuadPart = item->OriginalOffset.QuadPart; // + sizeof(GINGKO_HEADER);
					
					ReadStatus = TCReadDeviceEx ( queue->LowerDeviceObject, 
											  (PFILE_OBJECT)queue->FileObject, 
											  dataBuffer, 
											  &ActualOffset, item->OriginalLength, 
											  &IoStatusBlock,
											  IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE);

					CompleteOriginalIrp (item, ReadStatus, NT_SUCCESS( ReadStatus ) ? IoStatusBlock.Information : 0L );
				}else{
					LARGE_INTEGER  ActualOffset = {0};
					NTSTATUS WriteStatus = STATUS_SUCCESS;
					KdPrint(("Because we can not get the crypto key for encryption.\n"));
					ActualOffset.QuadPart = item->OriginalOffset.QuadPart; // + sizeof(GINGKO_HEADER);

					WriteStatus = TCWriteDeviceEx( queue->LowerDeviceObject, 
											  (PFILE_OBJECT)queue->FileObject, 
											  dataBuffer, 
											  &ActualOffset, item->OriginalLength, 
											  &IoStatusBlock,
											  IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | IRP_NOCACHE);

					CompleteOriginalIrp (item, WriteStatus, NT_SUCCESS( WriteStatus ) ? IoStatusBlock.Information : 0L );

					//CompleteOriginalIrp (item, STATUS_INVALID_PARAMETER, 0);
				}
				continue;

			}



			// Handle misaligned reads to support Windows System Assessment Tool which reads from disk devices at offsets not aligned on sector boundaries
			if ( dataBuffer != NULL 
				&& queue->IsFilterDevice
				&& !item->Write
				&& item->OriginalLength > 0)
			{
				ULONG commitLength = 0;
				item->Status = ProcessRead( queue, dataBuffer, item->OriginalOffset.QuadPart, item->OriginalLength, &commitLength );
				if( NT_SUCCESS( item->Status ) )
				{
					queue->FileObject->CurrentByteOffset.QuadPart = item->OriginalOffset.QuadPart + commitLength;
				}
				CompleteOriginalIrp (item, item->Status, NT_SUCCESS (item->Status) ? commitLength : 0);
				continue;
			} else if ( queue->IsFilterDevice && item->Write )
			{
				item->Status = ProcessWrite( queue, dataBuffer, 
									item->OriginalOffset.QuadPart, item->OriginalLength, 
									 IRP_SYNCHRONOUS_PAGING_IO | IRP_PAGING_IO | 0x40000001, // | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO | IRP_NOCACHE, 
									(dataBuffer == NULL) );
				//(item->OriginalIrp->Flags &~IRP_DEFER_IO_COMPLETION ) |
				if( NT_SUCCESS(item->Status) )
				{
					queue->FileObject->CurrentByteOffset.QuadPart = item->OriginalOffset.QuadPart + item->OriginalLength;
				}
				CompleteOriginalIrp( item, item->Status, NT_SUCCESS(item->Status) ? item->OriginalLength : 0 );
				continue;
			}else{
				KdPrint(("I don't know why come here.\n"));
				CompleteOriginalIrp( item, STATUS_SUCCESS, 0 );
			}
		}

	}

	//ZwClose( queue->HostFileObject );

	PsTerminateSystemThread (STATUS_SUCCESS);
}


NTSTATUS EncryptedIoQueueAddIrp (EncryptedIoQueue *queue, PIRP irp)
{
	NTSTATUS status;
	InterlockedIncrement (&queue->OutstandingIoCount);
	if (queue->StopPending)
	{
		status = STATUS_DEVICE_NOT_READY;
		goto err;
	}

	if (queue->IsFilterDevice)
	{
		//status = IoAcquireRemoveLock (&queue->RemoveLock, irp);
		//if (!NT_SUCCESS (status))
		//		goto err;
	}

	IoMarkIrpPending (irp);

	ExInterlockedInsertTailList (&queue->MainThreadQueue, &irp->Tail.Overlay.ListEntry, &queue->MainThreadQueueLock);
	
	KeReleaseSemaphore (&queue->MainThreadQueueNotEmptyEvent, 0, 1, FALSE);
	
	return STATUS_PENDING;

err:
	DecrementOutstandingIoCount (queue);
	return status;
}


NTSTATUS EncryptedIoQueueHoldWhenIdle (EncryptedIoQueue *queue, int64 timeout)
{
	NTSTATUS status;
	ASSERT (!queue->Suspended);

	KdPrint(("EncryptedIoQueueHoldWhenIdle.\n"));
	queue->SuspendPending = TRUE;
	
	while (TRUE)
	{
		while (InterlockedExchangeAdd (&queue->OutstandingIoCount, 0) > 0)
		{
			LARGE_INTEGER waitTimeout;

			waitTimeout.QuadPart = timeout * -10000;
			status = KeWaitForSingleObject (&queue->NoOutstandingIoEvent, Executive, KernelMode, FALSE, timeout != 0 ? &waitTimeout : NULL);
			if (status == STATUS_TIMEOUT)
				status = STATUS_UNSUCCESSFUL;

			if (!NT_SUCCESS (status))
				return status;

			TCSleep (1);
			if (InterlockedExchangeAdd (&queue->OutstandingIoCount, 0) > 0)
				return STATUS_UNSUCCESSFUL;
		}

		KeClearEvent (&queue->QueueResumedEvent);
		queue->Suspended = TRUE;

		if (InterlockedExchangeAdd (&queue->OutstandingIoCount, 0) == 0)
			break;

		queue->Suspended = FALSE;
		KeSetEvent (&queue->QueueResumedEvent, IO_DISK_INCREMENT, FALSE);

	}

	queue->SuspendPending = FALSE;
	//Dump ("Queue suspended  out=%d\n", queue->OutstandingIoCount);

	return STATUS_SUCCESS;
}


BOOL EncryptedIoQueueIsSuspended (EncryptedIoQueue *queue)
{
	KdPrint(("EncryptedIoQueueIsSuspended.\n"));
	return queue->Suspended;
}


BOOL EncryptedIoQueueIsRunning (EncryptedIoQueue *queue)
{
	KdPrint(("EncryptedIoQueueIsRunning.\n"));
	return !queue->StopPending;
}


NTSTATUS EncryptedIoQueueResumeFromHold (EncryptedIoQueue *queue)
{
	KdPrint(("EncryptedIoQueueResumeFromHold.\n"));
	ASSERT (queue->Suspended);
	
	queue->Suspended = FALSE;
	KeSetEvent (&queue->QueueResumedEvent, IO_DISK_INCREMENT, FALSE);

	//Dump ("Queue resumed  out=%d\n", queue->OutstandingIoCount);

	return STATUS_SUCCESS;
}


NTSTATUS EncryptedIoQueueStart (EncryptedIoQueue *queue)
{
	NTSTATUS status;
	SharedNotificationPtr SharedNotification = NULL;
	queue->ThreadExitRequested = FALSE;
	queue->StopPending = TRUE;

	KeInitializeEvent (&queue->NoOutstandingIoEvent, SynchronizationEvent, FALSE);
	KeInitializeEvent (&queue->RequestCompletedEvent, SynchronizationEvent, FALSE);
	KeInitializeEvent (&queue->QueueResumedEvent, SynchronizationEvent, FALSE);
	KeInitializeEvent (&queue->RequestKeyEvent, NotificationEvent, FALSE);

	queue->FragmentBufferA = TCalloc (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
	
	if (!queue->FragmentBufferA)
		goto noMemory;

	//queue->FragmentBufferB = TCalloc (TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE);
	//if (!queue->FragmentBufferB)
	//	goto noMemory;

	KeInitializeEvent (&queue->FragmentBufferAFreeEvent, SynchronizationEvent, TRUE);
	//KeInitializeEvent (&queue->FragmentBufferBFreeEvent, SynchronizationEvent, TRUE);

	// Main thread
	InitializeListHead (&queue->MainThreadQueue);
	KeInitializeSpinLock (&queue->MainThreadQueueLock);
	//KeInitializeEvent (&queue->MainThreadQueueNotEmptyEvent, SynchronizationEvent, FALSE);
	KeInitializeSemaphore( &queue->MainThreadQueueNotEmptyEvent, 0L, MAXLONG );


	// IO thread
	//InitializeListHead (&queue->IoThreadQueue);
	//KeInitializeSpinLock (&queue->IoThreadQueueLock);
	//KeInitializeEvent (&queue->IoThreadQueueNotEmptyEvent, SynchronizationEvent, FALSE);

	//status = TCStartThread (IoThreadProc, queue, &queue->IoThread);
	//if (!NT_SUCCESS (status))
	//{
	//	queue->ThreadExitRequested = TRUE;
	//	TCStopThread (queue->MainThread, &queue->MainThreadQueueNotEmptyEvent);
	//	goto err;
	//}

	// Completion thread
	//InitializeListHead (&queue->CompletionThreadQueue);
	//KeInitializeSpinLock (&queue->CompletionThreadQueueLock);
	//KeInitializeEvent (&queue->CompletionThreadQueueNotEmptyEvent, SynchronizationEvent, FALSE);

	//status = TCStartThread (CompletionThreadProc, queue, &queue->CompletionThread);
	//if (!NT_SUCCESS (status))
	//{
	//	queue->ThreadExitRequested = TRUE;
	//	TCStopThread (queue->MainThread, &queue->MainThreadQueueNotEmptyEvent);
	//	TCStopThread (queue->IoThread, &queue->IoThreadQueueNotEmptyEvent);
	//	goto err;
	//}

	///KeWaitForSingleObject( &gAcquireRequestKeyEvent, Executive, KernelMode, FALSE, NULL );
	
	//KeSetEvent( &gAcquireRequestKeyEvent, IO_NO_INCREMENT, FALSE );
	

	queue->CurrentLength = 0L;
	queue->CurrentStatus = STATUS_SUCCESS;
	queue->CurrentOffset = 0L;
	queue->CurrentWriteLength = 0L;
	queue->CurrentWriteStatus = STATUS_SUCCESS;
	queue->CurrentWriteOffset = 0L;

	queue->StopPending = FALSE;

	SharedNotification = GingkoReferenceSharedNotification();
	if( SharedNotification && SharedNotification->ReadEvent )
	{
		ULONG Permission = 0L;
		unsigned char* CryptoKeyBuf = NULL; //[MAXSIZE_PRIVATE_KEY] = {0};
		KIRQL irql;
		GingkoDereferenceSharedNotification(SharedNotification);
		
		CryptoKeyBuf = TCalloc( MAXSIZE_PRIVATE_KEY );
		
		if( FindCryptoPermission( queue->dwProcessId, queue->GingkoHeader->Md5, &Permission, CryptoKeyBuf, &irql ) )
		{
			ReadPrimaryKey( CryptoKeyBuf, &(queue->CryptoInfo) );
			TCfree( CryptoKeyBuf );
			CryptoKeyBuf = NULL;
		}

		if( queue->CryptoInfo == NULL )
		{
			ExInterlockedInsertTailList( &gGingkoPublicKeyRequestQueue, &(queue->RequestKeyList), &gGingkoPublicKeyRequestLock );
		}

		status = TCStartThread (MainThreadProc, queue, &queue->MainThread);
		
		if (!NT_SUCCESS (status))
			goto err;


		KeSetEvent( SharedNotification->ReadEvent, IO_NO_INCREMENT, FALSE );
	}else{
		goto err;
	}

	KdPrint (("Queue started\n"));

	return STATUS_SUCCESS;

noMemory:
	status = STATUS_INSUFFICIENT_RESOURCES;
	KdPrint (("No Memory\n"));
err:
	if (queue->FragmentBufferA)
		TCfree (queue->FragmentBufferA);
	//if (queue->FragmentBufferB)
	//	TCfree (queue->FragmentBufferB);

	KdPrint (("Queue can not started\n"));
	return status;
}


BOOLEAN EncryptedIoQueueCleanup (EncryptedIoQueue *queue)
{
	if( queue->CurrentWriteLength > 0 )
	{
		NTSTATUS status = ProcessWrite( queue, NULL, queue->GingkoObject->OffsetBytes.QuadPart , 0, 0, FALSE); 
		if( !NT_SUCCESS( status ) )
		{
			return FALSE;
		}
		queue->CurrentWriteOffset += queue->CurrentWriteLength;
		queue->CurrentWriteLength = 0;
	}

	//GingkoFileFlushCache( queue->FileObject );
	return TRUE;
}

/***
 * Before Stop the Queue, the file in the FragementBufferB should be sync into the file. and the cache also should be clear.
 */
NTSTATUS EncryptedIoQueueStop (EncryptedIoQueue *queue)
{
	PLIST_ENTRY tempQueue = NULL;
	PLIST_ENTRY pfirst = NULL;
	PFILE_OBJECT FileObject = queue->FileObject;
	PDEVICE_OBJECT DeviceObject = queue->LowerDeviceObject;
	IO_STATUS_BLOCK IoStatusBlock = {0};
	NTSTATUS status = STATUS_SUCCESS;
	ASSERT (!queue->StopPending);
	queue->StopPending = TRUE;



	//KeWaitForSingleObject( &gAcquireRequestKeyEvent, Executive, KernelMode, FALSE, NULL );
	
	//KeSetEvent( &gAcquireRequestKeyEvent, IO_NO_INCREMENT, FALSE );

	while( (tempQueue = ExInterlockedRemoveHeadList( &gGingkoPublicKeyRequestQueue, &gGingkoPublicKeyRequestLock )) != NULL )
	{
		EncryptedIoQueue* request = CONTAINING_RECORD (tempQueue, EncryptedIoQueue, RequestKeyList);
		if( pfirst == NULL )
		{
			pfirst = tempQueue;
		}else if( pfirst == tempQueue ){
			ExInterlockedInsertTailList(&gGingkoPublicKeyRequestQueue, &(request->RequestKeyList), &gGingkoPublicKeyRequestLock);
			break;
		}
		if( request == queue )
		{
			break;
		}
		ExInterlockedInsertTailList(&gGingkoPublicKeyRequestQueue, &(request->RequestKeyList), &gGingkoPublicKeyRequestLock);
	}

	//KeSetEvent( &gAcquireRequestKeyEvent, IO_NO_INCREMENT, FALSE );

	///Attemp to remove the EncryptedIoQueue from the list
	FindEncryptionIoQueueFromResponseKeyList( (LONG_PTR)queue->FileObject, (LONG_PTR)&(queue->RequestKeyEvent), NULL );

	//ASSERT( tempQueue == queue || tempQueue == NULL );
	while (InterlockedExchangeAdd (&queue->OutstandingIoCount, 0) > 0)
	{
		KeWaitForSingleObject (&queue->NoOutstandingIoEvent, Executive, KernelMode, FALSE, NULL);
	}

	queue->ThreadExitRequested = TRUE;


	KeSetEvent( &(queue->RequestKeyEvent), IO_NO_INCREMENT, FALSE );


	TCStopThreadBySemaphore ( queue->MainThread, &queue->MainThreadQueueNotEmptyEvent);

	//TCStopThread (queue->IoThread, &queue->IoThreadQueueNotEmptyEvent);
	//TCStopThread (queue->CompletionThread, &queue->CompletionThreadQueueNotEmptyEvent);

	TCfree (queue->FragmentBufferA);
	//TCfree (queue->FragmentBufferB);

	///Cleanup the FileObject related cache.
	//GingkoFileFlushCache( FileObject );

	//DirectIrpCleanup( FileObject, DeviceObject, KeGetCurrentThread(), &IoStatusBlock );

	return IoStatusBlock.Status;
}

NTSTATUS GingkoAsynchronousRequestCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PMDL mdl, nextMdl;

	//if( Irp->PendingReturned )
	//{
	//	IoMarkIrpPending( Irp );
	//	return STATUS_PENDING;
	//}

	if( Irp->UserIosb )
	{
		*Irp->UserIosb = Irp->IoStatus;
	}

    if(Irp->AssociatedIrp.SystemBuffer && (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {
            ExFreePool(Irp->AssociatedIrp.SystemBuffer);
    }
    else if (Irp->MdlAddress != NULL) {
        for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
            nextMdl = mdl->Next;
            MmUnlockPages( mdl ); 
			IoFreeMdl( mdl ); // This function will also unmap pages.
        }
        Irp->MdlAddress = NULL;
    }

	//IoFreeIrp(Irp);

	if(Context) {
		KeSetEvent( (PKEVENT)Context, IO_NO_INCREMENT, FALSE );
	}

	//IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
GingkoCompletedReadWriteRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
	PMDL mdl = NULL, nextMdl = NULL;
	PKEVENT Event = Context;
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( Context );

	if( Irp->UserIosb )
	{
		*Irp->UserIosb = Irp->IoStatus;
	}

    if(Irp->AssociatedIrp.SystemBuffer && (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {
            ExFreePool(Irp->AssociatedIrp.SystemBuffer);
    }
    else if (Irp->MdlAddress != NULL) {
        for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
            nextMdl = mdl->Next;
            MmUnlockPages( mdl ); 
			IoFreeMdl( mdl ); // This function will also unmap pages.
        }
        Irp->MdlAddress = NULL;
    }

	Irp->Tail.Overlay.Thread = NULL; //(PETHREAD)KeGetCurrentThread();

	if(Context) {
		KeSetEvent( (PKEVENT)Context, IO_NO_INCREMENT, FALSE );
	}

	//return STATUS_CONTINUE_COMPLETION;
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS TCReadWriteDeviceSync (BOOL write, PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, 
									   PVOID buffer, PLARGE_INTEGER offset, ULONG length,
									   PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags)
{
	NTSTATUS status;
	PIRP irp;
	KEVENT completionEvent;
	PIO_STACK_LOCATION pCurrStack = NULL;

	ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

	KeInitializeEvent ( &completionEvent, NotificationEvent, FALSE);
	
	irp = IoBuildSynchronousFsdRequest (write ? IRP_MJ_WRITE : IRP_MJ_READ, deviceObject, 
		buffer, length, offset, &completionEvent, IoStatusBlock);

	if (!irp)
		return STATUS_INSUFFICIENT_RESOURCES;

	irp->Flags = IrpFlags; //IRP_READ_OPERATION;

	pCurrStack = IoGetNextIrpStackLocation(irp);
	
	pCurrStack->FileObject = FileObject;

	IoSetCompletionRoutine(
               irp,
               GingkoCompletedReadWriteRoutine, //GingkoAsynchronousRequestCompletion,
               &completionEvent,
               TRUE,
               TRUE,
               TRUE );

	//ObReferenceObject (deviceObject);

	status = IoCallDriver (deviceObject, irp);

	//ObDereferenceObject (deviceObject);

	if ( status == STATUS_PENDING )
	{
		status = KeWaitForSingleObject ( &completionEvent, Executive, KernelMode, FALSE, NULL );
		if (NT_SUCCESS (status))
			status = IoStatusBlock->Status;
	}

	IoCompleteRequest( irp, IO_NO_INCREMENT );	
	//if( completionEvent ){
	//	TCfree( completionEvent );
	//}
	return status;
}


 NTSTATUS TCReadWriteDevice (BOOL write, PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags)
{
	NTSTATUS status;
	
	PIO_STACK_LOCATION pCurrStack = NULL;
	PIRP irp;
	KEVENT completionEvent;
	BOOLEAN isSynchronous = TRUE;

	ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

	KeInitializeEvent (&completionEvent, NotificationEvent, FALSE);

#ifdef __INTERNAL_ASYNC_IO__
	irp = IoBuildAsynchronousFsdRequest( write ? IRP_MJ_WRITE : IRP_MJ_READ,  //IoBuildSynchronousFsdRequest (write ? IRP_MJ_WRITE : IRP_MJ_READ, 
		deviceObject, buffer, 
		length, offset, 
		IoStatusBlock);
#else	
	irp = IoBuildSynchronousFsdRequest( write ? IRP_MJ_WRITE : IRP_MJ_READ,  //IoBuildSynchronousFsdRequest (write ? IRP_MJ_WRITE : IRP_MJ_READ, 
		deviceObject, buffer, 
		length, offset, &completionEvent,
		IoStatusBlock);
#endif

	if (!irp)
		return STATUS_INSUFFICIENT_RESOURCES;
	
	//IoSetNextIrpStackLocation(irp);
	irp->Flags = IrpFlags; //IRP_READ_OPERATION;
	
	pCurrStack = IoGetNextIrpStackLocation(irp);
	
	pCurrStack->FileObject = FileObject;

	//IoSetCompletionRoutine(irp,
	//                  MakeSynchronousNonIoctlRequestCompletion,
	//                  context,
	//                  TRUE,
	//                  TRUE,
	//                  TRUE);

#ifdef __INTERNAL_ASYNC_IO__
	IoSetCompletionRoutine(
               irp,
               GingkoAsynchronousRequestCompletion,
               &completionEvent,
               TRUE,
               TRUE,
               TRUE );
#else
	IoSetCompletionRoutine(
               irp,
               GingkoCompletedReadWriteRoutine, //GingkoAsynchronousRequestCompletion,
               &completionEvent,
               TRUE,
               TRUE,
               TRUE );
#endif

	//IoCopyCurrentIrpStackLocationToNext( irp );
	//ObReferenceObject( FileObject );
	//ObReferenceObject (deviceObject);
	//ObReferenceObject (deviceObject);
	status = IoCallDriver (deviceObject, irp);
    if( status == STATUS_PENDING )
    {
       //KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);
	   KeWaitForSingleObject( &completionEvent, Executive, KernelMode, FALSE, NULL);
    }
	
	KeClearEvent( &completionEvent );
	
	status = IoStatusBlock->Status;

#ifdef __INTERNAL_ASYNC_IO__	
	IoFreeIrp( irp );
#else
	IoCompleteRequest(irp, IO_NO_INCREMENT);
#endif

	//ObDereferenceObject (deviceObject);

    //if (!(NT_ERROR(status) && isSynchronous)) {
    //    KeWaitForSingleObject(&completionEvent,
    //                          Executive,
    //                          KernelMode,
    //                          FALSE, // Not alertable
    //                          NULL);
    //}	
	//ObDereferenceObject (FileObject);
	return status;
}

NTSTATUS TCReadDeviceDirect (PDEVICE_OBJECT DeviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags )
{
	return DirectIrpFileRead(
		DeviceObject,
		FileObject,
		offset,
		length,
		IrpFlags,
		buffer,
		IoStatusBlock
	);
}

NTSTATUS TCReadDeviceEx (PDEVICE_OBJECT DeviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags )
{
	PDEVICE_OBJECT DevObj = DeviceObject != NULL ? DeviceObject : FileObject->Vpb->DeviceObject;
	NTSTATUS Status = STATUS_SUCCESS;
	__try{
		//Status = TCReadWriteDeviceSync (FALSE, DevObj, FileObject, buffer, offset, length, IoStatusBlock, IrpFlags);
		Status = TCReadWriteDevice (FALSE, DevObj, FileObject, buffer, offset, length, IoStatusBlock, IrpFlags);
	}__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Error to Read FileObject: %p, %d, Offset: %d Length: %d.\n", FileObject, DevObj, offset, length ));
		Status = STATUS_UNSUCCESSFUL;
	}
	return Status;
}

NTSTATUS TCReadDevice (PDEVICE_OBJECT DeviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock )
{
	return TCReadDeviceEx (DeviceObject, FileObject, buffer, offset, length, IoStatusBlock, IRP_READ_OPERATION );
}


NTSTATUS TCWriteDevice (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock)
{
	return TCWriteDeviceEx (deviceObject, FileObject, buffer, offset, length, IoStatusBlock, IRP_WRITE_OPERATION );
}

NTSTATUS TCWriteDeviceEx (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags)
{
	KdPrint (("TCWriteDeviceEx\n"));
	return TCReadWriteDevice (TRUE, deviceObject, FileObject, buffer, offset, length, IoStatusBlock, IrpFlags);
}


void *AllocateMemoryWithTimeout (size_t size, int retryDelay, int timeout)
{
	LARGE_INTEGER waitInterval;
	waitInterval.QuadPart = retryDelay * -10000;
	ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
	ASSERT (retryDelay > 0 && retryDelay <= timeout);

	while (TRUE)
	{
		void *memory = TCalloc (size);
		if (memory)
			return memory;

		timeout -= retryDelay;
		if (timeout <= 0)
			break;

		KeDelayExecutionThread (KernelMode, FALSE, &waitInterval);
	}

	return NULL;
}


NTSTATUS TCStartThread (PKSTART_ROUTINE threadProc, PVOID threadArg, PKTHREAD *kThread)
{
	return TCStartThreadInProcess (threadProc, threadArg, kThread, NULL);
}


NTSTATUS TCStartThreadInProcess (PKSTART_ROUTINE threadProc, PVOID threadArg, PKTHREAD *kThread, PEPROCESS process)
{
	NTSTATUS status;
	HANDLE threadHandle;
	HANDLE processHandle = NULL;
	OBJECT_ATTRIBUTES threadObjAttributes;
	if (process)
	{
		status = ObOpenObjectByPointer (process, OBJ_KERNEL_HANDLE, NULL, 0, NULL, KernelMode, &processHandle);
		if (!NT_SUCCESS (status))
			return status;
	}

	InitializeObjectAttributes (&threadObjAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	
	status = PsCreateSystemThread (&threadHandle, THREAD_ALL_ACCESS, &threadObjAttributes, processHandle, NULL, threadProc, threadArg);
	if (!NT_SUCCESS (status))
		return status;

	status = ObReferenceObjectByHandle (threadHandle, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID *) kThread, NULL);
	if (!NT_SUCCESS (status))
	{
		ZwClose (threadHandle);
		*kThread = NULL;
		return status;
	}

	if (processHandle)
		ZwClose (processHandle);

	ZwClose (threadHandle);
	return STATUS_SUCCESS;
}


void TCStopThread (PKTHREAD kThread, PKEVENT wakeUpEvent)
{
	if (wakeUpEvent)
		KeSetEvent (wakeUpEvent, 0, FALSE);

	KdPrint(("WaitForSingleObject IRP kThread\n"));
	KeWaitForSingleObject (kThread, Executive, KernelMode, FALSE, NULL);
	KdPrint(("After WaitForSingleObject IRP kThread\n"));
	ObDereferenceObject (kThread);
}


void TCStopThreadBySemaphore (PKTHREAD kThread, PKSEMAPHORE wakeUpEvent)
{
	if (wakeUpEvent){
		KeReleaseSemaphore (wakeUpEvent,
					0,
					1,
					TRUE);
	}
	//KeSetEvent (wakeUpEvent, 0, FALSE);
	KeWaitForSingleObject (kThread, Executive, KernelMode, FALSE, NULL);
	ObDereferenceObject (kThread);
}

// Suspend current thread for a number of milliseconds
void TCSleep (int milliSeconds)
{
	PKTIMER timer = (PKTIMER) TCalloc (sizeof (KTIMER));
	LARGE_INTEGER duetime;
	if (!timer)
		return;

	duetime.QuadPart = (__int64) milliSeconds * -10000;
	KeInitializeTimerEx(timer, NotificationTimer);
	KeSetTimerEx(timer, duetime, 0, NULL);

	KeWaitForSingleObject (timer, Executive, KernelMode, FALSE, NULL);

	TCfree (timer);
}

//
//// Checks if two regions overlap (borders are parts of regions)
//BOOL RegionsOverlap (unsigned __int64 start1, unsigned __int64 end1, unsigned __int64 start2, unsigned __int64 end2)
//{
//	return (start1 < start2) ? (end1 >= start2) : (start1 <= end2);
//}
//
//
//void GetIntersection (uint64 start1, uint32 length1, uint64 start2, uint64 end2, uint64 *intersectStart, uint32 *intersectLength)
//{
//	uint64 end1 = start1 + length1 - 1;
//	uint64 intersectEnd = (end1 <= end2) ? end1 : end2;
//
//	*intersectStart = (start1 >= start2) ? start1 : start2;
//	*intersectLength = (uint32) ((*intersectStart > intersectEnd) ? 0 : intersectEnd + 1 - *intersectStart);
//	
//	if (*intersectLength == 0)
//		*intersectStart = start1;
//}

static NTSTATUS
IoCompletionRoutine(
	IN PDEVICE_OBJECT  DeviceObject,
	IN PIRP  Irp,
	IN PVOID  Context
	)
{
	//DbgPrint(("IoCompletionRoutine!\n"));
	*Irp->UserIosb = Irp->IoStatus;

	if (Irp->UserEvent)
		KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);

	if (Irp->MdlAddress)
	{
		IoFreeMdl(Irp->MdlAddress);
		Irp->MdlAddress = NULL;
	}

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
DirectIrpFileRead(
	IN PDEVICE_OBJECT Device,
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER ByteOffset OPTIONAL,
	IN ULONG Length,
	IN ULONG IrpFlags,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatusBlock
	)
{
	NTSTATUS status;
	KEVENT Event;
	PIRP irp;
	PIO_STACK_LOCATION irpSp;
	PDEVICE_OBJECT deviceObject = Device;

	if (ByteOffset == NULL)
	{
		if (!(FileObject->Flags & FO_SYNCHRONOUS_IO))
			return STATUS_INVALID_PARAMETER;

		ByteOffset = &FileObject->CurrentByteOffset;
	}

	if (FileObject->Vpb == 0 || FileObject->Vpb->RealDevice == NULL)
		return STATUS_UNSUCCESSFUL;

	if( deviceObject == NULL )
	{
		deviceObject = FileObject->Vpb->DeviceObject;
	}

	irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

	if (irp == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	irp->MdlAddress = IoAllocateMdl(Buffer, Length, FALSE, TRUE, NULL);

	if (irp->MdlAddress == NULL)
	{
		IoFreeIrp(irp);
		return STATUS_INSUFFICIENT_RESOURCES;;
	}

	MmBuildMdlForNonPagedPool(irp->MdlAddress);

	irp->Flags = IrpFlags;
	irp->RequestorMode = KernelMode;
	irp->UserIosb = IoStatusBlock;
	irp->UserEvent = &Event;
	irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
	irp->Tail.Overlay.OriginalFileObject = FileObject;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_READ;
	irpSp->MinorFunction = IRP_MN_NORMAL;
	irpSp->DeviceObject = deviceObject;
	irpSp->FileObject = FileObject;
	irpSp->Parameters.Read.Length = Length;
	irpSp->Parameters.Read.ByteOffset = *ByteOffset;

	KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

	IoSetCompletionRoutine(irp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);
	
	status = IoCallDriver(deviceObject, irp);

	if (status == STATUS_PENDING)
	{
		status = KeWaitForSingleObject(&Event, Executive, KernelMode, TRUE, NULL);
	}

	IoFreeIrp(irp);
	//IoCompleteRequest( irp, IO_NO_INCREMENT );

	return status;
}
