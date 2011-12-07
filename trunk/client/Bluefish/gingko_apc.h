#ifndef __GINGKO_APC_H__
#define __GINGKO_APC_H__

#include <ntifs.h>
#include "gingko_lib.h"


typedef VOID (NTAPI *PAPCFUNC)(  ULONG_PTR dwParam );

#define POOL_QUOTA_FAIL_INSTEAD_OF_RAISE 8

typedef enum _KAPC_ENVIRONMENT {
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment
} KAPC_ENVIRONMENT;

ULONG QueueUserAPC(
	PAPCFUNC pfnAPC, // pointer to APC function
	HANDLE hThread,  // handle to the thread
	ULONG_PTR dwData     // argument for the APC function
);

VOID
KeInitializeApc (
	IN PRKAPC Apc,
    IN PRKTHREAD Thread,
	IN KAPC_ENVIRONMENT Environment,
	IN PKKERNEL_ROUTINE KernelRoutine,
	IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
	IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
	IN KPROCESSOR_MODE ApcMode OPTIONAL,
	IN PVOID NormalContext OPTIONAL
);

BOOLEAN
KeInsertQueueApc (
	IN PRKAPC Apc,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2,
	IN KPRIORITY Increment
	);

typedef VOID (NTAPI *PGINGKO_APCFUN)(  ULONG_PTR dwParam );

#ifndef LPVOID 
#define LPVOID PVOID
#endif

typedef VOID (*PPS_APC_ROUTINE)(
	 LPVOID lpApcArgument1,
	 LPVOID lpApcArgument2,
	 LPVOID lpApcArgument3
);

NTSYSAPI
NTSTATUS
NTAPI
NtQueueApcThread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    );

VOID
BaseDispatchAPC(
	 LPVOID lpApcArgument1,
	 LPVOID lpApcArgument2,
	 LPVOID lpApcArgument3
);

void GingkoReadRoutine(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PVOID pContext );

VOID GingkoThreadReadRoutine(
	IN PVOID pContext
	);

NTSTATUS GingkoQueueUserAPC( PGINGKO_APCFUN GingkoAPCFunc, HANDLE hThread, ULONG_PTR dwData ); 

NTSTATUS GingkoInitializeApc( PKEVENT pKevent, PGINGKO_APC_PARAMETER pContext );

NTSTATUS GingkoSetApc(ULONG_PTR UserRoutine, IN PVOID pContext, IN PVOID pEvent );

NTSTATUS GingkoResetApc();

#endif
