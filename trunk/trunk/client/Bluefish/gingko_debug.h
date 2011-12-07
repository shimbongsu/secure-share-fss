#ifndef __GINGKO_DEBUG_H__
#define __GINGKO_DEBUG_H__
#include <ntifs.h>
#include <stdlib.h>

#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

VOID
GetIrpName (
    IN UCHAR MajorCode,
    IN UCHAR MinorCode,
    IN ULONG FsctlCode,
    OUT PCHAR MajorCodeName,
    OUT PCHAR MinorCodeName);

VOID
GetFastioName (
    IN FASTIO_TYPE FastioCode,
    OUT PCHAR FastioName);

#if WINVER >= 0x0501 /* See comment in DriverEntry */
VOID
GetFsFilterOperationName (
    IN UCHAR FsFilterOperation,
    OUT PCHAR FsFilterOperationName);
#endif

#endif

