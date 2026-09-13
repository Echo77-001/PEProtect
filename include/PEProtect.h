#ifndef __PEPROTECT_H__
#define __PEPROTECT_H__

#include <intrin.h>

extern "C" {
    void __peprotect_start();
    void __peprotect_end();

    __declspec(dllimport) void vmEntryStub();
}

#define PEPROTECT_START() \
    _ReadWriteBarrier(); \
    __peprotect_start(); \
    __nop(); __nop(); __nop(); __nop(); \
    __nop(); __nop(); __nop(); __nop(); \
    _ReadWriteBarrier(); \
    vmEntryStub();

#define PEPROTECT_END() \
    _ReadWriteBarrier(); \
    __peprotect_end(); \
    __nop(); __nop(); __nop(); __nop(); \
    __nop(); __nop(); __nop(); __nop(); \
    _ReadWriteBarrier();

#endif // __PEPROTECT_H__
