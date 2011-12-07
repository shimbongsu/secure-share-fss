/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 2004  Shankr.  All Rights Reserved.

Module Name:

main.cpp

Abstract:

Contains main function for service creation

Author:

Shankr        30-Nov-2004

Revision History:

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>
#include <assert.h>
#include "GingkoServiceDispatcher.h"
#include "GingkoSharedMemory.h"
#include "cheapman.h"

#define SERVICENAME _T("GkoBluefishService")

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   ServiceStatusHandle; 
BOOL	Is64BitWindows = FALSE;
//Indicates status of the Service : STOP, PAUSE, CONTINUE
HANDLE g_hServiceEvents[3] = {NULL};
FILE* hLogFile = NULL;


//Check if the command line parameters are valid
bool CheckParam(void)
{
	if (__argc > 1)
	{
		if ( _tcscmp(__wargv[1], _T("-Install") ) 
			&& _tcscmp(__wargv[1], _T("-Remove"))
			&& _tcscmp(__wargv[1], _T("-Hook") ) 
			)
		{
			return false;
		}
	}
	return true;
}

//Show help info
void Usage(void)
{
	_tprintf(_T("============================================================\n"));
	_tprintf(_T(" Name:\tBluefishService\n"));
	_tprintf(_T(" Author: Long Zou\n"));
	_tprintf(_T(" Date:\t2009-3-25\n"));
	_tprintf(_T(" Usage:\n"));
	_tprintf(_T(" \tInstall service:   BluefishService.exe -Install\n"));
	_tprintf(_T(" \tUninstall service: BluefishService.exe -Remove\n"));
	_tprintf(_T("============================================================\n"));
}

void InstallCmdService()
{
	SC_HANDLE        schSCManager;
	SC_HANDLE        schService;
	TCHAR			 szCurrentPath[MAX_PATH];
	DWORD            dwErrorCode;
	SERVICE_STATUS   InstallServiceStatus;

	GetModuleFileName(NULL,szCurrentPath,MAX_PATH);

	schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager==NULL)
	{
		_tprintf(_T("Open Service Control Manager Database Failure !\n"));
		return ;
	}

	_tprintf(_T("Creating Service .... "));
	schService=CreateService(schSCManager,SERVICENAME,SERVICENAME,SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,SERVICE_AUTO_START,
		SERVICE_ERROR_IGNORE,szCurrentPath,NULL,NULL,NULL,NULL,NULL); 
	if(schService==NULL)
	{
		dwErrorCode=GetLastError();
		if(dwErrorCode!=ERROR_SERVICE_EXISTS)
		{
			_tprintf(_T("Failure !\n"));
			CloseServiceHandle(schSCManager);
			return ;
		}
		else
		{
			_tprintf(_T("already Exists !\n"));
			schService=OpenService(schSCManager,SERVICENAME,SERVICE_START);
			if(schService==NULL)
			{
				_tprintf(_T("Opening Service .... Failure !\n"));
				CloseServiceHandle(schSCManager);
				return ;
			}
		}
	}
	else
	{
		InstallEventSource();
		_tprintf(_T("Success !\n"));
	}

	_tprintf(_T("Starting Service .... "));
	if(StartService(schService,0,NULL)==0)                         
	{
		dwErrorCode=GetLastError();
		if(dwErrorCode==ERROR_SERVICE_ALREADY_RUNNING)
		{
			_tprintf(_T("already Running !\n"));
			CloseServiceHandle(schSCManager);  
			CloseServiceHandle(schService);
			return ;
		}
	}
	else
	{
		_tprintf(_T("Pending ... "));
	}

	while(QueryServiceStatus(schService,&InstallServiceStatus)!=0)           
	{
		if(InstallServiceStatus.dwCurrentState==SERVICE_START_PENDING)
		{
			Sleep(100);
		}
		else
		{
			break;
		}
	}

	if(InstallServiceStatus.dwCurrentState!=SERVICE_RUNNING)
	{
		_tprintf(_T("Failure !\n"));
	}
	else
	{
		OpenDatabase( TRUE );
		_tprintf(_T("Success !\n"));
		CloseDatabase();
	}

	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);
	return ;
}

void RemoveCmdService() 
{
	SC_HANDLE        schSCManager;
	SC_HANDLE        schService;
	SERVICE_STATUS   RemoveServiceStatus;
	DWORD            dwErrorCode;

	schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager==NULL)
	{
		_tprintf(_T("Opening SCM ......... "));
		dwErrorCode=GetLastError();
		if(dwErrorCode!=5)
		{
			_tprintf(_T("Failure !\n")); 
		}
		else
		{
			_tprintf(_T("Failuer ... Access is Denied !\n"));
		}
		return ;
	}

	schService=OpenService(schSCManager,SERVICENAME,SERVICE_ALL_ACCESS);

	if(schService==NULL) 
	{
		_tprintf(_T("Opening Service ..... "));
		dwErrorCode=GetLastError();
		if(dwErrorCode==1060)
		{
			_tprintf(_T("no Exists !\n"));
		}
		else
		{
			_tprintf(_T("Failure !\n"));
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		_tprintf(_T("Stopping Service .... "));
		if(QueryServiceStatus(schService,&RemoveServiceStatus)!=0)
		{
			if(RemoveServiceStatus.dwCurrentState == SERVICE_STOPPED)
			{
				_tprintf(_T("already Stopped !\n")); 
			}
			else
			{
				_tprintf(_T("Pending ... "));
				if(ControlService(schService,SERVICE_CONTROL_STOP,&RemoveServiceStatus)!=0)
				{
					while(RemoveServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)         
					{
						Sleep(10);
						QueryServiceStatus(schService,&RemoveServiceStatus);
					}
					if(RemoveServiceStatus.dwCurrentState==SERVICE_STOPPED)
					{
						_tprintf(_T("Success !\n"));
					}
					else
					{
						_tprintf(_T("Failure !\n"));
					}
				}
				else
				{
					_tprintf(_T("Failure !\n"));          
				}
			}
		}
		else
		{
			_tprintf(_T("Query Failure !\n"));
		}

		_tprintf(_T("Removing Service .... "));     
		if(DeleteService(schService)==0)
		{
			_tprintf(_T("Failure !\n"));   
		}
		else
		{
			_tprintf(_T("Success !\n"));
		}
	}

	CloseServiceHandle(schSCManager);        
	CloseServiceHandle(schService);

	return;
}

// Service Ctrl handler
VOID WINAPI CmdCtrlHandler (DWORD Opcode) 
{ 
	DWORD status; 

	switch(Opcode) 
	{ 
	case SERVICE_CONTROL_STOP: 
		// Signal the event to stop the main thread
		

		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
		ServiceStatus.dwCheckPoint    = 0; 
		ServiceStatus.dwWaitHint      = 0; 

		if (!SetServiceStatus (ServiceStatusHandle, 
			&ServiceStatus))
		{ 
			status = GetLastError(); 
		} 

		SetEvent( g_hServiceEvents[0] );

		return; 

	case SERVICE_CONTROL_PAUSE:
		{
			SetEvent( g_hServiceEvents[1] );

			ServiceStatus.dwWin32ExitCode = 0; 
			ServiceStatus.dwCurrentState  = SERVICE_PAUSED; 
			ServiceStatus.dwCheckPoint    = 0; 
			ServiceStatus.dwWaitHint      = 0; 

			if (!SetServiceStatus (ServiceStatusHandle, 
				&ServiceStatus))
			{ 
				status = GetLastError(); 
			} 
			return; 
		}
	case SERVICE_CONTROL_CONTINUE:
		SetEvent( g_hServiceEvents[2] );

		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState  = SERVICE_RUNNING; 
		ServiceStatus.dwCheckPoint    = 0; 
		ServiceStatus.dwWaitHint      = 0; 

		if (!SetServiceStatus (ServiceStatusHandle, 
			&ServiceStatus))
		{ 
			status = GetLastError(); 
		} 
		return; 

	case SERVICE_CONTROL_INTERROGATE: 
		// Fall through to send current status. 
		break; 
	} 

	// Send current status. 
	if (!SetServiceStatus (ServiceStatusHandle,  &ServiceStatus)) 
	{ 
		status = GetLastError(); 
	} 
	return; 
}

// Deletes service
void DeleteSvc()
{
	// Open service manager
	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
		return;

	// OPen service
	SC_HANDLE hService = ::OpenService( hSCM, SERVICENAME, SERVICE_ALL_ACCESS );

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		return;
	}

	// Deletes service from service database
	DeleteService( hService );

	// Stop the service
	ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
	ServiceStatus.dwCheckPoint         = 0; 
	ServiceStatus.dwWaitHint           = 0; 
	ServiceStatus.dwWin32ExitCode      = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	SetServiceStatus (ServiceStatusHandle, &ServiceStatus); 

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
}

// Service "main" function
void _ServiceMain( void* )
{
	ThreadBag *pBag = NULL;
	//HANDLE hEvent;
	int tryTimes = 0;
	DWORD Current = 0;
	DWORD Previous = 0;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\__gingko_pipe_server"); 
	DWORD dwWaitRet = WaitForMultipleObjects(2, g_hServiceEvents, FALSE, 10000);
	LOG_START();
	if( GkoInitialize() == FALSE )
	{
		return;
	}

	InitCheapman();

	LOG(L"GkCryptoInitialize\n");
	//InstallAndLoadDriver();
	LOG(L"InstallAndLoadDriver\n");

	//StartGingkoService();

	//StartGingkoHookingBackendThread();

	//GingkoOneProcess( 1256, 0 );

	//hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	//if( hEvent == NULL || hEvent == INVALID_HANDLE_VALUE )
	//{
	//	LOG(L"Error to CreateEvent. Error Code: %d.\n", GetLastError());
	//}

	PipeListenerServerThreadStart();

	if( DriverLoad() != ERROR_SUCCESS )
	{
		PipeListenerServerThreadStop();
		GkoUnInitialize();
		UninitCheapman();
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( ServiceStatusHandle, &ServiceStatus );
		return;
	}

	StartThread(DeviceIoThreadProc, NULL, &pBag );

	if( pBag == NULL )
	{
		LOG(L"Error to Start DeviceIoThreadProc. Error Code: %d.\n", GetLastError());
	}

	while (dwWaitRet != WAIT_OBJECT_0)	//do until receive a STOP events
	{
		dwWaitRet = WaitForMultipleObjects(2, g_hServiceEvents, FALSE, 2000000);
		if (dwWaitRet == WAIT_OBJECT_0 + 1)	// Receive PAUSE events
		{
			WaitForSingleObject( g_hServiceEvents[2], INFINITE); // Wait for CONTINUE events
		}

		//LOG(L"CALL PING\n");
		Current = GetTickCount();
		if( Previous == 0 )
		{
			Previous = GetTickCount();
		}

		if( Current - Previous > 10000 )
		{
			Previous = Current;
			if( TRUE )
			{
				tryTimes ++;
			}

			if( tryTimes > 10 )
			{
				tryTimes = 0;
			}
		}
	}

	//SERVICE_STATUS ServiceStatus = {0};
	ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	SetServiceStatus( ServiceStatusHandle, &ServiceStatus );
	LOG(_T("Service stop pending\n") );
	LOG(_T("Service stop pending\n") );
	UninitCheapman();

	StopThread( pBag );

	LOG(_T("Service stop pending\n") );
	PipeListenerServerThreadStop();

	//SetEvent( hEvent ); ///Exit the server thread.

	// Let's delete itself, after the service stopped
	//DeleteSvc();
	LOG(_T("Close the Service Event Handle\n") );
	CloseHandle( g_hServiceEvents[0] );
	CloseHandle( g_hServiceEvents[1] );
	CloseHandle( g_hServiceEvents[2] );

	LOG(_T("Unload Driver\n") );
	CloseBluefishDriver();
	DriverUnload();
	LOG(_T("Uninitialize Gingko System.\n") );
	GkoUnInitialize();
	
	LOG(_T("Shutdown LOG\n") );
	LOG_SHUTDOWN();

	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus( ServiceStatusHandle, &ServiceStatus );
}

// Start service
VOID WINAPI CmdStart (DWORD, LPTSTR* ) 
{
	DWORD status = 0; 
	DWORD specificError = 0;

	ServiceStatus.dwServiceType        = SERVICE_WIN32; 
	ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
	ServiceStatus.dwWin32ExitCode      = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	ServiceStatus.dwCheckPoint         = 0; 
	ServiceStatus.dwWaitHint           = 0; 

	ServiceStatusHandle = RegisterServiceCtrlHandler( 
		SERVICENAME, CmdCtrlHandler ); 

	if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
		return; 

	// Initialization complete - report running status. 
	ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	ServiceStatus.dwCheckPoint         = 0; 
	ServiceStatus.dwWaitHint           = 0; 

	if (!SetServiceStatus (ServiceStatusHandle, &ServiceStatus)) 
		status = GetLastError(); 
	else
	{
		//we do all things in the thread of _ServiceMain, wo also could do things here
		//while(true)
		//{
		//}

		// Start the main thread
		g_hServiceEvents[0] = CreateEvent( NULL, FALSE, FALSE, NULL );
		g_hServiceEvents[1] = CreateEvent( NULL, FALSE, FALSE, NULL );
		g_hServiceEvents[2] = CreateEvent( NULL, FALSE, FALSE, NULL );
		_beginthread( _ServiceMain, 0, NULL );
	}

	return; 
} 
//Main function
int _tmain(int argc, TCHAR **argv, TCHAR** envp)
{
	if (!CheckParam())
	{
		Usage();
		return 0;
	}

	if (argc >= 2)
	{
		if ( 0 == _tcscmp(argv[1], _T("-Install")) )
		{
			InstallCmdService();
		}
		else if ( 0 == _tcscmp(argv[1], _T("-Remove")) )
		{
			RemoveCmdService();
		}else if ( 0 == _tcscmp(argv[1], _T("-Hook")) )
		{
			//TestInject( _ttol( argv[2] ) );
		}
		return 0;
	}

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{ 
		{SERVICENAME, CmdStart}, 
		{NULL, NULL} 
	};

	//Connect this module to SCM 
	StartServiceCtrlDispatcher(DispatchTable);

	return 0;
}
