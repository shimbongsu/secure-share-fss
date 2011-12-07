///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: Redfish.h
/// Project          : Redfish
/// Date of creation : <see Redfish.cpp>
/// Author(s)        : <see Redfish.cpp>
///
/// Purpose          : <see Redfish.cpp>
///
/// Revisions:         <see Redfish.cpp>
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __REDFISH_H_VERSION__
#define __REDFISH_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


/* filter result */
enum {
	FILTER_ALLOW = 1,
	FILTER_DENY,
	FILTER_PACKET_LOG,
	FILTER_PACKET_BAD,
	FILTER_DISCONNECT
};

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_UDP             17              /* user datagram protocol */

#define MEM_TAG		'pkTg'

typedef struct __completion {
	PIO_COMPLETION_ROUTINE	routine;
	PVOID					context;
	KIRQL*					irql;
}CompletionCtx;

/* context for tdi_skip_complete */
typedef struct {
    PIO_COMPLETION_ROUTINE	old_cr;			/* old (original) completion routine */
    PVOID					old_context;	/* old (original) parameter for old_cr */
    PIO_COMPLETION_ROUTINE	new_cr;			/* new (replaced) completion routine */
	PVOID					new_context;	/* new (replaced) parameter for new_cr */
	PFILE_OBJECT			fileobj;		/* FileObject from IO_STACK_LOCATION */
	PDEVICE_OBJECT			new_devobj;		/* filter device object */
	UCHAR					old_control;	/* old (original) irps->Control */
} TDI_SKIP_CTX;

typedef int tdi_ioctl_fn_t(PIRP irp, PIO_STACK_LOCATION irps, CompletionCtx *completion);

// helper struct for calling of TDI ioctls
struct tdi_ioctl {
	UCHAR			MinorFunction;
	tdi_ioctl_fn_t	*fn;
};

typedef NTSTATUS  TCPSendData_t(IN PIRP Irp, IN PIO_STACK_LOCATION IrpSp);
static TCPSendData_t *g_TCPSendData = NULL;
static TCPSendData_t new_TCPSendData;

extern struct tdi_ioctl g_tdi_ioctls[];

extern tdi_ioctl_fn_t tdi_create;
extern tdi_ioctl_fn_t tdi_cleanup;
extern tdi_ioctl_fn_t tdi_disp_stub;
extern tdi_ioctl_fn_t tdi_disassociate_address;
extern tdi_ioctl_fn_t tdi_associate_address;
extern tdi_ioctl_fn_t tdi_connect;
extern tdi_ioctl_fn_t tdi_disassociate_address;
extern tdi_ioctl_fn_t tdi_set_event_handler;
extern tdi_ioctl_fn_t tdi_send_datagram;
extern tdi_ioctl_fn_t tdi_receive_datagram;
extern tdi_ioctl_fn_t tdi_disconnect;
extern tdi_ioctl_fn_t tdi_send;
extern tdi_ioctl_fn_t tdi_receive;
extern tdi_ioctl_fn_t tdi_deny_stub;
extern PDEVICE_OBJECT	g_tcpfltobj;		// \Device\Tcp
extern PDEVICE_OBJECT	g_udpfltobj;		// \Device\Udp
extern PDEVICE_OBJECT	g_ipfltobj;		// \Device\RawIp
//extern PDEVICE_OBJECT	g_devcontrol;	// control device (exclusive access only!)
//extern PDEVICE_OBJECT	g_devnfo;		// information device 
extern PDEVICE_OBJECT  g_tcpoldobj;
extern PDEVICE_OBJECT  g_udpoldobj;
extern PDEVICE_OBJECT  g_ipoldobj;

extern BOOLEAN			gGingkoServerStarted;

NTSTATUS c_n_a_device(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT *fltobj, PDEVICE_OBJECT *oldobj, wchar_t *devname);

void d_n_d_device(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT oldobj, PDEVICE_OBJECT fltobj);

NTSTATUS RedfishDriverEntry(IN PDRIVER_OBJECT theDriverObject, IN PUNICODE_STRING theRegistryPath);
VOID RedfishUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS RedfishDeviceDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);

BOOLEAN IsGingkoServerProcess();

NTSTATUS
GingkoPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

#define REDFISH_NETWORK_DISPATCH(__DeviceObject, __Irp)			\
			if ( __DeviceObject == g_tcpfltobj ||					\
				 __DeviceObject == g_udpfltobj ||					\
				 __DeviceObject == g_ipfltobj )						\
				 {												\
					 return RedfishDeviceDispatch( __DeviceObject, __Irp );	\
				 }															\


#endif // __REDFISH_H_VERSION__
