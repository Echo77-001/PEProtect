#pragma once

#include<Windows.h>
#include<vector>
#include<cstdio>
#include<map>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))


// UINT64 hWnd, void* lpText, void* lpCaption, UINT32 uType
typedef int(*msgBoxA)(UINT64, void*, void*, UINT32);

// lib name
typedef UINT64(*loadLibrFn)(const char*);
// addr
typedef bool(*freeLibrFn)(void*);

typedef UINT64(*GetCurrentProcessFn)();

// NtAllocateVirtualMemory
typedef UINT64(*NtAllocateVirtualMemoryFn) (
    UINT64    ProcessHandle,
    UINT64 BaseAddress,
    ULONG_PTR ZeroBits,
    UINT64   RegionSize,
    ULONG     AllocationType,
    ULONG     Protect
    );

typedef UINT32(*NtFreeVirtualMemoryFn) (
    UINT64    ProcessHandle,
    UINT64 BaseAddress,
    UINT64   RegionSize,
    ULONG     FreeType
    );

typedef void* (__fastcall* malloc_prototype)(int);
typedef void(__fastcall* free_prototype)(void*);

struct CustomSection {
    void* data; // ptr to page
    char secName[8];
    UINT32 virtualSize, rawSize, characteristics, virtAddr;
    UINT64 RegionSize;
};

struct FuncToImport {
    UINT32 id; // (rva in original file)
    UINT64 addr;
};

struct DllToImport {
    FuncToImport* funcs = 0;
    UINT32 funcsToImportCount;
};

typedef void* (__stdcall* getPebFn)();

struct LogicalBlock {
    std::vector<uint8_t> instructions;
};

struct GeneralContext {
    UINT8 sectionCount;
    getPebFn getPeb;
    msgBoxA messaageBox;
    NtAllocateVirtualMemoryFn AllocateVirtualMemory;
    NtFreeVirtualMemoryFn FreeVirtualMemory;
    // NtFreeVirtualMemory
    GetCurrentProcessFn getCurrentProc;
    loadLibrFn LoadLib;
    freeLibrFn FreeLib;

    bool user32WasLoaded;

    UINT64 kernel32Base, user32Base, ntdllBase;

    DllToImport* dllsToImport = 0;
    UINT32 dllsToImportCount;

    UINT32 entryBlockID;

    UINT32 sizeofImage, entryPointRVA;
    UINT64 imageBase;

    // CustomSection *sections = 0;

    std::map<DWORD, LogicalBlock> logicalBlocks;
};

struct DataDirectoryInfo {
    IMAGE_DATA_DIRECTORY raw;
    UINT32 rva;
};

#pragma pack(push, 1)

struct alignas(1) VM_Memory {
    uint8_t  base;
    uint8_t  index;
    uint8_t  scale;
    uint32_t  offset;
};

struct alignas(1) BasicInstruction {
    uint8_t opCode;
    uint8_t size;
    uint8_t type0;
    uint8_t type1;

    union {
        uint8_t   reg0;
        VM_Memory mem0;
    };

    union {
        uint8_t   reg1;
        VM_Memory mem1;
    };
};
#pragma pack(pop)

enum class Register64 : INT8 {
    RAX = 0, RBX, RCX, RDX, RSI, RDI, RBP, RSP,
    R8, R9, R10, R11, R12, R13, R14, R15,
    UNK = 0xFF
};

enum class OpCode {
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

enum class OperandType : UINT32 {
    memory,
    reg,
    imm
};


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

typedef struct _PEB_FREE_BLOCK
{
    struct _PEB_FREE_BLOCK* Next;
    ULONG Size;
} PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;

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
