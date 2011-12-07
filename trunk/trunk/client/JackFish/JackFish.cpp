// JackFish.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "JackFish.h"

void WINAPI ProcessCall();
BOOL LockJackApi(JackApi *pJack);
BOOL UnLockJackApi(JackApi *pJack);
BOOL FindJackApi();

JackApi *g_JackApiLink = NULL;

BOOL RemoveProtection( JackApi *pinfo, BOOL blRestore )
{
	BYTE *papi =(BYTE *) pinfo->old_api + pinfo->start_pos;
	BOOL ret;

	MEMORY_BASIC_INFORMATION mbi;
	DWORD dwProtectionFlags;
	DWORD dwScratch;

	VirtualQuery( papi, &mbi, sizeof( mbi ) );

	if( blRestore == TRUE )
	{
		dwProtectionFlags = mbi.Protect;
		pinfo->old_protection_flags = dwProtectionFlags;
		dwProtectionFlags = PAGE_EXECUTE_READWRITE; // PAGE_READWRITE;
		ret = VirtualProtect( papi, 20, dwProtectionFlags, &dwScratch );
	}
	else
	{
		//pinfo->old_protection_flags =dwProtectionFlags;
		dwProtectionFlags = pinfo->old_protection_flags; // PAGE_READWRITE;
		ret = VirtualProtect( papi, 20, dwProtectionFlags, &dwScratch );
	}

	if(ret ==FALSE )
	{
		//char err[256];
		//GetErrString(err, sizeof(err), GetLastError());
		//WriteLog("Error VirtualProtect:%s", err);
		//WriteLog("VirtualProtect:%x,%x,%x", pvAddress, dwProtectionFlags, dwScratch);
		return false;
	}

	return ret;
}


JackApi *FindJackApi(FishFunction pOldFun)
{
	JackApi *pinfo = g_JackApiLink;

	while( pinfo )
	{
		if((BYTE *)pinfo->old_api + pinfo->start_pos == (BYTE *)pOldFun )
			break;

		pinfo = pinfo->Next;
	}

	return pinfo;
}

int RestoreAPICodes(JackApi *pJack )
{
	if(pJack ==NULL || pJack->f_hooked ==false) return 0;

	LockJackApi( pJack );
	//g_api_info.lock(pinfo);
	memcpy((PBYTE)pJack->old_api+pJack->start_pos, pJack->save_bytes, CALL_BYTES_SIZE/*(5>bytes)?bytes:5*/);
	UnLockJackApi( pJack );
	//g_api_info.unlock(pinfo);
	return 0;
}


BOOL JackHookApi( JackApi *pJack )
{
	//WriteLog("debug: Hook one api:%s, start_pos:%d", pinfo->api_name, pinfo->start_pos);

	if( pJack->f_hooked )
		return 0;

	BYTE *papi =(BYTE *)pJack->old_api + pJack->start_pos;  // insert call xxxxxxxx from start_pos. lgd 2003.03.01

	//��������ָ���ݸ�Ϊ��д
	if( !RemoveProtection( pJack, TRUE ) )
	{
		return -1;
	}

	// ����Դ����ǰ5���ֽڣ���Ϊ���潫���Ǵ˴�
	memcpy(pJack->save_bytes, papi, CALL_BYTES_SIZE /*sizeof(pinfo->save_bytes)*/ );

	//��Դ����ǰ����call ProcessCall����
	papi[0] = 0xE8;

	*(DWORD *)&papi[1] = (DWORD) ProcessCall - (DWORD) papi - CALL_BYTES_SIZE;

	pJack->f_hooked = true;

	RemoveProtection( pJack, FALSE );
		
	return 0;
}

// ��Ҫ����ProcessCall������api�������滻����
void WINAPI ProcessCall()
{
	PBYTE pbAfterCall = NULL;
	PDWORD pdwParam = NULL;
	PDWORD pdwESP = NULL;
	DWORD dwParamSize = 0;
	BYTE *pvReturnAddr = NULL;
	DWORD dwReturnValue = 0;
	BYTE cl_val = 0;
	BYTE *papi = NULL;
	DWORD errcode =0;

#ifdef WINNT
	PROCESS_INFORMATION *pi = NULL;
	char fname[128];
#endif

	__asm
	{
 		Mov [cl_val], CL   // ����CL for xxTextOutx in WIN9X
		Mov EAX, [EBP + 4] //ǰ���Ǳ��滻��call xxxxxxxx����
		Mov [pbAfterCall], EAX

		Mov EAX, [EBP + 8]
		Mov [pvReturnAddr], EAX // �����ϴε���λ��

		Lea EAX, [EBP + 12]
		Mov [pdwParam], EAX    //ȡ����
	}

	JackApi *pinfo;

	
	//if((pinfo = g_api_info.FindByOldAPI( (FishFunction)( pbAfterCall - CALL_BYTES_SIZE ) ) ) == NULL )
	if((pinfo = FindJackApi( (FishFunction)( pbAfterCall - CALL_BYTES_SIZE ) ) ) == NULL )
	{
		goto call_ret;
	}

	//if(strcmp(pinfo->api_name, "send") ==0)
	//	pbAfterCall =pbAfterCall-5;

	LockJackApi( pinfo );

	papi =(BYTE *)pinfo->old_api+pinfo->start_pos;

	dwParamSize = pinfo->param_count * 4;  // 32λ��ַʹ��4���ֽ�

	errcode = 0;
	
	if( !RemoveProtection( pinfo, TRUE ) )   // add 2004.03.30 for some time other program changed api's write permission
	{
		goto call_ret;
	}

	memcpy( papi, pinfo->save_bytes, CALL_BYTES_SIZE );   //��ԭԭapi����ǰ5�ֽ�(�ָ�apiԭ��������)

	RestoreAPICodes(pinfo);
	// ѹ�������ջ
	__asm
	{
		Sub ESP, [dwParamSize]
		Mov [pdwESP], ESP
	}

	memcpy(pdwESP, pdwParam, dwParamSize);

	FishFunction dwAPI = pinfo->my_api;

	__asm
	{

		//Push ESP
		//Sub ESP, EBP 
		Mov CL, [cl_val]
		Call [dwAPI]
		//Mov ESP, EBP
	}

	// ����mydll����滻����
	//pinfo->my_api();

	__asm
	{
		Push EAX
		Mov [dwReturnValue], EAX  //���췵��ֵ
	}

	errcode =GetLastError();

#ifdef WINNT
	if(!strcmp(pinfo->api_name, "CreateProcessW") || !strcmp(pinfo->api_name, "CreateProcessA") )
	{
		pi = (PROCESS_INFORMATION *) pdwParam[9];
		if(pi->hProcess)
		{
			HWND hwndExe = FindWindowW( NIBLESMARTWINDOWCLASS, NIBLESMARTWINDOWNAME );
			
			if( hwndExe && IsWindow( hwndExe ) )
			{
				//SendMessage( hwndExe, WM_APP + 1000, (DWORD) lpProcessInformation->dwProcessId, 0L );
				PostMessage( hwndExe, WM_APP + 1000, (DWORD) pi->dwProcessId,
						(DWORD) pi->dwThreadId );
			}
			else
			{
				ResumeThread( pi->hThread );
			}
			//HookOneProcess( pi->dwProcessId, pi->dwThreadId );
		}
		//else WriteLog("hProcess ==NULL");
	}
#endif

	RestoreAPICodes(pinfo);

	// �ָ� Call xxxx
	papi[0] =0xE8;
	*(DWORD *)&papi[1] =(DWORD) ProcessCall - (DWORD)papi - CALL_BYTES_SIZE;

	//g_api_info.unlock(pinfo);
	UnLockJackApi( pinfo );


call_ret:
	
	RemoveProtection( pinfo, FALSE );
	// �ָ�������
	SetLastError(errcode);	
	// �����麯������
	__asm
	{
		Pop EAX

		Mov ECX, [dwParamSize]
		//Mov EBX, [pbAfterCall]
		Mov EDX, [pvReturnAddr]
		Pop EDI
		Pop ESI
		Pop EBX
		Mov ESP, EBP
		Pop EBP
	}
	
	//_asm
	//{
	//	push	ebp
	//	push ebx
	//	push esi
	//	push edi
	//}
	
	__asm
	{
		Add ESP, 8
		Add ESP, ECX  //��ջָ��Ӳ�����С����ΪProcessCallû�в���������Ҫ���
		Push EDX
		Ret
	}
}

int LockJackApi(JackApi *pinfo)
{
	EnterCriticalSection(&pinfo->cs);
	return 0;
}

int UnLockJackApi(JackApi *pinfo)
{
	LeaveCriticalSection(&pinfo->cs);
	return 0;
}