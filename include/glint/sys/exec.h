#pragma once
#include <cstdint>
#include <cstddef>

struct Executable {

    const void* executable;
    const void* resource;

    uint32_t executable_size;
    uint32_t resource_size;

};

Executable execLoad(const char* path);

void* execExtract(const Executable* exec);
void execMountResource(const Executable* exec, char mountPoint[3]="H:/");
const void* execGetResource(const char* path, size_t* out_size=nullptr);

void execCallHandle(void* handle, const char* func_name);
int execCallHandleWithResult(void* handle, const char* func_name);
void execCallGlAttach(void* handle, void* ctx);