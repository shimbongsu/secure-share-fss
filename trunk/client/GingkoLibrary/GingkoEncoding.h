#ifndef __GINGKO_ENCODING_H__
#define __GINGKO_ENCODING_H__

static BOOL BuildCommand(GingkoCommand** pCmd, LPCTSTR szCmd, DWORD cchBuf, LPVOID lpBuf);
static BOOL ExecCommand(GingkoCommand* pCmd, LPVOID lpRetBuf, DWORD cchBuf);
static BOOL FreeCommand(GingkoCommand* pCmd);

#endif
