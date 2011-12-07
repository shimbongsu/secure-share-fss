#include <ntifs.h>
#include <ntimage.h>
#include "gingko_hook.h"
#include "gingko_lib.h"

BOOLEAN gWasHooked = FALSE;

BOOLEAN gWasEATHooked = FALSE;

extern ULONG g_OriginalOpenClipboard;

ULONG GingkoEATHookByModuleAddress(IN ULONG ModuleBaseAddress, IN const char* ProcName, LONG_PTR lptrMyProcessAddress, IN unsigned int test);

#define PEBOFFSET 0x1B0 // PEB指针位于EPPROCESS中偏移0x1B0处

#define FLINKOFFSET 0xA0 // 进程的链表指针。这些信息可以通过kd得到。

//
//NTSTATUS GingkoHookImportsOfImage(PIMAGE_DOS_HEADER image_addr, HANDLE h_proc);
//
//
///////////////////////////////////////////////////////////
//// GingkoImageLoadNotify gets called when an image is loaded
//// into kernel or user space. At this point, you could 
//// filter your hook based on ProcessId or on the name of
//// of the image.
//VOID GingkoImageLoadNotify(IN PUNICODE_STRING  FullImageName,
//                       IN HANDLE  ProcessId, // Process where image is mapped
//					   IN PIMAGE_INFO  ImageInfo)
//{
//	UNICODE_STRING u_targetDLL;
//
//	KdPrint(("Image name: %wZ\n", FullImageName));
//	// Setup the name of the DLL to target
//	RtlInitUnicodeString(&u_targetDLL, L"\\WINDOWS\\system32\\user32.dll");
//
//	if (RtlCompareUnicodeString(FullImageName, &u_targetDLL, TRUE) == 0)
//	{
//		//GingkoHookImportsOfImage(ImageInfo->ImageBase, ProcessId);
//		if( gWasEATHooked == FALSE )
//		{
//			//g_OriginalOpenClipboard = GingkoEATHookByModuleAddress(ImageInfo->ImageBase, "OpenClipboard", MyOpenClipboard, 1  );
//		}
//	}
//
//}
//
//
//NTSTATUS GingkoHookImportsOfImage(PIMAGE_DOS_HEADER image_addr, HANDLE h_proc)
//{
//	PIMAGE_DOS_HEADER dosHeader;
//	PIMAGE_NT_HEADERS pNTHeader;
//	PIMAGE_IMPORT_DESCRIPTOR importDesc;
//	PIMAGE_IMPORT_BY_NAME p_ibn;
//	DWORD importsStartRVA;
//	DWORD exportStartRVA;
//	PDWORD pd_IAT, pd_INTO;
//	int count, index;
//	char *dll_name = NULL;
//	char *pc_dlltar = "user32.dll";
//	char *pc_fnctar = "OpenClipboard";
//	PMDL  p_mdl;
//	PDWORD MappedImTable;
//	DWORD d_sharedM = 0x7ffe0800;
//	DWORD d_sharedK = 0xffdf0800; 
//
//	// Little detour
//	unsigned char new_code[] = { 
//		0x90,                          // NOP make INT 3 to see
//		0xb8, 0xff, 0xff, 0xff, 0xff,  // mov eax, 0xffffffff
//		0xff, 0xe0                     // jmp eax
//	};
//	
//	dosHeader = (PIMAGE_DOS_HEADER) image_addr;
//
//	pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader,
//								dosHeader->e_lfanew );
//	
//	// First, verify that the e_lfanew field gave us a reasonable
//	// pointer, then verify the PE signature.
//	if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
//		return STATUS_INVALID_IMAGE_FORMAT;
//
//	importsStartRVA = pNTHeader->OptionalHeader.DataDirectory
//							[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
//
//	if (!importsStartRVA)
//		return STATUS_INVALID_IMAGE_FORMAT;
//
//	exportStartRVA = pNTHeader->OptionalHeader.DataDirectory
//							[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
//
//	if( !exportStartRVA )
//	{
//		return STATUS_INVALID_IMAGE_FORMAT;
//	}
//
//	importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (importsStartRVA + (DWORD) dosHeader);
//	
//
//	for (count = 0; importDesc[count].Characteristics != 0; count++)
//	{
//		dll_name = (char*) (importDesc[count].Name + (DWORD) dosHeader);
//		pd_IAT = (PDWORD)(((DWORD) dosHeader) + (DWORD)importDesc[count].FirstThunk);
//		pd_INTO = (PDWORD)(((DWORD) dosHeader) + (DWORD)importDesc[count].OriginalFirstThunk);
//
//		
//		for (index = 0; pd_IAT[index] != 0; index++)
//		{
//			// If this is an import by ordinal the high
//			// bit is set
//			if ((pd_INTO[index] & IMAGE_ORDINAL_FLAG) != IMAGE_ORDINAL_FLAG)
//			{
//				p_ibn = (PIMAGE_IMPORT_BY_NAME)(pd_INTO[index]+((DWORD) dosHeader));
//				if ((_stricmp(dll_name, pc_dlltar) == 0) && \
//					(strcmp(p_ibn->Name, pc_fnctar) == 0))
//				{
//					KdPrint(("Imports from DLL: %s. OpenClipboard: %s Address: %08x\n", dll_name, p_ibn->Name, pd_IAT[index]));
//
//					// Use the trick you already learned to map a different
//					// virtual address to the same physical page so no
//					// permission problems.
//					//
//					// Map the memory into our domain so we can change the permissions on the MDL
//
//					p_mdl = MmCreateMdl(NULL, &pd_IAT[index], 4);
//
//					if(!p_mdl)
//						return STATUS_UNSUCCESSFUL;
//
//					MmBuildMdlForNonPagedPool(p_mdl);
//
//					// Change the flags of the MDL
//					p_mdl->MdlFlags = p_mdl->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;
//
//					MappedImTable = MmMapLockedPagesSpecifyCache(
//										p_mdl,
//										KernelMode,
//										MmWriteCombined,
//										NULL,
//										FALSE,
//										NormalPagePriority 
//									);
//					if( MappedImTable == NULL )
//					{
//						KdPrint(("MmMapLockedPagesSpecifyCache return NULL.\n"));
//					}
//					//MappedImTable = MmMapLockedPages(p_mdl, KernelMode);
//					if( MappedImTable != NULL )
//					{
//						if (!gWasHooked)
//						{
//							// Writing the raw opcodes to memory
//							// used a kernel address that gets mapped
//							// into the address space of all processes
//							// thanks to Barnaby Jack
//							RtlCopyMemory((PVOID)d_sharedK, new_code, 8);
//							RtlCopyMemory((PVOID)(d_sharedK+2),(PVOID)&pd_IAT[index], 4);
//							gWasHooked = TRUE;
//						}
//
//						// Offset to the "new function"
//						*MappedImTable = d_sharedM;
//
//						// Free MDL
//						MmUnmapLockedPages(MappedImTable, p_mdl);
//					}
//					IoFreeMdl(p_mdl);
//
//				}
//			}
//		}
//	}
//	return STATUS_SUCCESS;
//}
//
//PVOID GetModlueBaseAdress(char* ModlueName)
//{
//	ULONG size,index;
//	PULONG buf;
//    NTSTATUS status;
//	PSYSTEM_MODULE_INFORMATION module;
//	PVOID driverAddress=0;
//
//	ZwQuerySystemInformation(SystemModuleInformation,&size, 0, &size);
//    if(NULL==(buf = (PULONG)ExAllocatePoolWithTag(PagedPool, size, GINGKO_POOL_TAG)))
//	{
//		DbgPrint("failed alloc memory failed  \n");
//		return 0;
//	}
//    status=ZwQuerySystemInformation(SystemModuleInformation,buf, size , 0);
//	if(!NT_SUCCESS( status ))
//	{
//       DbgPrint("failed  query\n");
//	   return 0;
//	}
//    module = (PSYSTEM_MODULE_INFORMATION)(( PULONG )buf + 1);
//	for (index = 0; index < *buf; index++)
//	{
//		KdPrint(("Module List: %s.\n", module[index].ImageName ) );
//		if (_stricmp(module[index].ImageName + module[index].ModuleNameOffset, ModlueName) == 0)  
//		{
//			driverAddress = module[index].Base;
//			DbgPrint("Module found at:%x\n",driverAddress);
//			break;
//		}
//	}
//	ExFreePool(buf);
//	return driverAddress;
//}
//
//ULONG GingkoEATHookByModuleAddress(IN ULONG ModuleBaseAddress, IN const char* ProcName, LONG_PTR lptrMyProcessAddress, IN unsigned int test) 
//{
//	HANDLE   hMod;
//    PUCHAR BaseAddress = NULL;
//	ULONG temp;
//	PHYSICAL_ADDRESS  BasePhysicalAddress = {0};
//	IMAGE_DOS_HEADER * dosheader;
//	IMAGE_OPTIONAL_HEADER * opthdr;
//    PIMAGE_EXPORT_DIRECTORY exports;
//	ULONG lptrOriginalProcAddress;
//
//    USHORT   index=0 ; 
//	ULONG addr=0L,i;
//	
//    PUCHAR pFuncName = NULL;
//	PULONG pAddressOfFunctions,pAddressOfNames;
//    PUSHORT pAddressOfNameOrdinals;
//	
//	temp = *((PULONG)ModuleBaseAddress);
//
//	///BasePhysicalAddress =  MmGetPhysicalAddress(ModuleBaseAddress);
//	//BaseAddress = BasePhysicalAddress.LowPart;
//	BaseAddress = (PUCHAR)ModuleBaseAddress; //GetModlueBaseAdress( ModuleName );
//    DbgPrint("Module BaseAddress is:%x\n", BaseAddress);
//
//	//return NULL;
//
//    hMod = BaseAddress;
//
//	if( !BaseAddress )
//	{
//		return (LONG_PTR)NULL;
//	}
//	
//	dosheader = (IMAGE_DOS_HEADER *)hMod;
//
//    opthdr =(IMAGE_OPTIONAL_HEADER *) ((BYTE*)hMod+dosheader->e_lfanew+24);
//    
//	exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE*) hMod+ opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
//  
//    pAddressOfFunctions = (ULONG*)((BYTE*)hMod+exports->AddressOfFunctions);   
//	
//	pAddressOfNames = (ULONG*)((BYTE*)hMod+exports->AddressOfNames);           
//	
//	pAddressOfNameOrdinals = (USHORT*)((BYTE*)hMod+exports->AddressOfNameOrdinals); 
//
//    for (i = 0; i < exports->NumberOfNames; i++) 
//	{
//		index = pAddressOfNameOrdinals[i];
//		pFuncName = (PUCHAR)( (BYTE*)hMod + pAddressOfNames[i]);
//		if( pFuncName != NULL )
//		{
//		
//			__try
//			{
//				if (_stricmp( (char*)pFuncName, ProcName) == 0)
//				{
//					addr=pAddressOfFunctions[index];
//					gWasEATHooked = TRUE;
//					break;
//				}
//			}
//			__except(EXCEPTION_EXECUTE_HANDLER)
//			{
//				KdPrint(("Error"));
//			}
//
//		}
//	}
//
//	if( addr )
//	{
//		if(test == 1){	
//			_asm
//			{
//				CLI					
//				MOV	EAX, CR0		
//				AND EAX, NOT 10000H 
//				MOV	CR0, EAX		
//			}	
//
//			lptrOriginalProcAddress = (ULONG)( (PULONG) hMod + pAddressOfFunctions[index] ); 
//			DbgPrint("OpenClipboard is:%x\n", lptrOriginalProcAddress );
//			pAddressOfFunctions[index] = ( ULONG )lptrMyProcessAddress; // - BaseAddress;
//			DbgPrint("MyOpenClipboard is:%x\n", lptrMyProcessAddress);
//			_asm 
//			{
//				MOV	EAX, CR0		
//				OR	EAX, 10000H			
//				MOV	CR0, EAX			
//				STI
//			}
//		}
//		else
//		{
//			_asm
//			{
//				CLI					
//				MOV	EAX, CR0		
//				AND EAX, NOT 10000H 
//				MOV	CR0, EAX		
//			}	
//
//			pAddressOfFunctions[index] = ( PCHAR )lptrOriginalProcAddress - BaseAddress;
//
//			_asm 
//			{
//				MOV	EAX, CR0		
//				OR	EAX, 10000H			
//				MOV	CR0, EAX			
//				STI					
//			}
//		}
//	}
//
//	return lptrOriginalProcAddress;
//}
//
////StartHook_And_Unhook是安装钩子和卸载钩子,如果	test==1表示安装,否则表示卸载
//LONG_PTR GingkoEATHookByName(IN const char* ModuleName, IN  const  char* ProcName, LONG_PTR lptrMyProcessAddress, IN unsigned int test) 
//{
//	HANDLE   hMod;
//    HANDLE BaseAddress = NULL;    
//	BaseAddress = (HANDLE) GetModlueBaseAdress( (char *)ModuleName );
//    DbgPrint("Module BaseAddress is:%x\n", ModuleName, BaseAddress);
//    hMod = BaseAddress;
//
//	if( !BaseAddress )
//	{
//		return 0L;
//	}
//
//	return GingkoEATHookByModuleAddress((ULONG)BaseAddress, ProcName, lptrMyProcessAddress, test );
//}
//
