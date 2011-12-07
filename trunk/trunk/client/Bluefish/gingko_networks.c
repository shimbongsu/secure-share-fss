#include "gingko_networks.h"

#define LINKNAME_STRING     L"\\DosDevices\\Passthru"
#define NTDEVICE_STRING     L"\\Device\\Passthru"

CONTROL_DEVICESTATE  ControlDeviceState = PS_DEVICE_STATE_READY;
LONG               MiniportCount = 0;
NDIS_HANDLE     NdisDeviceHandle = NULL;
PDEVICE_OBJECT  ControlDeviceObject = NULL;

NTSTATUS GingkoNdisDriverEntry(
    IN PDRIVER_OBJECT        DriverObject,
    IN PUNICODE_STRING       RegistryPath
    )
{
	NDIS_STATUS                        Status;
    NDIS_PROTOCOL_CHARACTERISTICS      PChars;
    NDIS_MINIPORT_CHARACTERISTICS      MChars;
    NDIS_STRING                        Name;

    Status = NDIS_STATUS_SUCCESS;
    NdisAllocateSpinLock(&GlobalLock);
    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);
	KdPrint(("Calling NdisMInitializeWrapper Before\n"));

    do
    {
        //
        // Register the miniport with NDIS. Note that it is the miniport
        // which was started as a driver and not the protocol. Also the miniport
        // must be registered prior to the protocol since the protocol's BindAdapter
        // handler can be initiated anytime and when it is, it must be ready to
        // start driver instances.
        //

        NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

        MChars.MajorNdisVersion = PASSTHRU_MAJOR_NDIS_VERSION;
        MChars.MinorNdisVersion = PASSTHRU_MINOR_NDIS_VERSION;

        MChars.InitializeHandler = GingkoNdisInitialize;
        MChars.QueryInformationHandler = GingkoNdisQueryInformation;
        MChars.SetInformationHandler = GingkoNdisSetInformation;
        MChars.ResetHandler = NULL;
        MChars.TransferDataHandler = GingkoNdisTransferData;
        MChars.HaltHandler = GingkoNdisHalt;
#ifdef NDIS51_MINIPORT
        MChars.CancelSendPacketsHandler = GingkoNdisCancelSendPackets;
        MChars.PnPEventNotifyHandler = GingkoNdisDevicePnPEvent;
        MChars.AdapterShutdownHandler = GingkoNdisAdapterShutdown;
#endif // NDIS51_MINIPORT

        //
        // We will disable the check for hang timeout so we do not
        // need a check for hang handler!
        //
        MChars.CheckForHangHandler = NULL;
        MChars.ReturnPacketHandler = GingkoNdisReturnPacket;

        //
        // Either the Send or the SendPackets handler should be specified.
        // If SendPackets handler is specified, SendHandler is ignored
        //
        MChars.SendHandler = NULL;    // MPSend;
        MChars.SendPacketsHandler = GingkoNdisSendPackets;

        Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
                                                  &MChars,
                                                  sizeof(MChars),
                                                  &DriverHandle);

		KdPrint(("Calling NdisIMRegisterLayeredMiniport After\n"));
		///return 0;
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }


#ifndef WIN9X
        NdisMRegisterUnloadHandler(NdisWrapperHandle, GingkoNdisUnload);
#endif

        //
        // Now register the protocol.
        //
        NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
        PChars.MajorNdisVersion = PASSTHRU_PROT_MAJOR_NDIS_VERSION;
        PChars.MinorNdisVersion = PASSTHRU_PROT_MINOR_NDIS_VERSION;

        //
        // Make sure the protocol-name matches the service-name
        // (from the INF) under which this protocol is installed.
        // This is needed to ensure that NDIS can correctly determine
        // the binding and call us to bind to miniports below.
        //
        NdisInitUnicodeString(&Name, L"GingkoFilter");    // Protocol name
        PChars.Name = Name;
        PChars.OpenAdapterCompleteHandler = GingkoNdisOpenAdapterComplete;
        PChars.CloseAdapterCompleteHandler = GingkoNdisCloseAdapterComplete;
        PChars.SendCompleteHandler = GingkoNdisSendComplete;
        PChars.TransferDataCompleteHandler = GingkoNdisTransferDataComplete;
    
        PChars.ResetCompleteHandler = GingkoNdisResetComplete;
        PChars.RequestCompleteHandler = GingkoNdisRequestComplete;
        PChars.ReceiveHandler = GingkoNdisReceive;
        PChars.ReceiveCompleteHandler = GingkoNdisReceiveComplete;
        PChars.StatusHandler = GingkoNdisStatus;
        PChars.StatusCompleteHandler = GingkoNdisStatusComplete;
        PChars.BindAdapterHandler = GingkoNdisBindAdapter;
        PChars.UnbindAdapterHandler = GingkoNdisUnbindAdapter;
        PChars.UnloadHandler = GingkoNdisUnloadProtocol;

        PChars.ReceivePacketHandler = GingkoNdisReceivePacket;
        PChars.PnPEventHandler= GingkoNdisPNPHandler;

        NdisRegisterProtocol(&Status,
                             &ProtHandle,
                             &PChars,
                             sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

		KdPrint(("Calling NdisRegisterProtocol After\n"));
        if (Status != NDIS_STATUS_SUCCESS)
        {
            NdisIMDeregisterLayeredMiniport(DriverHandle);
            break;
        }

        NdisIMAssociateMiniport(DriverHandle, ProtHandle);

		KdPrint(("Calling NdisIMAssociateMiniport After\n"));
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }

    return(Status);
}


VOID
GingkoNdisUnload(
    IN PDRIVER_OBJECT        DriverObject
    )
//
// PassThru driver unload function
//
{
    UNREFERENCED_PARAMETER(DriverObject);
    
    KdPrint(("PtUnload: entered\n"));
    
    GingkoNdisUnloadProtocol();
    
    NdisIMDeregisterLayeredMiniport(DriverHandle);
    
    NdisFreeSpinLock(&GlobalLock);

    KdPrint(("PtUnload: done!\n"));
}


NDIS_STATUS
GingkoNdisRegisterDevice(
    VOID
    )
/*++

Routine Description:

    Register an ioctl interface - a device object to be used for this
    purpose is created by NDIS when we call NdisMRegisterDevice.

    This routine is called whenever a new miniport instance is
    initialized. However, we only create one global device object,
    when the first miniport instance is initialized. This routine
    handles potential race conditions with PtDeregisterDevice via
    the ControlDeviceState and MiniportCount variables.

    NOTE: do not call this from DriverEntry; it will prevent the driver
    from being unloaded (e.g. on uninstall).

Arguments:

    None

Return Value:

    NDIS_STATUS_SUCCESS if we successfully register a device object.

--*/
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING         DeviceName;
    UNICODE_STRING         DeviceLinkUnicodeString;
    PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    KdPrint(("==>PtRegisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
        ASSERT(ControlDeviceState != PS_DEVICE_STATE_CREATING);

        //
        // Another thread could be running PtDeregisterDevice on
        // behalf of another miniport instance. If so, wait for
        // it to exit.
        //
        while (ControlDeviceState != PS_DEVICE_STATE_READY)
        {
            NdisReleaseSpinLock(&GlobalLock);
            NdisMSleep(1);
            NdisAcquireSpinLock(&GlobalLock);
        }

        ControlDeviceState = PS_DEVICE_STATE_CREATING;

        NdisReleaseSpinLock(&GlobalLock);

    
        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));

        DispatchTable[IRP_MJ_CREATE] = GingkoNdisDispatch;
        DispatchTable[IRP_MJ_CLEANUP] = GingkoNdisDispatch;
        DispatchTable[IRP_MJ_CLOSE] = GingkoNdisDispatch;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = GingkoNdisDispatch;
        

        NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
        NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

        //
        // Create a device object and register our dispatch handlers
        //
        
        Status = NdisMRegisterDevice(
                    NdisWrapperHandle, 
                    &DeviceName,
                    &DeviceLinkUnicodeString,
                    &DispatchTable[0],
                    &ControlDeviceObject,
                    &NdisDeviceHandle
                    );

        NdisAcquireSpinLock(&GlobalLock);

        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    KdPrint(("<==PtRegisterDevice: %x\n", Status));

    return (Status);
}


NDIS_STATUS
GingkoNdisDeregisterDevice(
    VOID
    )
/*++

Routine Description:

    Deregister the ioctl interface. This is called whenever a miniport
    instance is halted. When the last miniport instance is halted, we
    request NDIS to delete the device object

Arguments:

    NdisDeviceHandle - Handle returned by NdisMRegisterDevice

Return Value:

    NDIS_STATUS_SUCCESS if everything worked ok

--*/
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    KdPrint(("==>PassthruDeregisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
        //
        // All miniport instances have been halted. Deregister
        // the control device.
        //

        ASSERT(ControlDeviceState == PS_DEVICE_STATE_READY);

        //
        // Block PtRegisterDevice() while we release the control
        // device lock and deregister the device.
        // 
        ControlDeviceState = PS_DEVICE_STATE_DELETING;

        NdisReleaseSpinLock(&GlobalLock);

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }

        NdisAcquireSpinLock(&GlobalLock);
        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    KdPrint(("<== PassthruDeregisterDevice: %x\n", Status));
    return Status;
    
}



NTSTATUS
GingkoNdisDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
/*++
Routine Description:

    Process IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object
    Irp      - pointer to an I/O Request Packet

Return Value:

    NTSTATUS - STATUS_SUCCESS always - change this when adding
    real code to handle ioctls.

--*/
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DeviceObject);
    
    KdPrint(("==>Pt Dispatch\n"));
    irpStack = IoGetCurrentIrpStackLocation(Irp);
      

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            break;
            
        case IRP_MJ_CLEANUP:
            break;
            
        case IRP_MJ_CLOSE:
            break;        
            
        case IRP_MJ_DEVICE_CONTROL:
            //
            // Add code here to handle ioctl commands sent to passthru.
            //
            break;        
        default:
            break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    KdPrint(("<== Pt Dispatch\n"));

    return status;

} 

