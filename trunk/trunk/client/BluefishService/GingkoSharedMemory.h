#ifndef __GINGKO_SHARED_MEMORY_H__
#define __GINGKO_SHARED_MEMORY_H__
BOOL  WINAPI GkoInitialize();
BOOL  WINAPI GkoUnInitialize();
BOOL InitialServiceUrl();
BOOL PipeListenerServerThreadStart();
BOOL PipeListenerServerThreadStop();
DWORD WINAPI ConnectedInstanceThread(LPVOID lpvParam);
#endif
