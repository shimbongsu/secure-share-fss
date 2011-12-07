/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef __GINGKO_IOQUEUE_H__
#define __GINGKO_IOQUEUE_H__

#include "gingko_defs.h"
#include "gingko_crypto.h"
#include "EncryptionThreadPool.h"


#define TC_ENC_IO_QUEUE_MAX_FRAGMENT_SIZE (256 * 1024)
#define TC_ENC_IO_QUEUE_MEM_ALLOC_RETRY_DELAY 5
#define TC_ENC_IO_QUEUE_MEM_ALLOC_TIMEOUT (60 * 1000)

typedef struct _GINGKO_HEADER GINGKO_HEADER;
typedef struct _GINGKO_OBJECT* PGINGKO_OBJECT;

typedef struct
{
	PDEVICE_OBJECT DeviceObject;
	CRYPTO_INFO *CryptoInfo;
	GINGKO_HEADER *GingkoHeader;
	PGINGKO_OBJECT GingkoObject;
	// File-handle-based IO
	PFILE_OBJECT FileObject;
	HANDLE	     HostFileObject;
	//int64 VirtualDeviceLength;
	SECURITY_CLIENT_CONTEXT *SecurityClientContext;
	// Filter device
	BOOL IsFilterDevice;
	PDEVICE_OBJECT LowerDeviceObject;
	int64 EncryptedAreaStart;
	int64 EncryptedAreaEnd;
	BOOL  RemapEncryptedArea;
	int64 RemappedAreaOffset;
	int64 RemappedAreaDataUnitOffset;
	long  AbstractOffset;
	IO_REMOVE_LOCK RemoveLock;

	// Main tread
	PKTHREAD MainThread;
	LIST_ENTRY MainThreadQueue;
	KSPIN_LOCK MainThreadQueueLock;
	KSEMAPHORE MainThreadQueueNotEmptyEvent;

	// IO thread
	//PKTHREAD	IoThread;
	//LIST_ENTRY	IoThreadQueue;
	//KSPIN_LOCK	IoThreadQueueLock;
	//KEVENT		IoThreadQueueNotEmptyEvent;

	// Completion thread
	//PKTHREAD	CompletionThread;
	//LIST_ENTRY	CompletionThreadQueue;
	//KSPIN_LOCK	CompletionThreadQueueLock;
	//KEVENT		CompletionThreadQueueNotEmptyEvent;

	byte *FragmentBufferA;
	//byte *FragmentBufferB;
	KEVENT FragmentBufferAFreeEvent;
	//KEVENT FragmentBufferBFreeEvent;

	__int64  CurrentOffset;
	LONG	  CurrentLength;
	NTSTATUS CurrentStatus;

	__int64  CurrentWriteOffset;
	LONG	  CurrentWriteLength;
	NTSTATUS CurrentWriteStatus;

	LONG OutstandingIoCount;
	KEVENT NoOutstandingIoEvent;
	
	KEVENT RequestCompletedEvent;

	LIST_ENTRY		ResponseKeyList;
	LIST_ENTRY		RequestKeyList;
	KEVENT			RequestKeyEvent; ///Add to wait for retrieve the key from the server.
	LONG			RequestKeyCount;

	BOOL	ShouldPassthrough;
	
	__int64 TotalBytesRead;
	__int64 TotalBytesWritten;

	BOOL ThreadExitRequested;
	
	BOOL Suspended;
	BOOL SuspendPending;
	BOOL StopPending;
	BOOL IsWrotedFirstBlockToDisk;
	KEVENT QueueResumedEvent;
	ULONG dwProcessId;

}  EncryptedIoQueue;


typedef struct
{
	EncryptedIoQueue *Queue;
	PIRP OriginalIrp;
	BOOL Write;
	BOOL ShouldReleaseMdl;
	ULONG OriginalLength;
	LARGE_INTEGER OriginalOffset;
	LONG OutstandingRequestCount;
	NTSTATUS Status;
} EncryptedIoQueueItem;


typedef struct
{
	EncryptedIoQueueItem *Item;
	BOOL CompleteOriginalIrp;
	LARGE_INTEGER Offset;
	ULONG Length;
	ULONG RequestLength;
	ULONG ReadLength;
	int64 EncryptedOffset;
	ULONG EncryptedLength;
	byte *Data;
	byte *OrigDataBufferFragment;

	LIST_ENTRY ListEntry;
	LIST_ENTRY CompletionListEntry;
} EncryptedIoRequest;



//typedef enum
//{
//	EncryptDataUnitsWork,
//	DecryptDataUnitsWork,
//	DeriveKeyWork
//} EncryptionThreadPoolWorkType;

NTSTATUS TCCompleteDiskIrp (PIRP irp, NTSTATUS status, ULONG_PTR information);
NTSTATUS TCCompleteIrp (PIRP irp, NTSTATUS status, ULONG_PTR information);


void EncryptionThreadPoolBeginKeyDerivation (TC_EVENT *completionEvent, TC_EVENT *noOutstandingWorkItemEvent, LONG *completionFlag, LONG *outstandingWorkItemCount, int pkcs5Prf, char *password, int passwordLength, char *salt, int iterationCount, char *derivedKey);
void EncryptionThreadPoolDoWork (EncryptionThreadPoolWorkType type, byte *data, const UINT64_STRUCT *startUnitNo, TC_LARGEST_COMPILER_UINT unitCount, PCRYPTO_INFO cryptoInfo);
BOOL EncryptionThreadPoolStart ();
void EncryptionThreadPoolStop ();
size_t GetEncryptionThreadCount ();
BOOL IsEncryptionThreadPoolRunning ();


NTSTATUS EncryptedIoQueueAddIrp (EncryptedIoQueue *queue, PIRP irp);
BOOL	 EncryptedIoQueueIsRunning (EncryptedIoQueue *queue);
BOOL	 EncryptedIoQueueIsSuspended (EncryptedIoQueue *queue);
NTSTATUS EncryptedIoQueueResumeFromHold (EncryptedIoQueue *queue);
NTSTATUS EncryptedIoQueueStart (EncryptedIoQueue *queue);
NTSTATUS EncryptedIoQueueStop (EncryptedIoQueue *queue);
BOOLEAN EncryptedIoQueueCleanup (EncryptedIoQueue *queue);
NTSTATUS EncryptedIoQueueHoldWhenIdle (EncryptedIoQueue *queue, int64 timeout);

NTSTATUS TCReadWriteDeviceSync (BOOL write, PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, 
									   PVOID buffer, PLARGE_INTEGER offset, ULONG length,
									   PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags);
NTSTATUS TCReadDevice (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock);
NTSTATUS TCWriteDevice (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject,  PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock);
NTSTATUS TCReadDeviceEx (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags);
NTSTATUS TCWriteDeviceEx (PDEVICE_OBJECT deviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags);

void *AllocateMemoryWithTimeout (size_t size, int retryDelay, int timeout);

NTSTATUS
DirectIrpFileRead(
	IN PDEVICE_OBJECT Device,
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER ByteOffset OPTIONAL,
	IN ULONG Length,
	IN ULONG IrpFlags,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatusBlock
	);
NTSTATUS TCReadDeviceDirect (PDEVICE_OBJECT DeviceObject, PFILE_OBJECT FileObject, PVOID buffer, PLARGE_INTEGER offset, ULONG length, PIO_STATUS_BLOCK IoStatusBlock, ULONG IrpFlags );

NTSTATUS TCStartThread (PKSTART_ROUTINE threadProc, PVOID threadArg, PKTHREAD *kThread);
NTSTATUS TCStartThreadInProcess (PKSTART_ROUTINE threadProc, PVOID threadArg, PKTHREAD *kThread, PEPROCESS process);
void TCStopThread (PKTHREAD kThread, PKEVENT wakeUpEvent);
void TCStopThreadBySemaphore (PKTHREAD kThread, PKSEMAPHORE wakeUpEvent);
void TCSleep (int milliSeconds);

// Checks if two regions overlap (borders are parts of regions)
//BOOL RegionsOverlap (unsigned __int64 start1, unsigned __int64 end1, unsigned __int64 start2, unsigned __int64 end2);
//
//void GetIntersection (uint64 start1, uint32 length1, uint64 start2, uint64 end2, uint64 *intersectStart, uint32 *intersectLength);

#endif // TC_HEADER_DRIVER_ENCRYPTED_IO_QUEUE
