#ifndef __RESIZABLE_PANEL_H__
#define __RESIZABLE_PANEL_H__

#ifndef __ATLCRACK_H__
#error please include atlcrack.h first
#endif

#ifndef __ATLCTRLS_H__
#include <atlctrls.h>
#endif

#ifndef __ATLSCRL_H__
#include <atlscrl.h>
#endif

#ifndef __ATLMISC_H__
#define _ATL_TMP_NO_CSTRING
#include <atlmisc.h>
#endif

#ifndef __ATLIMAGE_H_
#define __ATLTYPES_H__	// Use the WTL types
#include <atlimage.h>
#endif


class CResizablePanel : 
	public	CResizableDialogImpl<CResizablePanel>,
	public CMessageFilter, 
	public CIdleHandler
{
	typedef CResizableDialogImpl<CResizablePanel> baseClass;
	typedef CResizablePanel thisClass;

public:
	enum { IDD = IDD_NORMAL_PANEL };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return IsDialogMessage( pMsg );
	}
	
	virtual BOOL OnIdle()
	{
		return TRUE;
	}

	//BEGIN_MSG_MAP(CMainDlg)
	//	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	//	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	//	COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
	//	COMMAND_ID_HANDLER(IDOK, OnOK)
	//	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	//END_MSG_MAP()

	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return TRUE;
	}
};


// Notes:

#endif
