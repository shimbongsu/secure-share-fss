
#include "gingko_apc.h"
#include "gingko_filter.h"


void KernelApcCallback(PKAPC Apc, 
					   PKNORMAL_ROUTINE NormalRoutine, 
					   PVOID NormalContext, 
					   PVOID SystemArgument1, 
					   PVOID SystemArgument2);

//void GingkoReadRoutine(
//	IN PDEVICE_OBJECT DeviceObject, 
//	IN PVOID pContext )
//{
//	PGINGKO_WORKITEM_CONTEXT WorkItemContext = (PGINGKO_WORKITEM_CONTEXT) pContext;
//	PIRP Irp = WorkItemContext->Irp;
//	KEVENT kEvent;
//	
//	KeInitializeEvent( &kEvent, NotificationEvent, FALSE );
//	
//	GingkoSetApc( gGingkoNotification->GingkoDecryptBuffer, 
//		WorkItemContext->Parameter, 
//		&kEvent );
//	
//	KeWaitForSingleObject( &kEvent, Executive, KernelMode, FALSE, NULL );
//
//	IoCompleteRequest( WorkItemContext->Irp, IO_NO_INCREMENT );
//}
//
//void GingkoThreadReadRoutine(
//	IN PVOID pContext )	
//{
//	PGINGKO_WORKITEM_CONTEXT WorkItemContext = (PGINGKO_WORKITEM_CONTEXT) pContext;
//	PIRP Irp = WorkItemContext->Irp;
//	PRKEVENT pEvent = WorkItemContext->pKevent;
//	//KeInitializeEvent( &kEvent, NotificationEvent, FALSE );
//	
//	GingkoSetApc( gGingkoNotification->GingkoDecryptBuffer, 
//		WorkItemContext->Parameter, 
//		pEvent );
//	
//	KeWaitForSingleObject( pEvent, Executive, KernelMode, FALSE, NULL );
//
//	if( Irp != NULL )
//	{
//		IoCompleteRequest( WorkItemContext->Irp, IO_NO_INCREMENT );
//	}
//
//	PsTerminateSystemThread( STATUS_SUCCESS );
//}
//
//NTSTATUS GingkoResetApc()
//{
//	PKTHREAD		pKeThread = NULL;
//	pKeThread = (PKTHREAD)gGingkoNotification->ServerThread;
//	if( pKeThread != NULL )
//	{
//		//*((unsigned char *)pKeThread + 0x164) = 0;
//		*((unsigned char *)pKeThread+0x4a) = 0;
//	}
//
//	return STATUS_SUCCESS;
//}
//
//
//NTSTATUS GingkoSetApc(ULONG_PTR UserRoutine, IN PVOID pContext, IN PVOID pEvent)
//{
//	NTSTATUS        dwStatus = STATUS_SUCCESS;
//	PKAPC           pkApc;
//	BOOLEAN         bBool = FALSE;
//	PKTHREAD		pKeThread = NULL;
//	//
//	pKeThread = (PKTHREAD)gGingkoNotification->ServerThread;
//
//	if( pContext == NULL )
//	{
//		return STATUS_UNSUCCESSFUL;
//	}
//	
//	pkApc = ExAllocatePoolWithTag(
//			NonPagedPool,
//			sizeof(KAPC),
//			GINGKO_POOL_TAG
//			);	
//
//	if ( pkApc == NULL )
//	{
//		KdPrint(("error:ExAllocAtePool\n"));
//		return STATUS_INSUFFICIENT_RESOURCES;
//	}
//
//
//	RtlZeroMemory( pkApc, sizeof(KAPC));
//	///set the USER APC PENDING = 1
//	
//	//*((unsigned char *)pKeThread+0x4a) = 1;
//
//	*((unsigned char *)pKeThread + 0x164) = 1;
//
//	KeInitializeApc(
//		pkApc,
//		pKeThread, //(PKTHREAD)&((((PGINGKO_APC_PARAMETER)pContext)->hThread)),
//		OriginalApcEnvironment,
//		//AttachedApcEnvironment,
//		//CurrentApcEnvironment
//		(PKKERNEL_ROUTINE)KernelApcCallback,
//		NULL,
//		(PKNORMAL_ROUTINE)UserRoutine,//UserApcCAllBAck,   //modified by hAnd - -
//		UserMode,
//		pContext
//	);
//
//	*((unsigned char *)pKeThread+0x4a) = 1;
//
//	KdPrint(("Insert the Function to call Queue. Current Irql level: %d. Dispatch %d.Passive: %d, APC: %d.\n", KeGetCurrentIrql(),DISPATCH_LEVEL, PASSIVE_LEVEL, APC_LEVEL ));
//	bBool = KeInsertQueueApc(pkApc,
//				pEvent,
//				NULL,
//				//IO_NO_INCREMENT
//				0
//				);
//
//	if( *((unsigned char *)pKeThread+0x4a)  == 0 )
//	{
//		KdPrint(("Thead is not alterable.\n"));
//		//*((unsigned char *)pKeThread+0x4a) = 1;
//		*((unsigned char *)pKeThread+0x49) = 1;
//		*((unsigned char *)pKeThread+0x48) = 1;
//	}
//	KdPrint(("Thead is alterable now.\n"));
//	
//	//if( *((unsigned char *)pKeThread+0x4a)  == 0 )
//	//{
//	//	*((unsigned char *)pKeThread + 0x164) = 1;
//	//}
//
//	KdPrint(("KeInsertQueueApc: %x===%08x.\n", bBool, dwStatus));
//
//	if( bBool )
//	{
//		//dwStatus = STATUS_UNSUCCESSFUL;
//	}
//
//	return dwStatus;
//}
//
//
//void KernelApcCallback(PKAPC Apc, 
//					   PKNORMAL_ROUTINE NormalRoutine, 
//					   PVOID pContext, 
//					   PVOID SystemArgument1, 
//					   PVOID SystemArgument2)
//{
//	if( Apc != NULL )
//	{
//		KdPrint(("Free the Apc memory\n"));
//		ExFreePoolWithTag(Apc, GINGKO_POOL_TAG);    // free the kernel memory
//	}
//}
//
