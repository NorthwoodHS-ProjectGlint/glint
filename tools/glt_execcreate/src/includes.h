#pragma once

// exec info
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

// resource info
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
