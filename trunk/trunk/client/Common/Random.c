/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions of
 this file are Copyright (c) 2003-2008 TrueCrypt Foundation and are governed
 by the TrueCrypt License 2.6 the full text of which is contained in the
 file License.txt included in TrueCrypt binary and source code distribution
 packages. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "Tcdefs.h"
#include "Crc.h"
#include "Random.h"
#include "Apidrvr.h"

unsigned __int8 buffer[RNG_POOL_SIZE];
unsigned char *pRandPool = NULL;
static BOOL bRandDidInit = FALSE;
int nRandIndex = 0, randPoolReadIndex = 0;
int HashFunction = DEFAULT_HASH_ALGORITHM;
BOOL bDidSlowPoll = FALSE;	/* We do the slow poll only once */
BOOL volatile bFastPollEnabled = TRUE;	/* Used to reduce CPU load when performing benchmarks */
BOOL volatile bRandmixEnabled = TRUE;	/* Used to reduce CPU load when performing benchmarks */

/* Macro to add a single byte to the pool */
#define RandaddByte(x) {\
	if (nRandIndex == RNG_POOL_SIZE) nRandIndex = 0;\
	pRandPool[nRandIndex] = (unsigned char) ((unsigned char)x + pRandPool[nRandIndex]); \
	if (nRandIndex % RANDMIX_BYTE_INTERVAL == 0) Randmix();\
	nRandIndex++; \
	}

/* Macro to add four bytes to the pool */
#define RandaddInt32(x) RandAddInt((unsigned __int32)x);

void RandAddInt (unsigned __int32 x)
{
	RandaddByte(x); 
	RandaddByte((x >> 8)); 
	RandaddByte((x >> 16));
	RandaddByte((x >> 24));
}

#include <tlhelp32.h>


/* Variables for thread control, the thread is used to gather up info about
   the system in in the background */
CRITICAL_SECTION critRandProt;	/* The critical section */
BOOL volatile bThreadTerminate = FALSE;	/* This variable is shared among thread's so its made volatile */

/* Network library handle for the SlowPoll function */
HANDLE hNetAPI32 = NULL;

// CryptoAPI
HCRYPTPROV hCryptProv;


/* Init the random number generator, setup the hooks, and start the thread */
int Randinit ()
{
	if(bRandDidInit) return 0;

	InitializeCriticalSection (&critRandProt);

	bRandDidInit = TRUE;

	pRandPool = (unsigned char *) TCalloc (RANDOMPOOL_ALLOCSIZE);
	if (pRandPool == NULL)
		goto error;
	else
		memset (pRandPool, 0, RANDOMPOOL_ALLOCSIZE);

	VirtualLock (pRandPool, RANDOMPOOL_ALLOCSIZE);

	if (!CryptAcquireContext (&hCryptProv, NULL, NULL, PROV_EC_ECDSA_FULL /*PROV_RSA_FULL*/, 0)
		&& !CryptAcquireContext (&hCryptProv, NULL, NULL, PROV_EC_ECDSA_FULL/*PROV_RSA_FULL*/, CRYPT_NEWKEYSET))
	{
		hCryptProv = 0;
	}

	if (_beginthread (ThreadSafeThreadFunction, 0, NULL) == -1)
		goto error;

	return 0;

error:
	Randfree ();
	return 1;
}

/* Close everything down, including the thread which is closed down by
   setting a flag which eventually causes the thread function to exit */
void
Randfree ()
{
	if (bRandDidInit == FALSE)
		return;

	EnterCriticalSection (&critRandProt);

	bThreadTerminate = TRUE;

	LeaveCriticalSection (&critRandProt);

	for (;;)
	{
		Sleep (250);
		EnterCriticalSection (&critRandProt);
		if (bThreadTerminate == FALSE)
		{
			LeaveCriticalSection (&critRandProt);
			break;
		}
		LeaveCriticalSection (&critRandProt);
	}

	if (hNetAPI32 != 0)
	{
		FreeLibrary (hNetAPI32);
		hNetAPI32 = NULL;
	}

	if (hCryptProv)
	{
		CryptReleaseContext (hCryptProv, 0);
		hCryptProv = 0;
	}

	bThreadTerminate = FALSE;
	DeleteCriticalSection (&critRandProt);

	bRandDidInit = FALSE;

	if (pRandPool != NULL)
	{
		burn (pRandPool, RANDOMPOOL_ALLOCSIZE);
		TCfree (pRandPool);
		pRandPool = NULL;
	}

	nRandIndex = 0;
}


void RandSetHashFunction (int hash_algo_id)
{
	HashFunction = hash_algo_id;
}

int RandGetHashFunction (void)
{
	return HashFunction;
}

/* The random pool mixing function */
BOOL Randmix ()
{
	if (bRandmixEnabled)
	{
		unsigned char hashOutputBuffer [MAX_DIGESTSIZE];
		WHIRLPOOL_CTX	wctx;
		RMD160_CTX		rctx;
		sha512_ctx		sctx;
		int poolIndex, digestIndex, digestSize;

		switch (HashFunction)
		{
		case RIPEMD160:
			digestSize = RIPEMD160_DIGESTSIZE;
			break;

		case SHA512:
			digestSize = SHA512_DIGESTSIZE;
			break;

		case WHIRLPOOL:
			digestSize = WHIRLPOOL_DIGESTSIZE;
			break;

		default:
			return FALSE;
		}

		if (RNG_POOL_SIZE % digestSize)
			TC_THROW_FATAL_EXCEPTION;

		for (poolIndex = 0; poolIndex < RNG_POOL_SIZE; poolIndex += digestSize)		
		{
			/* Compute the message digest of the entire pool using the selected hash function. */
			switch (HashFunction)
			{
			case RIPEMD160:
				RMD160Init(&rctx);
				RMD160Update(&rctx, pRandPool, RNG_POOL_SIZE);
				RMD160Final(hashOutputBuffer, &rctx);
				break;

			case SHA512:
				sha512_begin (&sctx);
				sha512_hash (pRandPool, RNG_POOL_SIZE, &sctx);
				sha512_end (hashOutputBuffer, &sctx);
				break;

			case WHIRLPOOL:
				WHIRLPOOL_init (&wctx);
				WHIRLPOOL_add (pRandPool, RNG_POOL_SIZE * 8, &wctx);
				WHIRLPOOL_finalize (&wctx, hashOutputBuffer);
				break;

			default:		
				// Unknown/wrong ID
				TC_THROW_FATAL_EXCEPTION;
			}

			/* XOR the resultant message digest to the pool at the poolIndex position. */
			for (digestIndex = 0; digestIndex < digestSize; digestIndex++)
			{
				pRandPool [poolIndex + digestIndex] ^= hashOutputBuffer [digestIndex];
			}
		}

		/* Prevent leaks */
		burn (hashOutputBuffer, MAX_DIGESTSIZE);	
		switch (HashFunction)
		{
		case RIPEMD160:
			burn (&rctx, sizeof(rctx));		
			break;

		case SHA512:
			burn (&sctx, sizeof(sctx));		
			break;

		case WHIRLPOOL:
			burn (&wctx, sizeof(wctx));		
			break;

		default:		
			// Unknown/wrong ID
			TC_THROW_FATAL_EXCEPTION;
		}
	}
	return TRUE;
}

/* Add a buffer to the pool */
void
RandaddBuf (void *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		RandaddByte (((unsigned char *) buf)[i]);
	}
}

BOOL
RandpeekBytes (unsigned char *buf, int len)
{
	if (!bRandDidInit)
		return FALSE;

	if (len > RNG_POOL_SIZE)
	{
		//Error ("ERR_NOT_ENOUGH_RANDOM_DATA");	
		len = RNG_POOL_SIZE;
	}

	EnterCriticalSection (&critRandProt);
	memcpy (buf, pRandPool, len);
	LeaveCriticalSection (&critRandProt);

	return TRUE;
}


/* Get len random bytes from the pool (max. RNG_POOL_SIZE bytes per a single call) */
BOOL
RandgetBytes (unsigned char *buf, int len, BOOL forceSlowPoll)
{
	int i;
	BOOL ret = TRUE;

	if (HashFunction == 0)
		return FALSE;

	EnterCriticalSection (&critRandProt);

	if (bDidSlowPoll == FALSE || forceSlowPoll)
	{
		if (!SlowPoll ())
			ret = FALSE;
		else
			bDidSlowPoll = TRUE;
	}

	if (!FastPoll ())
		ret = FALSE;

	/* There's never more than RNG_POOL_SIZE worth of randomess */
	if (len > RNG_POOL_SIZE)
	{
		//Error ("ERR_NOT_ENOUGH_RANDOM_DATA");	
		len = RNG_POOL_SIZE;
		return FALSE;
	}

	// Requested number of bytes is copied from pool to output buffer,
	// pool is rehashed, and output buffer is XORed with new data from pool
	for (i = 0; i < len; i++)
	{
		buf[i] = pRandPool[randPoolReadIndex++];
		if (randPoolReadIndex == RNG_POOL_SIZE) randPoolReadIndex = 0;
	}

	/* Invert the pool */
	for (i = 0; i < RNG_POOL_SIZE / 4; i++)
	{
		((unsigned __int32 *) pRandPool)[i] = ~((unsigned __int32 *) pRandPool)[i];
	}

	// Mix the pool
	if (!FastPoll ())
		ret = FALSE;

	// XOR the current pool content into the output buffer to prevent pool state leaks
	for (i = 0; i < len; i++)
	{
		buf[i] ^= pRandPool[randPoolReadIndex++];
		if (randPoolReadIndex == RNG_POOL_SIZE) randPoolReadIndex = 0;
	}

	LeaveCriticalSection (&critRandProt);
	return ret;
}

/* This is the thread function which will poll the system for randomness */
void
ThreadSafeThreadFunction (void *dummy)
{
	HANDLE hEvent;
	if (dummy);		/* Remove unused parameter warning */

	hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	if( hEvent != NULL && hEvent != INVALID_HANDLE_VALUE )
	{
		while ( WAIT_TIMEOUT == WaitForSingleObject( hEvent, FASTPOLL_INTERVAL * 1000 ) )
		{
			EnterCriticalSection (&critRandProt);

			if (bThreadTerminate)
			{
				bThreadTerminate = FALSE;
				LeaveCriticalSection (&critRandProt);
				break;
			}
			else if (bFastPollEnabled)
			{
				FastPoll ();
			}

			LeaveCriticalSection (&critRandProt);

		}
	}
}

/* Type definitions for function pointers to call NetAPI32 functions */

typedef
  DWORD (WINAPI * NETSTATISTICSGET) (LPWSTR szServer, LPWSTR szService,
				     DWORD dwLevel, DWORD dwOptions,
				     LPBYTE * lpBuffer);
typedef
  DWORD (WINAPI * NETAPIBUFFERSIZE) (LPVOID lpBuffer, LPDWORD cbBuffer);
typedef
  DWORD (WINAPI * NETAPIBUFFERFREE) (LPVOID lpBuffer);

NETSTATISTICSGET pNetStatisticsGet = NULL;
NETAPIBUFFERSIZE pNetApiBufferSize = NULL;
NETAPIBUFFERFREE pNetApiBufferFree = NULL;


/* This is the slowpoll function which gathers up network/hard drive
   performance data for the random pool */
BOOL SlowPoll (void)
{
	int i;
	static int isWorkstation = -1;
	static int cbPerfData = 0x10000;
	HANDLE hDevice;
	LPBYTE lpBuffer;
	DWORD dwSize, status;
	LPWSTR lpszLanW, lpszLanS;
	int nDrive;

	/* Find out whether this is an NT server or workstation if necessary */
	if (isWorkstation == -1)
	{
		HKEY hKey;

		if (RegOpenKeyExA (HKEY_LOCAL_MACHINE,
		       "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
				  0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			unsigned char szValue[32];
			dwSize = sizeof (szValue);

			isWorkstation = TRUE;
			status = RegQueryValueExA (hKey, "ProductType", 0, NULL,
						  szValue, &dwSize);

			if (status == ERROR_SUCCESS && _stricmp ((char *) szValue, "WinNT"))
				/* Note: There are (at least) three cases for
				   ProductType: WinNT = NT Workstation,
				   ServerNT = NT Server, LanmanNT = NT Server
				   acting as a Domain Controller */
				isWorkstation = FALSE;

			RegCloseKey (hKey);
		}
	}
	/* Initialize the NetAPI32 function pointers if necessary */
	if (hNetAPI32 == NULL)
	{
		/* Obtain a handle to the module containing the Lan Manager
		   functions */
		hNetAPI32 = LoadLibraryA ("NETAPI32.DLL");
		if (hNetAPI32 != NULL)
		{
			/* Now get pointers to the functions */
			pNetStatisticsGet = (NETSTATISTICSGET) GetProcAddress (hNetAPI32,
							"NetStatisticsGet");
			pNetApiBufferSize = (NETAPIBUFFERSIZE) GetProcAddress (hNetAPI32,
							"NetApiBufferSize");
			pNetApiBufferFree = (NETAPIBUFFERFREE) GetProcAddress (hNetAPI32,
							"NetApiBufferFree");

			/* Make sure we got valid pointers for every NetAPI32
			   function */
			if (pNetStatisticsGet == NULL ||
			    pNetApiBufferSize == NULL ||
			    pNetApiBufferFree == NULL)
			{
				/* Free the library reference and reset the
				   static handle */
				FreeLibrary (hNetAPI32);
				hNetAPI32 = NULL;
			}
		}
	}

	/* Get network statistics.  Note: Both NT Workstation and NT Server
	   by default will be running both the workstation and server
	   services.  The heuristic below is probably useful though on the
	   assumption that the majority of the network traffic will be via
	   the appropriate service */
	lpszLanW = (LPWSTR) WIDE ("LanmanWorkstation");
	lpszLanS = (LPWSTR) WIDE ("LanmanServer");
	if (hNetAPI32 &&
	    pNetStatisticsGet (NULL,
			       isWorkstation ? lpszLanW : lpszLanS,
			       0, 0, &lpBuffer) == 0)
	{
		pNetApiBufferSize (lpBuffer, &dwSize);
		RandaddBuf ((unsigned char *) lpBuffer, dwSize);
		pNetApiBufferFree (lpBuffer);
	}

	/* Get disk I/O statistics for all the hard drives */
	for (nDrive = 0;; nDrive++)
	{
		DISK_PERFORMANCE diskPerformance;
		char szDevice[24];

		/* Check whether we can access this device */
		sprintf_s( szDevice, 24, "\\\\.\\PhysicalDrive%d", nDrive);
		hDevice = CreateFileA (szDevice, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
				      NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
			break;


		/* Note: This only works if you have turned on the disk
		   performance counters with 'diskperf -y'.  These counters
		   are off by default */
		if (DeviceIoControl (hDevice, IOCTL_DISK_PERFORMANCE, NULL, 0,
				&diskPerformance, sizeof (DISK_PERFORMANCE),
				     &dwSize, NULL))
		{
			RandaddBuf ((unsigned char *) &diskPerformance, dwSize);
		}
		CloseHandle (hDevice);
	}

	// CryptoAPI
	for (i = 0; i < 100; i++)
	{
		if (hCryptProv && CryptGenRandom(hCryptProv, sizeof (buffer), buffer)) 
			RandaddBuf (buffer, sizeof (buffer));
	}

	burn(buffer, sizeof (buffer));
	Randmix();
	return TRUE;
}


/* This is the fastpoll function which gathers up info by calling various api's */
BOOL FastPoll (void)
{
	int nOriginalRandIndex = nRandIndex;
	static BOOL addedFixedItems = FALSE;
	FILETIME creationTime, exitTime, kernelTime, userTime;
	SIZE_T minimumWorkingSetSize = 0, maximumWorkingSetSize = 0;
	LARGE_INTEGER performanceCount;
	MEMORYSTATUS memoryStatus;
	HANDLE handle;
	POINT point;

	/* Get various basic pieces of system information */
	RandaddInt32 (GetActiveWindow ());	/* Handle of active window */
	RandaddInt32 (GetCapture ());	/* Handle of window with mouse
					   capture */
	RandaddInt32 (GetClipboardOwner ());	/* Handle of clipboard owner */
	RandaddInt32 (GetClipboardViewer ());	/* Handle of start of
						   clpbd.viewer list */
	RandaddInt32 (GetCurrentProcess ());	/* Pseudohandle of current
						   process */
	RandaddInt32 (GetCurrentProcessId ());	/* Current process ID */
	RandaddInt32 (GetCurrentThread ());	/* Pseudohandle of current
						   thread */
	RandaddInt32 (GetCurrentThreadId ());	/* Current thread ID */
	RandaddInt32 (GetCurrentTime ());	/* Milliseconds since Windows
						   started */
	//RandaddInt32 (GetDesktopWindow ());	/* Handle of desktop window */
	//RandaddInt32 (GetFocus ());	/* Handle of window with kb.focus */
	//RandaddInt32 (GetInputState ());	/* Whether sys.queue has any events */
	//RandaddInt32 (GetMessagePos ());	/* Cursor pos.for last message */
	//RandaddInt32 (GetMessageTime ());	/* 1 ms time for last message */
	//RandaddInt32 (GetOpenClipboardWindow ());	/* Handle of window with
	//						   clpbd.open */
	//RandaddInt32 (GetProcessHeap ());	/* Handle of process heap */
	//RandaddInt32 (GetProcessWindowStation ());	/* Handle of procs
	//						   window station */
	//RandaddInt32 (GetQueueStatus (QS_ALLEVENTS));	/* Types of events in
	//						   input queue */

	/* Get multiword system information */
	GetCaretPos (&point);	/* Current caret position */
	RandaddBuf ((unsigned char *) &point, sizeof (POINT));
	GetCursorPos (&point);	/* Current mouse cursor position */
	RandaddBuf ((unsigned char *) &point, sizeof (POINT));

	/* Get percent of memory in use, bytes of physical memory, bytes of
	   free physical memory, bytes in paging file, free bytes in paging
	   file, user bytes of address space, and free user bytes */
	memoryStatus.dwLength = sizeof (MEMORYSTATUS);
	GlobalMemoryStatus (&memoryStatus);
	RandaddBuf ((unsigned char *) &memoryStatus, sizeof (MEMORYSTATUS));

	/* Get thread and process creation time, exit time, time in kernel
	   mode, and time in user mode in 100ns intervals */
	handle = GetCurrentThread ();
	GetThreadTimes (handle, &creationTime, &exitTime, &kernelTime, &userTime);
	RandaddBuf ((unsigned char *) &creationTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &exitTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &kernelTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &userTime, sizeof (FILETIME));
	handle = GetCurrentProcess ();
	GetProcessTimes (handle, &creationTime, &exitTime, &kernelTime, &userTime);
	RandaddBuf ((unsigned char *) &creationTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &exitTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &kernelTime, sizeof (FILETIME));
	RandaddBuf ((unsigned char *) &userTime, sizeof (FILETIME));

	/* Get the minimum and maximum working set size for the current
	   process */
	GetProcessWorkingSetSize (handle, &minimumWorkingSetSize,
				  &maximumWorkingSetSize);
	RandaddInt32 (minimumWorkingSetSize);
	RandaddInt32 (maximumWorkingSetSize);

	/* The following are fixed for the lifetime of the process so we only
	   add them once */
	if (addedFixedItems == 0)
	{
		STARTUPINFO startupInfo;

		/* Get name of desktop, console window title, new window
		   position and size, window flags, and handles for stdin,
		   stdout, and stderr */
		startupInfo.cb = sizeof (STARTUPINFO);
		GetStartupInfo (&startupInfo);
		RandaddBuf ((unsigned char *) &startupInfo, sizeof (STARTUPINFO));
		addedFixedItems = TRUE;
	}
	/* The docs say QPC can fail if appropriate hardware is not
	   available. It works on 486 & Pentium boxes, but hasn't been tested
	   for 386 or RISC boxes */
	if (QueryPerformanceCounter (&performanceCount))
		RandaddBuf ((unsigned char *) &performanceCount, sizeof (LARGE_INTEGER));
	else
	{
		/* Millisecond accuracy at best... */
		DWORD dwTicks = GetTickCount ();
		RandaddBuf ((unsigned char *) &dwTicks, sizeof (dwTicks));
	}

	// CryptoAPI
	if (hCryptProv && CryptGenRandom(hCryptProv, sizeof (buffer), buffer)) 
		RandaddBuf (buffer, sizeof (buffer));

	/* Apply the pool mixing function */
	Randmix();

	/* Restore the original pool cursor position. If this wasn't done, mouse coordinates
	   could be written to a limited area of the pool, especially when moving the mouse
	   uninterruptedly. The severity of the problem would depend on the length of data
	   written by FastPoll (if it was equal to the size of the pool, mouse coordinates
	   would be written only to a particular 4-byte area, whenever moving the mouse
	   uninterruptedly). */
	nRandIndex = nOriginalRandIndex;

	return TRUE;
}

