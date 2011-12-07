// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS


#include <comsvcs.h>

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlapp.h>
	#if (_WTL_VER < 0x0800)
	#error This project requires Windows Template Library version 8.0 or higher
	#endif
#include <atlmisc.h>
#include <atlconv.h>
#include <atlddx.h>

#include <shlobj.h>
#include <comdef.h>
///#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <string>
#include <list>
#include <vector>
#include <errno.h>
#include <math.h>
#include <algorithm>


using namespace ATL;
extern CComModule _AtlModule;

//short syntax for msgboxes
#ifdef _DEBUG
#define MSGBOX(x) MessageBox(NULL, _T(x),_T(""),MB_OK | MB_TASKMODAL)
#define MSGBOXV(x) MessageBox(x,_T(""),MB_OK | MB_TASKMODAL)
#define MSGBOX0(x) MessageBox(NULL,_T(x),_T(""),MB_OK | MB_TASKMODAL)
#define MSGBOXV0(x) MessageBox(NULL,x,_T(""),MB_OK | MB_TASKMODAL)
#define MSGBOXVAR(x) {CString FF8D54DD_5160_4057_9499_DB4616C36FBC;\
						FF8D54DD_5160_4057_9499_DB4616C36FBC.Append(x);\
						MessageBox(FF8D54DD_5160_4057_9499_DB4616C36FBC,_T(""),MB_OK | MB_TASKMODAL);}
#define MSGBOXVAR0(x) {CString FF8D54DD_5160_4057_9499_DB4616C36FBC;\
						FF8D54DD_5160_4057_9499_DB4616C36FBC.Append(x);\
						MessageBox(NULL,FF8D54DD_5160_4057_9499_DB4616C36FBC,_T(""),MB_OK | MB_TASKMODAL);}
#else
#define MSGBOX(x)
#define MSGBOXV(x)
#define MSGBOX0(x)
#define MSGBOXV0(x)
#define MSGBOXVAR(x)
#define MSGBOXVAR0(x)
#endif


//#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
