#ifndef __GINGKO_HOOK_H__
#define __GINGKO_HOOK_H__

//typedef enum _SYSTEM_INFORMATION_CLASS {
//	SystemBasicInformation, // 0 Y N
//	SystemProcessorInformation, // 1 Y N
//	SystemPerformanceInformation, // 2 Y N
//	SystemTimeOfDayInformation, // 3 Y N
//	SystemNotImplemented1, // 4 Y N
//	SystemProcessesAndThreadsInformation, // 5 Y N
//	SystemCallCounts, // 6 Y N
//	SystemConfigurationInformation, // 7 Y N
//	SystemProcessorTimes, // 8 Y N
//	SystemGlobalFlag, // 9 Y Y
//	SystemNotImplemented2, // 10 Y N
//	SystemModuleInformation, // 11 Y N
//	SystemLockInformation, // 12 Y N
//	SystemNotImplemented3, // 13 Y N
//	SystemNotImplemented4, // 14 Y N
//	SystemNotImplemented5, // 15 Y N
//	SystemHandleInformation, // 16 Y N
//	SystemObjectInformation, // 17 Y N
//	SystemPagefileInformation, // 18 Y N
//	SystemInstructionEmulationCounts, // 19 Y N
//	SystemInvalidInfoClass1, // 20
//	SystemCacheInformation, // 21 Y Y
//	SystemPoolTagInformation, // 22 Y N
//	SystemProcessorStatistics, // 23 Y N
//	SystemDpcInformation, // 24 Y Y
//	SystemNotImplemented6, // 25 Y N
//	SystemLoadImage, // 26 N Y
//	SystemUnloadImage, // 27 N Y
//	SystemTimeAdjustment, // 28 Y Y
//	SystemNotImplemented7, // 29 Y N
//	SystemNotImplemented8, // 30 Y N
//	SystemNotImplemented9, // 31 Y N
//	SystemCrashDumpInformation, // 32 Y N
//	SystemExceptionInformation, // 33 Y N
//	SystemCrashDumpStateInformation, // 34 Y Y/N
//	SystemKernelDebuggerInformation, // 35 Y N
//	SystemContextSwitchInformation, // 36 Y N
//	SystemRegistryQuotaInformation, // 37 Y Y
//	SystemLoadAndCallImage, // 38 N Y
//	SystemPrioritySeparation, // 39 N Y
//	SystemNotImplemented10, // 40 Y N
//	SystemNotImplemented11, // 41 Y N
//	SystemInvalidInfoClass2, // 42
//	SystemInvalidInfoClass3, // 43
//	SystemTimeZoneInformation, // 44 Y N
//	SystemLookasideInformation, // 45 Y N
//	SystemSetTimeSlipEvent, // 46 N Y
//	SystemCreateSession, // 47 N Y
//	SystemDeleteSession, // 48 N Y
//	SystemInvalidInfoClass4, // 49
//	SystemRangeStartInformation, // 50 Y N
//	SystemVerifierInformation, // 51 Y Y
//	SystemAddVerifier, // 52 N Y
//	SystemSessionProcessesInformation // 53 Y N
//} SYSTEM_INFORMATION_CLASS;
//
//NTSYSAPI
//NTSTATUS
//NTAPI
//ZwQuerySystemInformation(
//	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
//	IN OUT PVOID SystemInformation,
//	IN ULONG SystemInformationLength,
//	OUT PULONG ReturnLength OPTIONAL
//);
//
//typedef struct _SYSTEM_MODULE_INFORMATION { // Information Class 11
//	ULONG Reserved[2];
//	PVOID Base;
//	ULONG Size;
//	ULONG Flags;
//	USHORT Index;
//	USHORT Unknown;
//	USHORT LoadCount;
//	USHORT ModuleNameOffset;
//	CHAR ImageName[256];
//} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;
//
//typedef unsigned short      			WORD;
//typedef unsigned char BYTE;
//typedef unsigned long DWORD;
//typedef unsigned long *PDWORD;
//
//
//
//
//#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
//
//typedef IMAGE_OPTIONAL_HEADER32             IMAGE_OPTIONAL_HEADER;
//typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;
//
//#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
//#define IMAGE_DIRECTORY_ENTRY_EXPORT          0
//
//typedef unsigned char *PBYTE;
//
//
//#define SEC_IMAGE	0x01000000
//#define SEC_BASED 0x00200000
//typedef struct _SECTION_IMAGE_INFORMATION {
//  PVOID                   EntryPoint;
//  ULONG                   StackZeroBits;
//  ULONG                   StackReserved;
//  ULONG                   StackCommit;
//  ULONG                   ImageSubsystem;
//  WORD                    SubsystemVersionLow;
//  WORD                    SubsystemVersionHigh;
//  ULONG                   Unknown1;
//  ULONG                   ImageCharacteristics;
//  ULONG                   ImageMachineType;
//  ULONG                   Unknown2[3];
//} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;
//
//
//NTSYSAPI
//NTSTATUS
//NTAPI
//  ZwCreateSection(
//    OUT PHANDLE  SectionHandle,
//    IN ACCESS_MASK  DesiredAccess,
//    IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
//    IN PLARGE_INTEGER  MaximumSize OPTIONAL,
//    IN ULONG  SectionPageProtection,
//    IN ULONG  AllocationAttributes,
//    IN HANDLE  FileHandle OPTIONAL
//    ); 
//
//#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
//
//PVOID GetModlueBaseAdress(char* ModlueName);

#endif