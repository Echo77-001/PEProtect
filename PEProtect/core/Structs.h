#pragma once

#include<Windows.h>

enum class Register64 : INT8 {
    RAX = 0, RBX, RCX, RDX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    UNK = 0xFF
};

enum class OperandType : INT8 {
    none,
    reg,
    number,
    externalCall
};

enum JumpType {
    internal_func,
    extenral_func_iat
};

enum class OpCode : INT8 {
    mov = 0,
    movsx,
    movsxd,
    movzx,
    movdqa,
    movdqu,
    lea,
    add,
    sub,
    sbb,
    inc,
    dec,
    imul,
    neg,
    and_,
    or_,
    xor_,
    not_,
    shl,
    shr,
    sar,
    rol,
    ror,
    cmp,
    test,
    push,
    pop,
    xchg,
    nop,

    jmp,
    call,
    ret,
    jz,
    jnz,
    js,
    jns,
    jl,
    jnl,
    jle,
    jb,
    jnb,
    jbe,
    jnbe,

    bt,
    cmpxchg,
    cmovz,
    cmovnz,
    setz,
    setnz,
    setnbe,
    pushfq,
    cpuid,
    xgetbv,
    int_,
    int3,
    outsb,
    setbe,

    COUNT
};

#if 1

typedef struct _UNICODE_STRING
{
    WORD Length;
    WORD MaximumLength;
    WORD* Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _STRING
{
    WORD Length;
    WORD MaximumLength;
    char* Buffer;
} STRING, * PSTRING;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    PVOID Handle;
} CURDIR, * PCURDIR;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    WORD Flags;
    WORD Length;
    ULONG TimeStamp;
    STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    UCHAR Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    PVOID ConsoleHandle;
    ULONG ConsoleFlags;
    PVOID StandardInput;
    PVOID StandardOutput;
    PVOID StandardError;
    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PVOID Environment;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[32];
    ULONG EnvironmentSize;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

//typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION, * PRTL_CRITICAL_SECTION;
//
//typedef struct _RTL_CRITICAL_SECTION_DEBUG
//{
//    WORD Type;
//    WORD CreatorBackTraceIndex;
//    PRTL_CRITICAL_SECTION CriticalSection;
//    LIST_ENTRY ProcessLocksList;
//    ULONG EntryCount;
//    ULONG ContentionCount;
//    ULONG Flags;
//    WORD CreatorBackTraceIndexHigh;
//    WORD SpareUSHORT;
//} RTL_CRITICAL_SECTION_DEBUG, * PRTL_CRITICAL_SECTION_DEBUG;

//typedef struct _RTL_CRITICAL_SECTION
//{
//    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
//    LONG LockCount;
//    LONG RecursionCount;
//    PVOID OwningThread;
//    PVOID LockSemaphore;
//    ULONG SpinCount;
//} RTL_CRITICAL_SECTION, * PRTL_CRITICAL_SECTION;

typedef struct _PEB_FREE_BLOCK
{
    struct _PEB_FREE_BLOCK* Next;
    ULONG Size;
} PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;

//typedef struct _LARGE_INTEGER
//{
//    union
//    {
//        struct
//        {
//            ULONG LowPart;
//            LONG HighPart;
//        };
//        INT64 QuadPart;
//    };
//} LARGE_INTEGER, * PLARGE_INTEGER;

//typedef struct _ULARGE_INTEGER
//{
//    union
//    {
//        struct
//        {
//            ULONG LowPart;
//            ULONG HighPart;
//        };
//        UINT64 QuadPart;
//    };
//} ULARGE_INTEGER, * PULARGE_INTEGER;



typedef struct _CUSTOM_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    WORD LoadCount;
    WORD TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
} CUSTOM_LDR_DATA_TABLE_ENTRY, * PCUSTOM_LDR_DATA_TABLE_ENTRY;

//typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
//    WORD   e_magic;                     // Magic number
//    WORD   e_cblp;                      // Bytes on last page of file
//    WORD   e_cp;                        // Pages in file
//    WORD   e_crlc;                      // Relocations
//    WORD   e_cparhdr;                   // Size of header in paragraphs
//    WORD   e_minalloc;                  // Minimum extra paragraphs needed
//    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
//    WORD   e_ss;                        // Initial (relative) SS value
//    WORD   e_sp;                        // Initial SP value
//    WORD   e_csum;                      // Checksum
//    WORD   e_ip;                        // Initial IP value
//    WORD   e_cs;                        // Initial (relative) CS value
//    WORD   e_lfarlc;                    // File address of relocation table
//    WORD   e_ovno;                      // Overlay number
//    WORD   e_res[4];                    // Reserved words
//    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
//    WORD   e_oeminfo;                   // OEM information; e_oemid specific
//    WORD   e_res2[10];                  // Reserved words
//    LONG   e_lfanew;                    // File address of new exe header
//} IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;


typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    WORD LoadCount;
    WORD TlsIndex;
    union
    {
        LIST_ENTRY HashLinks;
        struct
        {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union
    {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    void* EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

//typedef struct _IMAGE_FILE_HEADER {
//    WORD    Machine;
//    WORD    NumberOfSections;
//    DWORD   TimeDateStamp;
//    DWORD   PointerToSymbolTable;
//    DWORD   NumberOfSymbols;
//    WORD    SizeOfOptionalHeader;
//    WORD    Characteristics;
//} IMAGE_FILE_HEADER, * PIMAGE_FILE_HEADER;

//typedef struct _IMAGE_DATA_DIRECTORY {
//    DWORD   VirtualAddress;
//    DWORD   Size;
//} IMAGE_DATA_DIRECTORY, * PIMAGE_DATA_DIRECTORY;

//typedef struct _IMAGE_OPTIONAL_HEADER64 {
//    WORD        Magic;
//    BYTE        MajorLinkerVersion;
//    BYTE        MinorLinkerVersion;
//    DWORD       SizeOfCode;
//    DWORD       SizeOfInitializedData;
//    DWORD       SizeOfUninitializedData;
//    DWORD       AddressOfEntryPoint;
//    DWORD       BaseOfCode;
//    ULONGLONG   ImageBase;
//    DWORD       SectionAlignment;
//    DWORD       FileAlignment;
//    WORD        MajorOperatingSystemVersion;
//    WORD        MinorOperatingSystemVersion;
//    WORD        MajorImageVersion;
//    WORD        MinorImageVersion;
//    WORD        MajorSubsystemVersion;
//    WORD        MinorSubsystemVersion;
//    DWORD       Win32VersionValue;
//    DWORD       SizeOfImage;
//    DWORD       SizeOfHeaders;
//    DWORD       CheckSum;
//    WORD        Subsystem;
//    WORD        DllCharacteristics;
//    ULONGLONG   SizeOfStackReserve;
//    ULONGLONG   SizeOfStackCommit;
//    ULONGLONG   SizeOfHeapReserve;
//    ULONGLONG   SizeOfHeapCommit;
//    DWORD       LoaderFlags;
//    DWORD       NumberOfRvaAndSizes;
//    IMAGE_DATA_DIRECTORY DataDirectory[16];
//} IMAGE_OPTIONAL_HEADER64, * PIMAGE_OPTIONAL_HEADER64;

//typedef struct _IMAGE_NT_HEADERS64 {
//    DWORD Signature;
//    IMAGE_FILE_HEADER FileHeader;
//    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
//} IMAGE_NT_HEADERS64, * PIMAGE_NT_HEADERS64;

//typedef PIMAGE_NT_HEADERS64                 PIMAGE_NT_HEADERS;

//typedef struct _IMAGE_EXPORT_DIRECTORY {
//    DWORD   Characteristics;
//    DWORD   TimeDateStamp;
//    WORD    MajorVersion;
//    WORD    MinorVersion;
//    DWORD   Name;
//    DWORD   Base;
//    DWORD   NumberOfFunctions;
//    DWORD   NumberOfNames;
//    DWORD   AddressOfFunctions;     // RVA from base of image
//    DWORD   AddressOfNames;         // RVA from base of image
//    DWORD   AddressOfNameOrdinals;  // RVA from base of image
//} IMAGE_EXPORT_DIRECTORY, * PIMAGE_EXPORT_DIRECTORY;

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct _PEB
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
    ULONG ImageUsesLargePages : 1;
    ULONG IsProtectedProcess : 1;
    ULONG IsLegacyProcess : 1;
    ULONG IsImageDynamicallyRelocated : 1;
    ULONG SpareBits : 4;
    PVOID Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    ULONG CrossProcessFlags;
    ULONG ProcessInJob : 1;
    ULONG ProcessInitializing : 1;
    ULONG ReservedBits0 : 30;
    union
    {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG SpareUlong;
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID HotpatchInformation;
    void** ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
    LARGE_INTEGER CriticalSectionTimeout;
    ULONG HeapSegmentReserve;
    ULONG HeapSegmentCommit;
    ULONG HeapDeCommitTotalFreeThreshold;
    ULONG HeapDeCommitFreeBlockThreshold;
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    void** ProcessHeaps;
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
    PRTL_CRITICAL_SECTION LoaderLock;
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    WORD OSBuildNumber;
    WORD OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[34];
    PVOID PostProcessInitRoutine;
    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];
    ULONG SessionId;
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;
    UNICODE_STRING CSDVersion;
    void* ActivationContextData;
    void* ProcessAssemblyStorageMap;
    void* SystemDefaultActivationContextData;
    void* SystemAssemblyStorageMap;
    ULONG MinimumStackCommit;
    void* FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[4];
    ULONG FlsHighIndex;
    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
} PEB, * PPEB;

#endif