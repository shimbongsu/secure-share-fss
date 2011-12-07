#ifndef __GINGKO_INTERACT_H__
#define __GINGKO_INTERACT_H__

typedef struct {
	LONG_PTR ProcessId;
	BOOLEAN	 Action;
}RequestEncryption;

typedef struct {
	LONG_PTR	FileObject;
	LONG_PTR	RequestKeyEvent;
	ULONG		dwProcessId;
	LONG_PTR	Queue;
	LIST_ENTRY	ListEntry;
} RequestKeyResponseQueue;


//void ClearRequestBuffer();
//
//void SetRequestBuffer(PGINGKO_APC_PARAMETER req);
ULONG GetPermissionUnderDecryptionByProcessId( ULONG ProcessId, KIRQL *irql );

BOOL FindEncryptionIoQueueFromResponseKeyList(LONG_PTR FileObject, LONG_PTR RequestKeyEvent, RequestKeyResponseQueue** pQueue);

NTSTATUS GingkoRetrievPublicKeyRequest(PIRP Irp, PVOID InputBuffer, LONG InputLength,  PVOID OutputBuffer, ULONG OutputLength);

NTSTATUS GingkoPublicKeyResponse(PIRP Irp, PVOID InputBuffer, LONG InputLength,  PVOID OutputBuffer);

NTSTATUS GingkoWaitForDecryptionEvent();

NTSTATUS GingkoSetEventForRequest();

NTSTATUS GingkoSetEncryptionProcess(PIRP Irp, LONG_PTR ProcessId, BOOLEAN Action);



#endif
