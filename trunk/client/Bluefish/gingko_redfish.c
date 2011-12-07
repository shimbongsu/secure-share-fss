///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: Redfish.cpp
/// Project          : Redfish
/// Date of creation : 2009-10-26
/// Author(s)        : 
///
/// Purpose          : <description>
///
/// Revisions:
///  0000 [2009-10-26] Initial revision.
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#include <ntddk.h>
#include <tdikrnl.h>
#include <string.h>
#include "gingko_redfish.h"


/* device objects for: */
PDEVICE_OBJECT	g_tcpfltobj = NULL;		// \Device\Tcp
PDEVICE_OBJECT	g_udpfltobj = NULL;		// \Device\Udp
PDEVICE_OBJECT	g_ipfltobj = NULL;		// \Device\RawIp
//PDEVICE_OBJECT	g_devcontrol = NULL;	// control device (exclusive access only!)
//PDEVICE_OBJECT	g_devnfo = NULL;		// information device 
PDEVICE_OBJECT  g_tcpoldobj = NULL;
PDEVICE_OBJECT  g_udpoldobj = NULL;
PDEVICE_OBJECT  g_ipoldobj = NULL;

KSPIN_LOCK g_traffic_guard;
NTSTATUS tdi_dispatch_complete(PDEVICE_OBJECT devobj, PIRP irp, int filter, PIO_COMPLETION_ROUTINE cr, PVOID context);
NTSTATUS tdi_skip_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);

///Stub from gingko_interact.c
ULONG GetPermissionUnderDecryptionByProcessId( ULONG ProcessId, KIRQL *irql );

#define ENTRY(code, fn)	{code, fn}
#define LAST_ENTRY		{0, NULL}

struct tdi_ioctl g_tdi_ioctls[] = {
	ENTRY(TDI_ASSOCIATE_ADDRESS,	tdi_disp_stub),
	ENTRY(TDI_CONNECT,				tdi_disp_stub),
	ENTRY(TDI_DISASSOCIATE_ADDRESS,	tdi_disassociate_address),
	ENTRY(TDI_SET_EVENT_HANDLER,	tdi_set_event_handler),
	ENTRY(TDI_SEND_DATAGRAM,		tdi_disp_stub),
	ENTRY(TDI_RECEIVE_DATAGRAM,		tdi_receive_datagram),
	ENTRY(TDI_DISCONNECT,			tdi_disconnect),
	ENTRY(TDI_SEND,					tdi_disp_stub),
	ENTRY(TDI_RECEIVE,				tdi_receive),
#if 1		// for now only deny stubs for security reasons
	ENTRY(TDI_ACCEPT,				tdi_disp_stub),
	ENTRY(TDI_LISTEN,				tdi_disp_stub),
#endif
	LAST_ENTRY
};

//struct tdi_ioctl g_tdi_ioctls[] = {
//	ENTRY(TDI_ASSOCIATE_ADDRESS,	tdi_associate_address),
//	ENTRY(TDI_CONNECT,				tdi_connect),
//	ENTRY(TDI_DISASSOCIATE_ADDRESS,	tdi_disassociate_address),
//	ENTRY(TDI_SET_EVENT_HANDLER,	tdi_set_event_handler),
//	ENTRY(TDI_SEND_DATAGRAM,		tdi_send_datagram),
//	ENTRY(TDI_RECEIVE_DATAGRAM,		tdi_receive_datagram),
//	ENTRY(TDI_DISCONNECT,			tdi_disconnect),
//	ENTRY(TDI_SEND,					tdi_send),
//	ENTRY(TDI_RECEIVE,				tdi_receive),
//#if 1		// for now only deny stubs for security reasons
//	ENTRY(TDI_ACCEPT,				tdi_deny_stub),
//	ENTRY(TDI_LISTEN,				tdi_deny_stub),
//#endif
//	LAST_ENTRY
//};
//
/* initialization */
NTSTATUS RedfishDriverEntry(IN PDRIVER_OBJECT theDriverObject,
            IN PUNICODE_STRING theRegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;
	int i;
	UNICODE_STRING name, linkname;

	KeInitializeSpinLock(&g_traffic_guard);

	//for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	//{
	//	theDriverObject->MajorFunction[i] = NULL; //DeviceDispatch;
	//}

	// register UnLoad procedure
	//theDriverObject->DriverUnload = OnUnload;

	/* create control device and symbolic link */

	//RtlInitUnicodeString(&name, L"\\Device\\redfishfw");

	//status = IoCreateDevice(theDriverObject,
	//						0,
	//						&name,
	//						0,
	//						0,
	//						TRUE,		// exclusive!
	//						&g_devcontrol);

	//if (status != STATUS_SUCCESS) {
	//	KdPrint(("[tdi_fw] DriverEntry: IoCreateDevice(control): 0x%x!\n", status));
	//	goto done;
	//}

	//RtlInitUnicodeString(&linkname, L"\\??\\redfishfw");

	//status = IoCreateSymbolicLink(&linkname, &name);
	//if (status != STATUS_SUCCESS) {
	//	KdPrint(("[tdi_fw] DriverEntry: IoCreateSymbolicLink: 0x%x!\n", status));
	//	goto done;
	//}

	//RtlInitUnicodeString(&name, L"\\Device\\redfishfw_nfo");

	//status = IoCreateDevice(theDriverObject,
	//						0,
	//						&name,
	//						0,
	//						0,
	//						FALSE,		// not exclusive!
	//						&g_devnfo);
	//if (status != STATUS_SUCCESS) {
	//	KdPrint(("[tdi_fw] DriverEntry: IoCreateDevice(nfo): 0x%x!\n", status));
	//	goto done;
	//}

	//RtlInitUnicodeString(&linkname, L"\\??\\redfishfw_nfo");

	//status = IoCreateSymbolicLink(&linkname, &name);
	//if (status != STATUS_SUCCESS) {
	//	KdPrint(("[tdi_fw] DriverEntry: IoCreateSymbolicLink: 0x%x!\n", status));
	//	goto done;
	//}


	status = c_n_a_device(theDriverObject, &g_tcpfltobj, &g_tcpoldobj, L"\\Device\\Tcp");
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] DriverEntry: c_n_a_device: 0x%x\n", status));
		goto done;
	}

	status = c_n_a_device(theDriverObject, &g_udpfltobj, &g_udpoldobj, L"\\Device\\Udp");
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] DriverEntry: c_n_a_device: 0x%x\n", status));
		goto done;
	}

	status = c_n_a_device(theDriverObject, &g_ipfltobj, &g_ipoldobj, L"\\Device\\RawIp");
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] DriverEntry: c_n_a_device: 0x%x\n", status));
		goto done;
	}

	status = STATUS_SUCCESS;

done:
	if (status != STATUS_SUCCESS) {
		// cleanup
		RedfishUnload(theDriverObject);
	}

    return status;
}

/* deinitialization */
VOID RedfishUnload(IN PDRIVER_OBJECT DriverObject)
{
	d_n_d_device(DriverObject, g_tcpoldobj, g_tcpfltobj);
	d_n_d_device(DriverObject, g_udpoldobj, g_udpfltobj);
	d_n_d_device(DriverObject, g_ipoldobj, g_ipfltobj);

	// delete control device and symbolic link
	//if (g_devcontrol != NULL) {
	//	UNICODE_STRING linkname;
	//	
	//	RtlInitUnicodeString(&linkname, L"\\??\\redfishfw");
	//	IoDeleteSymbolicLink(&linkname);

	//	IoDeleteDevice(g_devcontrol);
	//}

	//// delete info device and symbolic link
	//if (g_devnfo != NULL) {
	//	UNICODE_STRING linkname;
	//	RtlInitUnicodeString(&linkname, L"\\??\\redfishfw_nfo");
	//	IoDeleteSymbolicLink(&linkname);
	//	IoDeleteDevice(g_devnfo);
	//}
}



/* get original device object by filtered */
PDEVICE_OBJECT
get_original_devobj(PDEVICE_OBJECT flt_devobj, int *proto)
{
	PDEVICE_OBJECT result;
	int ipproto;

	if (flt_devobj == g_tcpfltobj) {
		result = g_tcpoldobj;
		ipproto = IPPROTO_TCP;
	} else if (flt_devobj == g_udpfltobj) {
		result = g_udpoldobj;
		ipproto = IPPROTO_UDP;
	} else if (flt_devobj == g_ipfltobj) {
		result = g_ipoldobj;
		ipproto = IPPROTO_IP;
	} else {
		KdPrint(("[tdi_fw] get_original_devobj: Unknown DeviceObject 0x%x!\n",
			flt_devobj));
		ipproto = IPPROTO_IP;		// what else?
		result = NULL;
	}

	if (result != NULL && proto != NULL)
		*proto = ipproto;

	return result;
}


/* create & attach device */
NTSTATUS
c_n_a_device(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT *fltobj, PDEVICE_OBJECT *oldobj,
			 wchar_t *devname)
{
	NTSTATUS status;
	UNICODE_STRING str;

	/* create filter device */

	status = IoCreateDevice(DriverObject,
							0,
							NULL,
							FILE_DEVICE_UNKNOWN,
							0,
							TRUE,
							fltobj);
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] c_n_a_device: IoCreateDevice(%S): 0x%x\n", devname, status));
		return status;
	}

	(*fltobj)->Flags |= DO_DIRECT_IO;

	RtlInitUnicodeString(&str, devname);
	
	status = IoAttachDevice(*fltobj, &str, oldobj);
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] DriverEntry: IoAttachDevice(%S): 0x%x\n", devname, status));
		return status;
	}

	KdPrint(("[tdi_fw] DriverEntry: %S fileobj: 0x%x\n", devname, *fltobj));

	return STATUS_SUCCESS;
}

/* detach & delete device */
void
d_n_d_device(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT oldobj, PDEVICE_OBJECT fltobj)
{
	/*
	 * Detaching of a filter driver at runtime is a high-risk deal
	 */

#if 1
	// for extremal guys only!
	if (oldobj != NULL && fltobj != NULL) {
		int i;
		for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
			DriverObject->MajorFunction[i] = g_tcpoldobj->DriverObject->MajorFunction[i];
	}
#endif

	if (oldobj != NULL)
		IoDetachDevice(oldobj);

	if (fltobj != NULL)
		IoDeleteDevice(fltobj);
}

/* dispatch */
NTSTATUS RedfishDeviceDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp)
{
	PIO_STACK_LOCATION irps;
	NTSTATUS status;
	

	// sanity check
	if (irp == NULL) {
		KdPrint(("[tdi_fw] DeviceDispatch: !irp\n"));
		return STATUS_SUCCESS;
	}


	
	irps = IoGetCurrentIrpStackLocation(irp);

	if (DeviceObject == g_tcpfltobj || DeviceObject == g_udpfltobj ||
		DeviceObject == g_ipfltobj) {

		/*
		 * This IRP is for filtered device
		 */
		int result;
		CompletionCtx completion;
		KIRQL    irql;		
		memset(&completion, 0, sizeof(CompletionCtx));

		if( IsGingkoServerProcess() || !gGingkoServerStarted )
		{
			///Just passthrough 
			return tdi_dispatch_complete( DeviceObject, irp, FILTER_ALLOW, NULL, NULL );
		}

		// Analyze MajorFunction
		switch (irps->MajorFunction) {
		case IRP_MJ_CREATE:		/* create fileobject */
			result = tdi_disp_stub(irp, irps, &completion); //tdi_create(irp, irps, &completion);
			completion.irql = &irql;
			status = tdi_dispatch_complete(DeviceObject, irp, result,
				completion.routine, completion.context);			
			break;

		case IRP_MJ_DEVICE_CONTROL:
			
			//KdPrint(("[tdi_fw] DeviceDispatch: IRP_MJ_DEVICE_CONTROL, control 0x%x for 0x%08X\n",
			//	irps->Parameters.DeviceIoControl.IoControlCode, irps->FileObject));

			//if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
			//	/*
			//	 * try to convert it to IRP_MJ_INTERNAL_DEVICE_CONTROL
			//	 * (works on PASSIVE_LEVEL only!)
			//	 */
			//	status = STATUS_NOT_IMPLEMENTED; //TdiMapUserRequest(DeviceObject, irp, irps);
			//} else
			//	status = STATUS_NOT_IMPLEMENTED; // set fake status
			
			status = STATUS_NOT_IMPLEMENTED; // set fake status

			if (status != STATUS_SUCCESS) {
				void *buf = (irps->Parameters.DeviceIoControl.IoControlCode == IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER) ?
					irps->Parameters.DeviceIoControl.Type3InputBuffer : NULL;

				result = tdi_disp_stub(irp, irps, &completion);
				// send IRP to original driver
				//status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW, NULL, NULL);
				status = tdi_dispatch_complete(DeviceObject, irp, result, NULL, NULL);

				if (buf != NULL && status == STATUS_SUCCESS) {

					g_TCPSendData = *(TCPSendData_t **)buf;

					KdPrint(("[tdi_fw] DeviceDispatch: IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER: TCPSendData = 0x%x\n",
						g_TCPSendData));

					*(TCPSendData_t **)buf = new_TCPSendData;
				}

				break;
			}

			// don't break! go to internal device control!
		
		case IRP_MJ_INTERNAL_DEVICE_CONTROL: {
			/*
			 * Analyze ioctl for TDI driver
			 */
			int i;

			completion.irql = &irql;

			for (i = 0; g_tdi_ioctls[i].MinorFunction != 0; i++)
				if (g_tdi_ioctls[i].MinorFunction == irps->MinorFunction) {
					
					if (g_tdi_ioctls[i].fn == NULL) {
						// send IRP to original driver
						status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW,
							NULL, NULL);
						break;
					}
					// call dispatch function
					result = g_tdi_ioctls[i].fn(irp, irps, &completion);

					// complete request
					status = tdi_dispatch_complete(DeviceObject, irp, result,
						completion.routine, completion.context);

					break;
				}
	
			// if dispatch function hasn't been found
			if (g_tdi_ioctls[i].MinorFunction == 0) {
				// send IRP to original driver
				status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW, NULL, NULL);
			}

			break;
		}

		case IRP_MJ_CLEANUP:		/* cleanup fileobject */

			//completion.irql = &irql;
			//result = tdi_cleanup(irp, irps, &completion);

			status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW,
				completion.routine, completion.context);
			break;

		case IRP_MJ_CLOSE:

			KdPrint(("[redfish] DeviceDispatch: IRP_MJ_CLOSE fileobj 0x%x\n", irps->FileObject));
			// passthrough IRP
			status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW,
				completion.routine, completion.context);

			break;
		default:
			KdPrint(("[redfish] DeviceDispatch: major 0x%x, minor 0x%x for 0x%x\n",
				irps->MajorFunction, irps->MinorFunction, irps->FileObject));

			// passthrough IRP
			status = tdi_dispatch_complete(DeviceObject, irp, FILTER_ALLOW,
				completion.routine, completion.context);
		}

	} else {

		KdPrint(("[tdi_fw] DeviceDispatch: ioctl for unknown DeviceObject 0x%x\n", DeviceObject));

		status = irp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(irp, IO_NO_INCREMENT);
	}

	return status;
}

/*
 * Completion routines must call this function at the end of their execution
 */
NTSTATUS
tdi_generic_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	KdPrint(("[tdi_fw] tdi_generic_complete: STATUS = 0x%x\n", Irp->IoStatus.Status));

	if (Irp->PendingReturned) {
		KdPrint(("[tdi_fw] tdi_generic_complete: PENDING\n"));
		IoMarkIrpPending(Irp);
	}

	return STATUS_SUCCESS;
}


/*
 * Dispatch routines call this function to complete their processing.
 * They _MUST_ call this function anyway.
 */
NTSTATUS
tdi_dispatch_complete(PDEVICE_OBJECT devobj, PIRP irp, int filter,
					  PIO_COMPLETION_ROUTINE cr, PVOID context)
{
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(irp);
	NTSTATUS status;

	if (filter == FILTER_DENY) {
		
		/*
		 * DENY: complete request with status "Access violation"
		 */

		KdPrint(("[tdi_fw] tdi_dispatch_complete: [DROP!]"
			" major (0x%x), minor 0x%x for devobj 0x%x; fileobj 0x%x\n",
			irps->MajorFunction,
			irps->MinorFunction,
			devobj,
			irps->FileObject));

		if (irp->IoStatus.Status == STATUS_SUCCESS) {
			// change status
			status = irp->IoStatus.Status = STATUS_ACCESS_DENIED;
		} else {
			// set IRP status unchanged
			status = irp->IoStatus.Status;
		}

		IoCompleteRequest (irp, IO_NO_INCREMENT);
		
	} else if (filter == FILTER_ALLOW) {

		/*
		 * ALLOW: pass IRP to the next driver
		 */

		PDEVICE_OBJECT old_devobj = get_original_devobj(devobj, NULL);

		if (old_devobj == NULL) {
			KdPrint(("[tdi_fw] tdi_send_irp_to_old_driver: Unknown DeviceObject 0x%x!\n", devobj));
	
			status = irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			IoCompleteRequest (irp, IO_NO_INCREMENT);
			
			return status;
		}


		//KdPrint(("[tdi_fw] tdi_dispatch_complete: [ALLOW.]"
		//	" major 0x%x, minor 0x%x for devobj 0x%x; fileobj 0x%x\n",
		//	irps->MajorFunction,
		//	irps->MinorFunction,
		//	devobj,
		//	irps->FileObject));

		if (cr == NULL || irp->CurrentLocation <= 1) {
			/*
			 * we use _THIS_ way of sending IRP to old driver
			 * a) to avoid NO_MORE_STACK_LOCATIONS
			 * b) and if we haven't our completions - no need to copy stack locations!
			 */

			// stay on this location after IoCallDriver
			IoSkipCurrentIrpStackLocation(irp);

			if (cr != NULL) {
				/*
				 * set completion routine (this way is slow)
				 */

				// save old completion routine and context
				TDI_SKIP_CTX *ctx = (TDI_SKIP_CTX *)ExAllocatePoolWithTag( NonPagedPool,sizeof(TDI_SKIP_CTX), MEM_TAG );
				if (ctx == NULL) {
					KdPrint(("[tdi_fw] tdi_send_irp_to_old_driver: malloc_np\n"));
					
					status = irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
					IoCompleteRequest(irp, IO_NO_INCREMENT);
					
					return status;
				}

				ctx->old_cr = irps->CompletionRoutine;
				ctx->old_context = irps->Context;
				ctx->new_cr = cr;
				ctx->new_context = context;
				ctx->fileobj = irps->FileObject;
				ctx->new_devobj = devobj;

				ctx->old_control = irps->Control;

				IoSetCompletionRoutine(irp, tdi_skip_complete, ctx, TRUE, TRUE, TRUE);
			}
	
		} else {
			PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(irp),
				next_irps = IoGetNextIrpStackLocation(irp);
			
			memcpy(next_irps, irps, sizeof(*irps));

			if (cr != NULL) {
				/*
				 * this way for completion is more quicker than used above
				 */

				IoSetCompletionRoutine(irp, cr, context, TRUE, TRUE, TRUE);
			} else
				IoSetCompletionRoutine(irp, tdi_generic_complete, NULL, TRUE, TRUE, TRUE);
		}

		/* call original driver */

		status = IoCallDriver(old_devobj, irp);

	} else {	/* FILTER_UNKNOWN */

		/*
		 * UNKNOWN: just complete the request
		 */

		status = irp->IoStatus.Status = STATUS_SUCCESS;	// ???
		IoCompleteRequest (irp, IO_NO_INCREMENT);
	}

	return status;
}

/*
 * completion routine for case if we use IoSkipCurrentIrpStackLocation way
 * or we USE_TDI_HOOKING
 */
NTSTATUS
tdi_skip_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	TDI_SKIP_CTX *ctx = (TDI_SKIP_CTX *)Context;
	NTSTATUS status;
	PIO_STACK_LOCATION irps;

	if (Irp->IoStatus.Status != STATUS_SUCCESS)
	{
		KdPrint(("[tdi_fw] tdi_skip_complete: status 0x%x\n", Irp->IoStatus.Status));
	}

	// restore IRP for using in our completion

	Irp->CurrentLocation--;
	Irp->Tail.Overlay.CurrentStackLocation--;

	irps = IoGetCurrentIrpStackLocation(Irp);

	//KdPrint(("[tdi_fw] tdi_skip_complete: DeviceObject = 0x%x; FileObject = 0x%x\n",
	//	DeviceObject, irps->FileObject));

	DeviceObject = irps->DeviceObject;

	if (ctx->new_cr != NULL) {
		// restore fileobject (it's NULL)
		irps->FileObject = ctx->fileobj;
		// set new device object in irps
		irps->DeviceObject = ctx->new_devobj;
		
		// call new completion 
		status = ctx->new_cr(ctx->new_devobj, Irp, ctx->new_context);

	} else
		status = STATUS_SUCCESS;

	/* patch IRP back */

	// restore routine and context (and even control!)
	irps->CompletionRoutine = ctx->old_cr;
	irps->Context = ctx->old_context;
	irps->Control = ctx->old_control;

	// restore device object
	irps->DeviceObject = DeviceObject;

	Irp->CurrentLocation++;
	Irp->Tail.Overlay.CurrentStackLocation++;

	if (ctx->old_cr != NULL) {

		if (status != STATUS_MORE_PROCESSING_REQUIRED) {
			// call old completion (see the old control)
			BOOLEAN b_call = FALSE;

			if (Irp->Cancel) {
				// cancel
				if (ctx->old_control & SL_INVOKE_ON_CANCEL)
					b_call = TRUE;
			} else {
				if (Irp->IoStatus.Status >= STATUS_SUCCESS) {
					// success
					if (ctx->old_control & SL_INVOKE_ON_SUCCESS)
						b_call = TRUE;
				} else {
					// error
					if (ctx->old_control & SL_INVOKE_ON_ERROR)
						b_call = TRUE;
				}
			}

			if (b_call)
				status = ctx->old_cr(DeviceObject, Irp, ctx->old_context);
		
		} else {

			/*
			 * patch IRP to set IoManager to call completion next time
			 */

			// restore Control
			irps->Control = ctx->old_control;

		}
	}

	ExFreePoolWithTag(ctx, MEM_TAG);

	return status;
}


int
tdi_send(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	TDI_REQUEST_KERNEL_SEND *param = (TDI_REQUEST_KERNEL_SEND *)(&irps->Parameters);
	struct ot_entry *ote_conn;
	KIRQL irql;

	KdPrint(("[tdi_fw] tdi_send: connobj: 0x%x; SendLength: %u; SendFlags: 0x%x\n",
		irps->FileObject, param->SendLength, param->SendFlags));

	//ote_conn = ot_find_fileobj(irps->FileObject, &irql);
	//if (ote_conn != NULL) {
	//	ULONG bytes = param->SendLength;

	//	ote_conn->bytes_out += bytes;

	//	// traffic stats
	//	KeAcquireSpinLockAtDpcLevel(&g_traffic_guard);
	//	
	//	g_traffic[TRAFFIC_TOTAL_OUT] += bytes;
	//	
	//	if (ote_conn->log_disconnect)
	//		g_traffic[TRAFFIC_COUNTED_OUT] += bytes;
	//	
	//	KeReleaseSpinLockFromDpcLevel(&g_traffic_guard);

	//	KeReleaseSpinLock(&g_ot_hash_guard, irql);
	//}

	// TODO: process TDI_SEND_AND_DISCONNECT flag (used by IIS for example)

	return FILTER_ALLOW;
}


NTSTATUS
new_TCPSendData(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp)
{
	CompletionCtx completion;
	int result;

	KdPrint(("[tdi_fw] new_TCPSendData\n"));

#if 1
	memset(&completion, 0, sizeof(CompletionCtx));

	result = tdi_send(Irp, IrpSp, &completion);

	// complete request
	return tdi_dispatch_complete(IrpSp->DeviceObject, Irp, result,
		completion.routine, completion.context);
#else
	return g_TCPSendData(Irp, IrpSp);
#endif
}


/* this completion routine queries address and port from address object */
NTSTATUS
tdi_create_addrobj_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(Irp);
	PIRP query_irp = (PIRP)Context;
	PDEVICE_OBJECT devobj;
	PMDL mdl = NULL;

	KdPrint(("[tdi_fw] tdi_create_addrobj_complete: devobj 0x%x; addrobj 0x%x\n",
		DeviceObject, irps->FileObject));

	if (Irp->IoStatus.Status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] tdi_create_addrobj_complete: status 0x%x\n", Irp->IoStatus.Status));
		status = Irp->IoStatus.Status;
	}

	return tdi_generic_complete(DeviceObject, Irp, Context);
}


int
tdi_create(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	NTSTATUS status;
	FILE_FULL_EA_INFORMATION *ea = (FILE_FULL_EA_INFORMATION *)irp->AssociatedIrp.SystemBuffer;

	/* pid resolving stuff: a good place for it (PASSIVE level, begin of working with TDI-objects) */
	ULONG pid = (ULONG)PsGetCurrentProcessId();

	if (ea != NULL) {
		/*
		 * We have FILE_FULL_EA_INFORMATION
		 */

		PDEVICE_OBJECT devobj;
		int ipproto;
		
		devobj = get_original_devobj(irps->DeviceObject, &ipproto);
		if (devobj == NULL) {
			KdPrint(("[tdi_fw] tdi_create: unknown device object 0x%x!\n", irps->DeviceObject));
			return FILTER_DENY;
		}
		// NOTE: for RawIp you can extract protocol number from irps->FileObject->FileName

		if (ea->EaNameLength == TDI_TRANSPORT_ADDRESS_LENGTH &&
			memcmp(ea->EaName, TdiTransportAddress, TDI_TRANSPORT_ADDRESS_LENGTH) == 0) {

			PIRP query_irp;

			/*
			 * This is creation of address object
			 */

			KdPrint(("[tdi_fw] tdi_create: devobj 0x%x; addrobj 0x%x\n",
				irps->DeviceObject,
				irps->FileObject));

			//status = ot_add_fileobj(irps->DeviceObject, irps->FileObject, FILEOBJ_ADDROBJ, ipproto, NULL);
			//if (status != STATUS_SUCCESS) {
			//	KdPrint(("[tdi_fw] tdi_create: ot_add_fileobj: 0x%x\n", status));
			//	return FILTER_DENY;
			//}

			// while we're on PASSIVE_LEVEL build control IRP for completion
			query_irp = TdiBuildInternalDeviceControlIrp(TDI_QUERY_INFORMATION,
				devobj, irps->FileObject, NULL, NULL);
			if (query_irp == NULL) {
				KdPrint(("[tdi_fw] tdi_create: TdiBuildInternalDeviceControlIrp\n"));
				return FILTER_DENY;
			}

			/* set IRP completion & context for completion */

			completion->routine = tdi_create_addrobj_complete;
			completion->context = query_irp;

		} else if (ea->EaNameLength == TDI_CONNECTION_CONTEXT_LENGTH &&
			memcmp(ea->EaName, TdiConnectionContext, TDI_CONNECTION_CONTEXT_LENGTH) == 0) {
			
			/*
			 * This is creation of connection object
			 */

			CONNECTION_CONTEXT conn_ctx = *(CONNECTION_CONTEXT *)
				(ea->EaName + ea->EaNameLength + 1);

			KdPrint(("[tdi_fw] tdi_create: devobj 0x%x; connobj 0x%x; conn_ctx 0x%x\n",
				irps->DeviceObject,
				irps->FileObject,
				conn_ctx));

			//status = ot_add_fileobj(irps->DeviceObject, irps->FileObject,
			//	FILEOBJ_CONNOBJ, ipproto, conn_ctx);

			//if (status != STATUS_SUCCESS) {
			//	KdPrint(("[tdi_fw] tdi_create: ot_add_fileobj: 0x%x\n", status));
			//	return FILTER_DENY;
			//}
			return FILTER_DENY;
		}
	
	} else {
		/*
		 * This is creation of control object
		 */
		
		KdPrint(("[tdi_fw] tdi_create(pid:%u): devobj 0x%x; Control Object: 0x%x\n",
			pid, irps->DeviceObject, irps->FileObject));
	}

	return FILTER_ALLOW;
}

int
tdi_cleanup(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	NTSTATUS status;
	int type;

	// success anyway
	return FILTER_ALLOW;
}

//----------------------------------------------------------------------------

/*
 * TDI_ASSOCIATE_ADDRESS handler
 *
 * With help of this routine we can get address object by connection object
 * and get connection object by connection context and address object
 */
int
tdi_associate_address(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	HANDLE addr_handle = ((TDI_REQUEST_KERNEL_ASSOCIATE *)(&irps->Parameters))->AddressHandle;
	PFILE_OBJECT addrobj = NULL;
	NTSTATUS status;
	struct ot_entry *ote_conn = NULL;
	KIRQL irql;
	int result = FILTER_DENY;

	KdPrint(("[tdi_fw] tdi_associate_address: devobj 0x%x; connobj 0x%x\n",
		irps->DeviceObject, irps->FileObject));

	status = ObReferenceObjectByHandle(addr_handle, GENERIC_READ, NULL, KernelMode, &addrobj, NULL);
	if (status != STATUS_SUCCESS) {
		KdPrint(("[tdi_fw] tdi_associate_address: ObReferenceObjectByHandle: 0x%x\n", status));
		goto done;
	}

	KdPrint(("[tdi_fw] tdi_associate_address: connobj = 0x%x ---> addrobj = 0x%x\n",
		irps->FileObject, addrobj));


	result = FILTER_ALLOW;
done:
	if (addrobj != NULL)
		ObDereferenceObject(addrobj);

	return result;
}

//----------------------------------------------------------------------------

/*
 * TDI_DISASSOCIATE_ADDRESS handler
 */
int
tdi_disassociate_address(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	struct ot_entry *ote_conn = NULL;
	KIRQL irql;
	NTSTATUS status;

	KdPrint(("[tdi_fw] tdi_disassociate_address: connobj 0x%x\n", irps->FileObject));

	// success anyway
	return FILTER_ALLOW;
}




NTSTATUS
tdi_receive_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(Irp);
	struct ot_entry *ote_conn;
	KIRQL irql;

	KdPrint(("[tdi_fw] tdi_receive_complete: connobj: 0x%x; status: 0x%x; received: %u\n",
		irps->FileObject, Irp->IoStatus.Status, Irp->IoStatus.Information));

	return tdi_generic_complete(DeviceObject, Irp, Context);
}


int
tdi_receive(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	TDI_REQUEST_KERNEL_RECEIVE *param = (TDI_REQUEST_KERNEL_RECEIVE *)(&irps->Parameters);

	KdPrint(("[tdi_fw] tdi_receive: connobj: 0x%x; ReceiveLength: %u; ReceiveFlags: 0x%x\n",
		irps->FileObject, param->ReceiveLength, param->ReceiveFlags));

	if (!(param->ReceiveFlags & TDI_RECEIVE_PEEK)) {
		completion->routine = tdi_receive_complete;
	}

	return FILTER_ALLOW;
}





NTSTATUS
tdi_disconnect_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(Irp);
	int state = (int)Context;

	KdPrint(("[tdi_fw] tdi_disconnect_complete: connobj 0x%x; status: 0x%x\n",
		irps->FileObject, Irp->IoStatus.Status));

	if (Irp->IoStatus.Status == STATUS_SUCCESS) {
		
		// update TCP state table
		
		//if (state == TCP_STATE_FIN_WAIT1)
		//	set_tcp_conn_state(irps->FileObject, TCP_STATE_FIN_WAIT2);
		//else if (state == TCP_STATE_LAST_ACK)
		//	del_tcp_conn(irps->FileObject, TRUE);
		//else if (state == TCP_STATE_CLOSED)
		//	del_tcp_conn(irps->FileObject, TRUE);
		//else
		//	KdPrint(("[tdi_fw] tdi_disconnect_complete: weird conn state: %d\n", state));
	}

	return tdi_generic_complete(DeviceObject, Irp, Context);
}


int
tdi_disconnect(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	TDI_REQUEST_KERNEL_DISCONNECT *param = (TDI_REQUEST_KERNEL_DISCONNECT *)(&irps->Parameters);

	KdPrint(("[tdi_fw] tdi_disconnect: connobj 0x%x (flags: 0x%x)\n",
		irps->FileObject, param->RequestFlags));

	if (param->RequestFlags & TDI_DISCONNECT_RELEASE) {
		//int state = get_tcp_conn_state_by_obj(irps->FileObject), new_state;

		//if (state == TCP_STATE_ESTABLISHED_IN || state == TCP_STATE_ESTABLISHED_OUT)
		//	new_state = TCP_STATE_FIN_WAIT1;
		//else if (state == TCP_STATE_CLOSE_WAIT)
		//	new_state = TCP_STATE_LAST_ACK;
		//else
		//	KdPrint(("[tdi_fw] tdi_disconnect: weird conn state: %d\n", state));

		//set_tcp_conn_state(irps->FileObject, new_state);

		completion->routine = tdi_disconnect_complete;
		completion->context = (PVOID)0;

	} else {

		// set TCP_STATE_CLOSED and delete object in completion

		//set_tcp_conn_state(irps->FileObject, TCP_STATE_CLOSED);

		completion->routine = tdi_disconnect_complete;
		completion->context = (PVOID)0;

	}

	return FILTER_ALLOW;
}




NTSTATUS
tdi_connect_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	NTSTATUS status;
	struct flt_request *request = (struct flt_request *)Context;
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(Irp);

	KdPrint(("[tdi_fw] tdi_connect_complete: status 0x%x\n", Irp->IoStatus.Status));

	return tdi_generic_complete(DeviceObject, Irp, Context);
}

//----------------------------------------------------------------------------

/*
 * TDI_CONNECT handler
 */

int
tdi_connect(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	PTDI_REQUEST_KERNEL_CONNECT param = (PTDI_REQUEST_KERNEL_CONNECT)(&irps->Parameters);
	TA_ADDRESS *remote_addr = ((TRANSPORT_ADDRESS *)(param->RequestConnectionInformation->RemoteAddress))->Address;
	PFILE_OBJECT addrobj;
	NTSTATUS status;
	TA_ADDRESS *local_addr;
	int result = FILTER_DENY, ipproto;
	KIRQL irql;

	result = tdi_disp_stub( irp, irps, completion );
	if (result != FILTER_ALLOW) {
		irp->IoStatus.Status = STATUS_REMOTE_NOT_LISTENING;	// set fake status
	}

	return result;
}


static NTSTATUS tdi_receive_datagram_complete(
	IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);


//----------------------------------------------------------------------------

/*
 * TDI_SEND_DATAGRAM handler
 */

int
tdi_send_datagram(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	TDI_REQUEST_KERNEL_SENDDG *param = (TDI_REQUEST_KERNEL_SENDDG *)(&irps->Parameters);
	TA_ADDRESS *local_addr, *remote_addr;
	NTSTATUS status;


	return FILTER_ALLOW;
}

//----------------------------------------------------------------------------

/*
 * TDI_RECEIVE_DATAGRAM handler
 */

int
tdi_receive_datagram(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	KdPrint(("[tdi_fw] tdi_receive_datagram: addrobj 0x%x\n", irps->FileObject));

	completion->routine = tdi_receive_datagram_complete;

	return FILTER_ALLOW;
}

NTSTATUS
tdi_receive_datagram_complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	PIO_STACK_LOCATION irps = IoGetCurrentIrpStackLocation(Irp);
	TDI_REQUEST_KERNEL_RECEIVEDG *param = (TDI_REQUEST_KERNEL_RECEIVEDG *)(&irps->Parameters);
	PFILE_OBJECT addrobj = irps->FileObject;

	return tdi_generic_complete(DeviceObject, Irp, Context);
}



int
tdi_set_event_handler(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	PTDI_REQUEST_KERNEL_SET_EVENT r = (PTDI_REQUEST_KERNEL_SET_EVENT)&irps->Parameters;
	NTSTATUS status;

	//KdPrint(("[tdi_fw] tdi_set_event_handler: [%s] devobj 0x%x; addrobj 0x%x; EventType: %d\n",
	//	r->EventHandler ? "(+)ADD" : "(-)REMOVE",
	//	irps->DeviceObject,
	//	irps->FileObject,
	//	r->EventType));


	return FILTER_ALLOW;
}


/*
 * deny stub for dispatch table
 */
int
tdi_deny_stub(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	KdPrint(("[tdi_fw] tdi_deny_stub!\n"));
	return FILTER_DENY;
}


/*
 * deny stub for dispatch table
 */
int
tdi_disp_stub(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion)
{
	ULONG dwProcessId = (ULONG) PsGetCurrentProcessId();
	ULONG Permission = GetPermissionUnderDecryptionByProcessId( dwProcessId, completion ? completion->irql : NULL );
	if( ( Permission & 0x8FFFFFF0 ) > 0x80000000 )
	{
		KdPrint(("[redfish] tdi_disp_stub: Current ProcessId = 0x%08x, Permission=0x%08x!\n", dwProcessId, Permission ));
		return FILTER_DENY;
	}
	else
	{
		return FILTER_ALLOW;
	}
}
