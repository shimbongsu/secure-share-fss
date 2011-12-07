#include "stdafx.h"
#include <stdlib.h>
#include <distorm.h>
#include "JackFish.h"
// 10000 hooks should be enough

typedef struct _HOOK_INFO
{
	ULONG_PTR Function;	// Address of the original function

	ULONG_PTR Hook;		// Address of the function to call 
						// instead of the original

	ULONG_PTR Bridge;   // Address of the instruction bridge
						// necessary because of the hook jmp
						// which overwrites instructions

} HOOK_INFO, *PHOOK_INFO;

HOOK_INFO HookInfo[MAX_HOOKS];

UINT NumberOfHooks = 0;

BYTE *pBridgeBuffer = NULL; // Here are going to be stored all the bridges

UINT CurrentBridgeBufferSize = 0; // This number is incremented as
								  // the bridge buffer is growing
HOOK_INFO *GetHookInfoFromFunction(ULONG_PTR OriginalFunction)
{
	if (NumberOfHooks == 0)
		return NULL;

	for (UINT x = 0; x < NumberOfHooks; x++)
	{
		if (HookInfo[x].Function == OriginalFunction)
			return &HookInfo[x];
	}

	return NULL;
}

//
// This function  retrieves the necessary size for the jump
//

UINT GetJumpSize(ULONG_PTR PosA, ULONG_PTR PosB)
{
	ULONG_PTR res = max(PosA, PosB) - min(PosA, PosB);

	// if you want to handle relative jumps
	/*if (res <= (ULONG_PTR) 0x7FFF0000)
	{
		return 5; // jmp rel
	}
	else
	{*/
		// jmp [xxx] + addr

#ifdef _M_IX86

		return 10;

#else ifdef _M_AMD64

		return 14;

#endif
	//}

	return 0; // error
}

//
// This function writes unconditional jumps
// both for x86 and x64
//

VOID WriteJump(VOID *pAddress, ULONG_PTR JumpTo)
{
	DWORD dwOldProtect = 0;

	VirtualProtect(pAddress, JUMP_WORST, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	BYTE *pCur = (BYTE *) pAddress;

	// if you want to handle relative jumps
	/*if (JumpTo - (ULONG_PTR) pAddress <= (ULONG_PTR) 0x7FFF0000)
	{
		*pCur = 0xE9;  // jmp rel

		DWORD RelAddr = (DWORD) (JumpTo - (ULONG_PTR) pAddress) - 5;

		memcpy(++pCur, &RelAddr, sizeof (DWORD));
	}
	else
	{*/

#ifdef _M_IX86

		*pCur = 0xff;     // jmp [addr]
		*(++pCur) = 0x25;
		pCur++;
		*((DWORD *) pCur) = (DWORD)(((ULONG_PTR) pCur) + sizeof (DWORD));
		pCur += sizeof (DWORD);
		*((ULONG_PTR *)pCur) = JumpTo;

#else ifdef _M_AMD64

		*pCur = 0xff;		// jmp [rip+addr]
		*(++pCur) = 0x25;
		*((DWORD *) ++pCur) = 0; // addr = 0
		pCur += sizeof (DWORD);
		*((ULONG_PTR *)pCur) = JumpTo;

#endif
	//}

	DWORD dwBuf = 0;	// nessary othewrise the function fails

	VirtualProtect(pAddress, JUMP_WORST, dwOldProtect, &dwBuf);
}


//
// This function creates a bridge of the original function
//

VOID *CreateBridge(ULONG_PTR Function, const UINT JumpSize)
{
	if (pBridgeBuffer == NULL) return NULL;

#define MAX_INSTRUCTIONS 100

	_DecodeResult res;
	_DecodedInst decodedInstructions[MAX_INSTRUCTIONS];
	unsigned int decodedInstructionsCount = 0;

#ifdef _M_IX86

	_DecodeType dt = Decode32Bits;

#else ifdef _M_AMD64

	_DecodeType dt = Decode64Bits;

#endif

	_OffsetType offset = 0;

	res = distorm_decode(offset,	// offset for buffer
		(const BYTE *) Function,	// buffer to disassemble
		50,							// function size (code size to disasm) 
									// 50 instr should be _quite_ enough
		dt,							// x86 or x64?
		decodedInstructions,		// decoded instr
		MAX_INSTRUCTIONS,			// array size
		&decodedInstructionsCount	// how many instr were disassembled?
		);

	if (res == DECRES_INPUTERR)
		return NULL;

	DWORD InstrSize = 0;

	VOID *pBridge = (VOID *) &pBridgeBuffer[CurrentBridgeBufferSize];

	for (UINT x = 0; x < decodedInstructionsCount; x++)
	{
		if (InstrSize >= JumpSize)
			break;

		BYTE *pCurInstr = (BYTE *) (InstrSize + (ULONG_PTR) Function);

		//
		// This is an sample attempt of handling a jump
		// It works, but it converts the jz to jmp
		// since I didn't write the code for writing
		// conditional jumps
		//
		/*
		if (*pCurInstr == 0x74) // jz near
		{
			ULONG_PTR Dest = (InstrSize + (ULONG_PTR) Function)
				+ (char) pCurInstr[1];

			WriteJump(&pBridgeBuffer[CurrentBridgeBufferSize], Dest);

			CurrentBridgeBufferSize += JumpSize;
		}
		else
		{*/
			memcpy(&pBridgeBuffer[CurrentBridgeBufferSize], 
				(VOID *) pCurInstr, decodedInstructions[x].size);

			CurrentBridgeBufferSize += decodedInstructions[x].size;
		//}

		InstrSize += decodedInstructions[x].size;
	}

	WriteJump(&pBridgeBuffer[CurrentBridgeBufferSize], Function + InstrSize);
	CurrentBridgeBufferSize += GetJumpSize((ULONG_PTR) &pBridgeBuffer[CurrentBridgeBufferSize],
		Function + InstrSize);

	return pBridge;
}

//
// Hooks a function
//

//extern "C" __declspec(dllexport)
BOOL __cdecl HookFunction(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction)
{
	//
	// Check if the function has already been hooked
	// If so, no disassembling is necessary since we already
	// have our bridge
	//

	HOOK_INFO *hinfo = GetHookInfoFromFunction(OriginalFunction);

	if (hinfo)
	{
		WriteJump((VOID *) OriginalFunction, NewFunction);
	}
	else
	{
		if (NumberOfHooks == (MAX_HOOKS - 1))
			return FALSE;

		VOID *pBridge = CreateBridge(OriginalFunction, GetJumpSize(OriginalFunction, NewFunction));

		if (pBridge == NULL)
			return FALSE;

		HookInfo[NumberOfHooks].Function = OriginalFunction;
		HookInfo[NumberOfHooks].Bridge = (ULONG_PTR) pBridge;
		HookInfo[NumberOfHooks].Hook = NewFunction;

		NumberOfHooks++;

		WriteJump((VOID *) OriginalFunction, NewFunction);
	}

	return TRUE;
}


//
// Unhooks a function
//

//extern "C" __declspec(dllexport)
VOID __cdecl UnhookFunction(ULONG_PTR Function)
{
	//
	// Check if the function has already been hooked
	// If not, I can't unhook it
	//

	HOOK_INFO *hinfo = GetHookInfoFromFunction(Function);

	if (hinfo)
	{
		//
		// Replaces the hook jump with a jump to the bridge
		// I'm not completely unhooking since I'm not
		// restoring the original bytes
		//

		WriteJump((VOID *) hinfo->Function, hinfo->Bridge);
	}
}

//
// Get the bridge to call instead of the original function from hook
//

//extern "C" __declspec(dllexport)
ULONG_PTR __cdecl GetOriginalFunction(ULONG_PTR Hook)
{
	if (NumberOfHooks == 0)
		return NULL;

	for (UINT x = 0; x < NumberOfHooks; x++)
	{
		if (HookInfo[x].Hook == Hook)
			return HookInfo[x].Bridge;
	}

	return NULL;
}

BOOL BuildCommand(GingkoCommand** pCmd, LPCTSTR szCmd, DWORD cchBuf, LPVOID lpBuf)
{
	DWORD dwCmdSize = 0;
	if( pCmd != NULL )
	{
		GingkoCommand *pTempCmd = (GingkoCommand*) malloc( sizeof(GingkoCommand) + cchBuf + sizeof(TCHAR) );
		if( pTempCmd != NULL )
		{
			memset( pTempCmd, 0, sizeof(GingkoCommand) + cchBuf + sizeof(TCHAR) );
			
			dwCmdSize = (DWORD) _tcslen(szCmd);
			if( dwCmdSize  >= GINGKO_COMMAND_LENGTH )
			{
				FreeCommand(pTempCmd);
				return FALSE;
			}
			
			memcpy( pTempCmd->szCommand, szCmd, dwCmdSize * sizeof(TCHAR) );
			
			if( lpBuf != NULL )
			{
				memcpy( pTempCmd->szCommandBody, lpBuf, cchBuf );
			}

			pTempCmd->dwSize = sizeof(GingkoCommand) + cchBuf;
			pTempCmd->dwBufSize = cchBuf;

			*pCmd = pTempCmd;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ExecCommand(GingkoCommand* pCmd, LPVOID lpRetBuf, DWORD cchBuf)
{
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;
	DWORD cbWritten = 0;
	DWORD cbRead = 0;
	LPCTSTR lpszPipename = GINGKO_PIPE_NAME;
	TCHAR	ReplyBuff[ GINGKO_BLOCK_SIZE ];

	while (TRUE)
	{
		hPipe = CreateFile( 
			lpszPipename,   // pipe name 
			GENERIC_READ | GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 
		
		if (hPipe != INVALID_HANDLE_VALUE) 
			break; 
 
		// Exit if an error other than ERROR_PIPE_BUSY occurs. 
 
		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			printf("Could not open pipe"); 
			return FALSE;
		}
 
		// All pipe instances are busy, so wait for 20 seconds. 
 
		if (!WaitNamedPipe(lpszPipename, 20000)) 
		{ 
			printf("Could not open pipe"); 
			CloseHandle( hPipe );
			return FALSE;
		} 
	}
 
	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess) 
	{
		printf("SetNamedPipeHandleState failed"); 
		CloseHandle( hPipe );
		return FALSE;
	}
 
// Send a message to the pipe server. 
 
	fSuccess = WriteFile( 
				hPipe,                  // pipe handle 
				pCmd,             // message 
				pCmd->dwSize, // message length 
				&cbWritten,             // bytes written 
				NULL);                  // not overlapped 

	if (!fSuccess) 
	{
		printf("WriteFile failed"); 
		CloseHandle( hPipe );
		return FALSE;
	}
 
	fSuccess = ReadFile( 
				hPipe,    // pipe handle 
				ReplyBuff,    // buffer to receive reply 
				GINGKO_BLOCK_SIZE,  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

	if( fSuccess )
	{
		if( lpRetBuf )
		{
			memset( lpRetBuf, 0, cchBuf );
			memcpy( lpRetBuf, ReplyBuff, cbRead > cchBuf ? cchBuf : cbRead );
		}
	}
 
   CloseHandle(hPipe);  
   return TRUE; 

}

BOOL FreeCommand(GingkoCommand* pCmd)
{
	if( pCmd != NULL )
	{
		free( pCmd );
		return TRUE;
	}
	return FALSE;
}


ULONG GetProcessPermission( ULONG ProcessId )
{
	GingkoCommand *pCmd = NULL;
	TCHAR szProcessId[17] = {0};
	_stprintf_s( szProcessId, 16, _T("%d"), ProcessId );
	LOG( L"Process Id: %d, the Process Id Str: %s.\n", ProcessId, szProcessId );
	if( BuildCommand( &pCmd, _T("GetPermission:"), sizeof(TCHAR) * 16, szProcessId ) )
	{
		ULONG Permission = 0;
		TCHAR szPermission[17] = {0};
		LOG( L"Calling ExecCommand by Pipe\n");
		ExecCommand( pCmd, szPermission, sizeof(szPermission) ); 
		Permission = (ULONG) _ttol( szPermission );
		FreeCommand( pCmd );
		LOG( L"Return the Permission %s (%d) \n", szPermission, Permission );
		return Permission;
	}
	return 0L;
}

BOOL NotifyProcessExit()
{

	GingkoCommand *pCmd = NULL;
	TCHAR szProcessId[17] = {0};
	_stprintf_s( szProcessId, 16, _T("%d"), GetCurrentProcessId() );
	LOG( L"Process (%s) exit.\n", szProcessId );
	if( BuildCommand( &pCmd, _T("__NotifyProcessExit:"), sizeof(TCHAR) * 16, szProcessId ) )
	{
		int i = 0;
		while( !ExecCommand( pCmd, NULL, 0 ) && i < 5 ) 
			i ++;

		FreeCommand( pCmd );
	}
	return TRUE;
}
