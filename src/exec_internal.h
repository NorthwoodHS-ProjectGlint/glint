#pragma once
#include <cstdint>
#include <glint/io/fs.h>
#include <glint/io/io.h>
#include <cstring>
#include <algorithm>

struct executable_header {
    char magic[4];
    uint32_t executable_size;
    uint32_t resource_size;
};

struct executable_title_info {
    char id[16];
    char name[32];
    char description[128];
    uint8_t icon_data[128*128];
    char tags[3][16];
};

struct executable_file {
    executable_header header;
    executable_title_info title_info;
    const void* executable;
    const void* resource;
};

inline executable_file loadExeFile(const char* path) {
    
    size_t size;
    const void* data = fsReadFile(path, &size);

    if (!data) {
        ioDebugPrint("Failed to read executable: %s\n", path);
        return executable_file();
    }

    if (size < sizeof(executable_header)) {
        ioDebugPrint("Executable file too small: %s\n", path);
        return executable_file();
    }

    if (std::memcmp(data, "GLTE", 4) != 0) {
        ioDebugPrint("Invalid executable format: %s\n", path);
        return executable_file();
    }

    executable_file exec_file;

    size_t headerSize = std::min(sizeof(executable_header), size);
    std::memcpy(&exec_file.header, data, headerSize);

    size_t titleSize = std::min(sizeof(executable_title_info), size);
    std::memcpy(&exec_file.title_info, data+headerSize, titleSize);

    exec_file.executable = (const char*)data + headerSize + titleSize;
    exec_file.resource = (const char*)data + headerSize + titleSize + exec_file.header.executable_size;


    return exec_file;
}