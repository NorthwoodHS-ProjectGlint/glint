#include "glint/glint.h"

#include <cstring>
#include <algorithm>
#include <cstdint>
#include <sys/mman.h>
#include <dlfcn.h>
#include <cstdio>
#include <unistd.h>
#include "exec_internal.h"



Executable execLoad(const char *path)
{
    Executable exec;
    exec.executable = nullptr;
    exec.resource = nullptr;

    executable_file exec_file = loadExeFile(path);

    ioDebugPrint("Executable name: %s\nExecutable description: %s\n", exec_file.title_info.name, exec_file.title_info.description);

    exec.executable = exec_file.executable;
    exec.resource = exec_file.resource;

    exec.executable_size = exec_file.header.executable_size;
    exec.resource_size = exec_file.header.resource_size;

    return exec;
}

void *execExtract(const Executable* exec)
{
    if (!exec || !exec->executable) {
        ioDebugPrint("Invalid executable\n");
        return nullptr;
    }

    // Create an in-memory file
    int fd = memfd_create("game", MFD_CLOEXEC);

    // Write your .so data into it
    write(fd, exec->executable, exec->executable_size);

    // dlopen it via the /proc/self/fd trick
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);

    void* handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    return handle;
}

void execCallGlAttach(void *handle, void* ctx)
{
    auto func = (void(*)(void*))dlsym(handle, "glattach");
    if (!func) {
        ioDebugPrint("Failed to find function: glattach\n");
        return;
    }
    func(ctx);
}

void execCallHandle(void *handle, const char *func_name)
{
    auto func = (void(*)())dlsym(handle, func_name);
    if (!func) {
        ioDebugPrint("Failed to find function: %s\n", func_name);
        return;
    }
    func();
}

int execCallHandleWithResult(void *handle, const char *func_name)
{
    auto func = (int(*)())dlsym(handle, func_name);
    if (!func) {
        ioDebugPrint("Failed to find function: %s\n", func_name);
        return -1;
    }
    return func();
}