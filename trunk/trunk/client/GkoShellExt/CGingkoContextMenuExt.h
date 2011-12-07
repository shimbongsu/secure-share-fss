// CGingkoContextMenuExt.h : Declaration of the CCGingkoContextMenuExt

#pragma once
#include "resource.h"       // main symbols
//#include "GkoShellExt_i.h"
#include "dllmain.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif


extern HBITMAP  __g_hBitmap;
// CCGingkoContextMenuExt

class ATL_NO_VTABLE CCGingkoContextMenuExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCGingkoContextMenuExt, &CLSID_CGingkoContextMenuExt>,
	public IDispatchImpl<IGingkoContextMenuExt, &IID_IGingkoContextMenuExt, &LIBID_GkoShellExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
    public IContextMenu
{
private:
	HBITMAP m_hBitmap;
public:
	CCGingkoContextMenuExt()
	{
		//m_hBitmap = (HBITMAP) ::LoadImage(_AtlModule.m_Instance, 
		//						MAKEINTRESOURCE(IDB_GKOSHELLEXT),
		//						IMAGE_BITMAP, 
		//						32, 
		//						32, 
		//						LR_DEFAULTCOLOR);
		
		m_hBitmap = __g_hBitmap; //LoadBitmap( _AtlModule.GetResourceInstance(), 
								//MAKEINTRESOURCE(IDB_GINGKOSHELL) );
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CGINGKOCONTEXTMENUEXT)
DECLARE_NOT_AGGREGATABLE(CCGingkoContextMenuExt)
DECLARE_PROTECT_FINAL_CONSTRUCT()


BEGIN_COM_MAP(CCGingkoContextMenuExt)
	COM_INTERFACE_ENTRY(IGingkoContextMenuExt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}



public:

	    // IShellExtInit
    STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu
    STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
    STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);

protected:
	BOOL		m_SelectedFile;
	CString		m_szSelectedFilename;
};

OBJECT_ENTRY_AUTO(__uuidof(CGingkoContextMenuExt), CCGingkoContextMenuExt)


