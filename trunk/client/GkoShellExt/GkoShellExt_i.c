

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Jun 03 10:25:07 2011
 */
/* Compiler settings for .\GkoShellExt.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IComponentRegistrar,0xa817e7a2,0x43fa,0x11d0,0x9e,0x44,0x00,0xaa,0x00,0xb6,0x77,0x0a);


MIDL_DEFINE_GUID(IID, IID_IGingkoContextMenuExt,0xDE85BF1D,0x8185,0x4C31,0x94,0xFC,0xC1,0x9E,0xE6,0xE4,0xC7,0xF9);


MIDL_DEFINE_GUID(IID, LIBID_GkoShellExtLib,0x1A0BE39A,0xA0A0,0x4D53,0xB6,0x4E,0xDC,0xC4,0xCD,0xFF,0xC2,0x44);


MIDL_DEFINE_GUID(CLSID, CLSID_CompReg,0xD1046D0B,0x8B45,0x461D,0xA3,0xC8,0x25,0x3F,0x93,0x95,0x8D,0x3F);


MIDL_DEFINE_GUID(CLSID, CLSID_CGingkoContextMenuExt,0x7DFB6A6B,0x8CB7,0x4B1B,0x9C,0xA9,0x6C,0x9F,0x11,0xE9,0x23,0xC9);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



