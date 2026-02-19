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
    uint8_t icon_data[128*128*3];
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

struct resource_pack_header {
    char magic[4];
    uint32_t resource_count;
};

struct resource_entry {
    u_int16_t path_length;
    char* path;

    uint32_t data_size;
    const void* data;
};

struct resource_pack_file {
    resource_pack_header header;
    resource_entry* entries;
};

inline resource_pack_file read_resource_pack(const void* data, size_t data_size)
{
    resource_pack_file res_pack;
    const char* ptr = static_cast<const char*>(data);
    size_t offset = 0;

    // Read header
    std::memcpy(&res_pack.header, ptr + offset, sizeof(res_pack.header));
    offset += sizeof(res_pack.header);

    // Allocate entries array
    res_pack.entries = new resource_entry[res_pack.header.resource_count];

    // Read each entry
    for (size_t i = 0; i < res_pack.header.resource_count; i++) {
        resource_entry& entry = res_pack.entries[i];

        // Read path_length
        std::memcpy(&entry.path_length, ptr + offset, sizeof(entry.path_length));
        offset += sizeof(entry.path_length);

        // Read path
        entry.path = new char[entry.path_length];
        std::memcpy(entry.path, ptr + offset, entry.path_length);
        offset += entry.path_length;

        // Read data_size
        std::memcpy(&entry.data_size, ptr + offset, sizeof(entry.data_size));
        offset += sizeof(entry.data_size);

        // Read data
        entry.data = new char[entry.data_size];
        std::memcpy(const_cast<char*>(static_cast<const char*>(entry.data)), ptr + offset, entry.data_size);
        offset += entry.data_size;
    }

    return res_pack;
}