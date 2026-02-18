#pragma once
#include <cstdint>

struct Executable {

    const void* executable;
    const void* resource;

    uint32_t executable_size;
    uint32_t resource_size;

};

Executable execLoad(const char* path);

void* execExtract(const Executable* exec);
void execCallHandle(void* handle, const char* func_name);
int execCallHandleWithResult(void* handle, const char* func_name);
void execCallGlAttach(void* handle, void* ctx);