#include "glint/glint.h"

#include <map>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <sys/mman.h>
#include <dlfcn.h>
#include <cstdio>
#include <unistd.h>
#include <string>
#include "exec_internal.h"

typedef std::map<std::string, resource_entry> ResourceMap;

static std::map<char*, ResourceMap> g_mountedResources = {} ;

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



void execMountResource(const Executable *exec, char mountPoint[3])
{
    if (!exec || !exec->resource) {
        ioDebugPrint("Invalid executable or no resources to mount\n");
        return;
    }

    ResourceMap resourceMap;
    resource_pack_file pack = read_resource_pack(exec->resource, exec->resource_size);

    for (int i = 0; i < pack.header.resource_count; i++)
    {
        ioDebugPrint("Mounting resource: %s (size: %u bytes)\n", pack.entries[i].path, pack.entries[i].data_size);
        resourceMap[pack.entries[i].path] = pack.entries[i];
    }
    g_mountedResources[mountPoint] = resourceMap;
    

}

const void *execGetResource(const char *path, size_t *out_size)
{
    // get first 3 characters as mount point
    const char mountPoint[4] = {path[0], path[1], path[2], '\0'};
    const char* resourcePath = path + 3; // skip mount point



    for (const auto& mountPointEntry : g_mountedResources) {


        if (strcmp(mountPointEntry.first, mountPoint) == 0) {


            const auto& resourceMap = mountPointEntry.second;
            const auto& it = resourceMap.find(std::string(resourcePath));
            if (it != resourceMap.end()) {

                if (out_size) {
                    *out_size = it->second.data_size;
                }
                return it->second.data;
            } else {
                ioDebugPrint("Resource not found in mount point: %s\n", mountPointEntry.first);

            }
        }
    }
    return nullptr;
}

void execCallGlAttach(void *handle, void *ctx)
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