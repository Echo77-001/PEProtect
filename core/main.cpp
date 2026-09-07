#include"Structs.h"
#include<Windows.h>
#include<string>

#include<cstdio>
#include <vector>

#include<iostream>
#include <map>
#include<iomanip>
#include<string_view>
#include <sstream>

#include<zstd.h>

void LogMessage(const char* label, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int p = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    MessageBoxA(0, buffer, label, MB_OK);
}

std::vector<uint8_t> decompress_data(const std::vector<uint8_t>& compressed_data, size_t original_size) {
    std::vector<uint8_t> decompressed_buffer(original_size);
    size_t const decompressed_size = ZSTD_decompress(
        decompressed_buffer.data(), decompressed_buffer.size(),
        compressed_data.data(), compressed_data.size()
    );

    if (ZSTD_isError(decompressed_size)) {
        throw std::runtime_error(std::string("ZSTD decompression error: ") + ZSTD_getErrorName(decompressed_size));
    }

    if (decompressed_size != original_size) {
        throw std::runtime_error("ZSTD decompression error: Size mismatch!");
    }

    return decompressed_buffer;
}

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

DWORD AlignUp(UINT32 size, UINT32 alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

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
    FuncToImport *funcs=0;
    UINT32 funcsToImportCount;
};

typedef void* (__stdcall* getPebFn)();

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
    
    DllToImport *dllsToImport=0;
    UINT32 dllsToImportCount;
    
    UINT32 entryBlockID;

    UINT32 sizeofImage, entryPointRVA;
    UINT64 imageBase;
    
    CustomSection *sections = 0;
};

extern "C" void* globalCtx = 0;

GeneralContext* ctxt;

struct VMSettings {
    UINT64 SizeOfStackReserve;
};

typedef void* (__fastcall* malloc_prototype)(int);
typedef void(__fastcall* free_prototype)(void*);

void readU8(INT8* data, UINT32& offset, volatile UINT8& result) {
    result = data[offset];
    offset += 1;
}

void readU32(INT8* data, UINT32& offset, UINT32& result) {
    result = *(UINT32*)(data + offset);
    offset += 4;
}

void readU16(INT8* data, UINT32& offset, UINT16& result) {
    result = *(UINT16*)(data + offset);
    offset += 2;
}

void readU64(INT8* data, UINT32& offset, UINT64& result) {
    result = *(UINT64*)(data + offset);
    offset += 8;
}

void readStr(INT8* data, UINT32& offset, INT8* result) {
    int i = 0;
    while (data[offset] != 0) {
        result[i++] = data[offset++];
    }
    result[i] = 0;
    offset++;
}

void utf8_to_wchar_custom(const char* src, wchar_t* dest) {
    if (!src || !dest) return;

    while (*src) {
        *dest = (wchar_t)((unsigned char)*src);
        src++;
        dest++;
    }
    *dest = L'\0';
}

UINT64 GetModuleFromPEB(GeneralContext* ctxt, const wchar_t* dllName,bool log = 0) {

    PPEB peb = 0;
    peb = (PPEB)ctxt->getPeb();

    if (!peb) {
        MessageBoxA(0, "PEB is zero!", 0, 0);
        return 0;
    }

    if (!peb->Ldr) {
        MessageBoxA(0, "LDR is zero!", 0, 0);
        return 0;
    }

    PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;
    PLIST_ENTRY curr = head->Flink;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)curr;

        if (entry->BaseDllName.Buffer != 0 && entry->DllBase != 0) {

            if (wcsicmp((wchar_t*)entry->BaseDllName.Buffer, dllName) == 0) {
                return (UINT64)entry->DllBase;
            }
            else {
                if (log) {
                    LogMessage("", "%ls != %ls", dllName, (wchar_t*)entry->BaseDllName.Buffer);
                }
            }
        }
        curr = curr->Flink;
    }
    return 0;
}

UINT64 GetProcAddressFromPE(UINT64 moduleBase, const char* funcName) {
    if (!moduleBase) return 0;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return 0;

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(moduleBase + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return 0;

    IMAGE_DATA_DIRECTORY exportDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (exportDir.VirtualAddress == 0) return 0;

    PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)(moduleBase + exportDir.VirtualAddress);

    DWORD* names = (DWORD*)(moduleBase + exports->AddressOfNames);
    DWORD* functions = (DWORD*)(moduleBase + exports->AddressOfFunctions);
    WORD* ordinals = (WORD*)(moduleBase + exports->AddressOfNameOrdinals);

    for (int i = 0; i < exports->NumberOfNames; i++) {
        char* currentFuncName = (char*)(moduleBase + names[i]);

        if (strcmp(currentFuncName, funcName) == 0) {
            WORD ordinal = ordinals[i];
            DWORD funcRva = functions[ordinal];

            if (funcRva >= exportDir.VirtualAddress && funcRva < (exportDir.VirtualAddress + exportDir.Size)) {
                char* forwardedStr = (char*)(moduleBase + funcRva);

                char* dot = forwardedStr;
                while (*(dot++) != '.');
                dot--;

                if (!dot) return 0;

                char targetDllName[256];
                size_t targetDllNameLen = dot - forwardedStr;

                memcpy(targetDllName, forwardedStr, targetDllNameLen);
                strcpy_s(targetDllName + targetDllNameLen, sizeof(targetDllName) - targetDllNameLen, ".dll");

                const char* targetFunc = dot + 1;

                wchar_t wDllName[256];

                utf8_to_wchar_custom(targetDllName, wDllName);

                UINT64 targetModule = GetModuleFromPEB(ctxt, wDllName);
                if(!targetModule)LoadLibraryA(targetDllName);

                return GetProcAddressFromPE((UINT64)targetModule, targetFunc);
            }

            return (UINT64)(moduleBase + funcRva);
        }
    }
    return 0;
}


UINT64 resolveFunc(GeneralContext* ctxt, const wchar_t* moduleName, const char* funcName,UINT16 ordVal = 0,bool ord = 0, bool log = 0)
{
    UINT64 targetModule = GetModuleFromPEB(ctxt, moduleName, log);
    UINT64 funcAddr = 0;

    if (!targetModule) {
        void* ad = LoadLibraryW(moduleName);

        if (!ad) {

            LogMessage("", "Cant load module: %ls",moduleName);
            
            return 0;
        }

        targetModule = (UINT64)ad;
    }

    if (!ord) {
        funcAddr = GetProcAddressFromPE(targetModule, funcName);

        if (!funcAddr)
        {
            funcAddr = (UINT64)GetProcAddress((HMODULE)targetModule, funcName);

        }

        if (!funcAddr) {
            
            LogMessage("", "cant resolve func: %s",funcName);
        }
    }
    else {

        if (!funcAddr) {
            HMODULE hRealModule = GetModuleHandleW(moduleName);
            if (hRealModule) {

                PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hRealModule;
                PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hRealModule + dosHeader->e_lfanew);
                DWORD exportRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                if (exportRVA) {
                    PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hRealModule + exportRVA);
                    DWORD* nameTable = (DWORD*)((BYTE*)hRealModule + exportDir->AddressOfNames);
                    WORD* ordinalTable = (WORD*)((BYTE*)hRealModule + exportDir->AddressOfNameOrdinals);

                    DWORD targetFuncIndex = ordVal - exportDir->Base;
                    const char* realFuncName = nullptr;

                    for (DWORD k = 0; k < exportDir->NumberOfNames; k++) {
                        if (ordinalTable[k] == targetFuncIndex) {
                            realFuncName = (const char*)((BYTE*)hRealModule + nameTable[k]);
                            break;
                        }
                    }

                    if (realFuncName) {
                        funcAddr = (UINT64)GetProcAddress(hRealModule, realFuncName);
                    }
                }
            }
        }

    }


    return funcAddr;
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:

        if (lpvReserved != nullptr)
        {
            break; // do not do cleanup if process termination scenario
        }

        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

void* resolveIat(DWORD id) {
    
    for (int i = 0;i < ctxt->dllsToImportCount;i++) {
        for (int j = 0;j < ctxt->dllsToImport[i].funcsToImportCount;j++) {
            if (id == ctxt->dllsToImport[i].funcs[j].id) {
                return (void*)ctxt->dllsToImport[i].funcs[j].addr;
            }
        }
    }

}

extern "C" void run_orig_code(void* a);

DWORD ConvertPeCharacteristicsToProtect(DWORD characteristics) {

    bool isExecute = (characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
    bool isRead = (characteristics & IMAGE_SCN_MEM_READ) != 0;
    bool isWrite = (characteristics & IMAGE_SCN_MEM_WRITE) != 0;

    if (isExecute && isRead && isWrite) {
        return PAGE_EXECUTE_READWRITE;
    }
    if (isExecute && isRead) {
        return PAGE_EXECUTE_READ;
    }
    if (isExecute) {
        return PAGE_EXECUTE;
    }
    if (isRead && isWrite) {
        return PAGE_READWRITE;
    }
    if (isRead) {
        return PAGE_READONLY;
    }

    return PAGE_NOACCESS;
}

bool compareKey(uint8_t* crypted, std::vector<uint8_t>& sig) {
    for (int i = 0;i < sig.size();i++) {
        if (crypted[i] != sig[i]) return 0;
        
    }
    return 1;
}

struct DataDirectoryInfo {
    IMAGE_DATA_DIRECTORY raw;
    UINT32 rva;
};

// runVM
extern "C" __declspec(dllexport) int runVM(INT8* inputParams) {
    uint32_t compSz = *reinterpret_cast<uint32_t*>(inputParams);
    uint32_t origSz = *reinterpret_cast<uint32_t*>(inputParams + 4);
    uint8_t* dataStart = reinterpret_cast<uint8_t*>(inputParams) + 8;

    std::vector<uint8_t> compressed_vec(dataStart, dataStart + compSz);
    std::vector<uint8_t> decompressed = decompress_data(compressed_vec, origSz);
    
    std::vector<uint8_t> magikSig = {
        0x0E, 0x0D, 0x11, 0x10, 0xDF, 0x0D, 0x04, 0xDF,
        0x0F, 0x1A, 0x1F, 0x14, 0x03, 0x12, 0x11, 0xDF,
        0x10, 0x0F, 0x11, 0x08, 0x17, 0x0D, 0x1F, 0x14,
        0xDF, 0x02, 0x0D, 0x11, 0xDF, 0xC0, 0xDF, 0x0A,
        0x18, 0xDF, 0x15, 0x0D, 0x11, 0xDF, 0x08, 0x17,
        0x0D, 0x1F, 0x1A, 0x0D, 0xDF, 0x02, 0x0D, 0x11,
        0xDF, 0x12, 0x11, 0xDF, 0x1D, 0x11, 0x0D, 0xDF,
        0x13, 0x11, 0x16, 0xDF, 0x1B, 0x0E, 0xC5, 0xDF,
        0x9A, 0x9C, 0x97, 0x90, 0xA0, 0xC8, 0xC8, 0xC8,
        0xC8, 0xC8, 0x13, 0x37, 0x42, 0x89, 0x54, 0x73
    };

    if (!compareKey(decompressed.data() + origSz - magikSig.size(), magikSig)) {
        LogMessage("", "Invalid sig!");
        return 1;
    }

    memcpy(inputParams, decompressed.data(), origSz);
    
    ctxt = new GeneralContext();

    ctxt->getPeb = (getPebFn)inputParams;
#if 1
    {
        // for the future: fill with zeros before leaving
        const wchar_t* user32_ws = L"user32.dll";
        ctxt->user32Base = GetModuleFromPEB(ctxt, user32_ws);
    }
    
    {
        const wchar_t* kernel_ws = L"kernel32.dll";
        ctxt->kernel32Base = GetModuleFromPEB(ctxt, kernel_ws);
    }

    {
        const char* loadLibA = "LoadLibraryA";
        loadLibrFn loadLib = (loadLibrFn)GetProcAddressFromPE(ctxt->kernel32Base, loadLibA);
        ctxt->LoadLib = loadLib;
    }

    {
        const char* freeLib_s = "FreeLibrary";
        freeLibrFn freeLib = (freeLibrFn)GetProcAddressFromPE(ctxt->kernel32Base, freeLib_s);
        ctxt->FreeLib = freeLib;
    }
#endif
    {
        ctxt->getCurrentProc = (GetCurrentProcessFn)GetProcAddressFromPE(ctxt->kernel32Base, "GetCurrentProcess");
    }

    // if not loaded (by default)
#if 0
    if (!ctxt.user32Base) {
        ctxt.user32WasLoaded = 0;
        
        const char* user32_s = "user32.dll";

        ctxt.user32Base = ctxt.LoadLib(user32_s);

        if (!ctxt.user32Base) {
            return 2; // cant load user32
        }
    }
    else {
        ctxt.user32WasLoaded = 1;
    }
#endif
    
    {
        //  NtFreeVirtualMemory
        ctxt->AllocateVirtualMemory =  (NtAllocateVirtualMemoryFn)resolveFunc(ctxt,L"ntdll.dll", "NtAllocateVirtualMemory"); // resolveFunc
        ctxt->FreeVirtualMemory = (NtFreeVirtualMemoryFn)resolveFunc(ctxt, L"ntdll.dll", "NtFreeVirtualMemory");
    }
    
    UINT32 offset = 10; // sizeof get peb code

    readU32(inputParams, offset, ctxt->sizeofImage);
    readU32(inputParams, offset, ctxt->entryPointRVA);

    readU64(inputParams, offset, ctxt->imageBase);
    
    readU8(inputParams, offset, ctxt->sectionCount);

    char* allocedMemRaw = (char*)VirtualAlloc((void*)0, ctxt->sizeofImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // ctxt->imageBase

    if (!allocedMemRaw) {
        DWORD errorCode = GetLastError();

        MessageBoxA(0,"VirtualAlloc not sus!", "VirtualAlloc Fail", MB_ICONERROR);
        return 1;
    }

#if 1

    std::vector<CustomSection> shortInfoSection;

    for (int i = 0; i < ctxt->sectionCount; i++) {

        CustomSection section;
        readU32(inputParams, offset, section.rawSize);
        readU32(inputParams, offset, section.virtualSize);
        readU32(inputParams, offset, section.virtAddr);

        memcpy(section.secName, inputParams + offset, 8);
        char safeSecName[9] = { 0 };
        memcpy(safeSecName, section.secName, 8);
        offset += 8;

        readU32(inputParams, offset, section.characteristics);

        memcpy(allocedMemRaw + section.virtAddr, inputParams + offset, section.rawSize);

        shortInfoSection.push_back(section);
        
        offset += section.rawSize;

    }

    UINT32 dllCount = 0;
    readU32(inputParams, offset, dllCount);

    for (int i = 0; i < (int)dllCount; i++) {
        char dllName[128] = { 0 };
        wchar_t w_dllName1[128] = { 0 };

        readStr(inputParams, offset, (INT8*)dllName);
        utf8_to_wchar_custom(dllName, w_dllName1);

        UINT32 fnCount = 0;
        readU32(inputParams, offset, fnCount);

        for (int j = 0; j < (int)fnCount; j++) {
            UINT32 originalRVA = 0;
            readU32(inputParams, offset, originalRVA);
            UINT8 isOrd = 0;
            readU8(inputParams, offset, isOrd);

            char* targetIATAddress = allocedMemRaw + originalRVA;

            uint64_t resolvedAddr = 0;
            if (isOrd) {
                uint16_t ordVal = 0;
                readU16(inputParams, offset, ordVal);

                UINT64 targMod = GetModuleFromPEB(ctxt, w_dllName1);
                if (!targMod) {
                    targMod = (UINT64)LoadLibraryW(w_dllName1);
                    if (!targMod) {
                        LogMessage("", "err: Cant load DLL for ordinal import\n");
                    }
                }

                if (targMod) {
                    resolvedAddr = (UINT64)GetProcAddress((HMODULE)targMod, (LPCSTR)(DWORD_PTR)ordVal);
                }
            }
            else {
                char funcName0[512] = { 0 };
                readStr(inputParams, offset, (INT8*)funcName0);

                resolvedAddr = resolveFunc(ctxt, w_dllName1, funcName0);
            }

            if (resolvedAddr) {
                *(uint64_t*)(targetIATAddress) = resolvedAddr;
            }
            else {
                std::cout << "[ERR] Failed to resolve function at RVA: 0x" << std::hex << originalRVA << "\n";
            }
            
            *(uint64_t*)(targetIATAddress) = resolvedAddr;
        }
    }

    UINT32 relocsCount = 0;

    DataDirectoryInfo dataDirs[16];

    for (int i = 0;i < 16;i++) {
        
        readU32(inputParams, offset, dataDirs[i].rva);
        readU32(inputParams, offset, (UINT32&)dataDirs[i].raw.Size);
        readU32(inputParams, offset, (UINT32&)dataDirs[i].raw.VirtualAddress);
        // va,size
        *(uint32_t*)(allocedMemRaw + dataDirs[i].rva) = dataDirs[i].raw.VirtualAddress;
        *(uint32_t*)(allocedMemRaw + dataDirs[i].rva + 4) = dataDirs[i].raw.Size;
    }

    readU32(inputParams, offset, relocsCount);

    UINT64 delta = ((UINT64)allocedMemRaw) - ctxt->imageBase;
    //LogMessage("tt", "fixing %d relocs", relocsCount);
    // fixing relocs
    for (int i = 0;i < relocsCount;i++) {
        UINT32 realocType = 0, realocVA = 0;
        readU32(inputParams, offset, realocType);
        readU32(inputParams, offset, realocVA);
        if (realocType == 0xA) { // DIR64
            *(UINT64*)(allocedMemRaw + realocVA) += delta;
        }
        else {
            // unsoported
            LogMessage(" ", "Unsuported reloc type (DIR64 only)!");
        }
    }

    for (int i = 0; i < ctxt->sectionCount; i++) {
        CustomSection* section = &shortInfoSection[i];

        DWORD protectFlags = ConvertPeCharacteristicsToProtect(section->characteristics);

        LPVOID sectionAddrInMem = (LPVOID)(allocedMemRaw + section->virtAddr);

        SIZE_T protectSize = section->virtualSize;

        DWORD oldProtect = 0;

        if (!VirtualProtect(sectionAddrInMem, protectSize, protectFlags, &oldProtect)) {
            LogMessage("Err", "VirtualProtect failed at %s. Err code: %lu",
                section->secName, GetLastError());
        }
    }
    
#endif
    
    run_orig_code(allocedMemRaw + ctxt->entryPointRVA);

     // free data

    for (int i = 0;i < ctxt->sectionCount;i++) {
        UINT64 result = ctxt->FreeVirtualMemory(ctxt->getCurrentProc(), // -1
            (UINT64)&ctxt->sections[i].data,
            (UINT64)&ctxt->sections[i].RegionSize,
            MEM_RELEASE);
        ctxt->sections;
        if (result != 0) // STATUS_SUCCESS
        {
            MessageBoxA(0, "cant free section!", 0, 0);
        }
    }

    delete[] ctxt->sections;
    delete[] ctxt->dllsToImport;
    delete ctxt;

    MessageBoxA(0, "sus exit!", 0, 0);

    ExitProcess(0);

    return 0;

}