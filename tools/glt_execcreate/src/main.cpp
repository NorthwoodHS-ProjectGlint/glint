#include <iostream>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>

#include <cjson/cJSON.h>
#include <vector>

#include "includes.h"

const void *read_file(const char *path, size_t *out_size)
{
    size_t size = std::filesystem::file_size(path);
    char *buffer = new char[size];
    std::ifstream file(path, std::ios::binary);
    file.read(buffer, size);
    
    if (out_size) *out_size = size;
    return buffer;
}



resource_pack_file create_resource_pack(const char* resource_directory, size_t* out_size)
{
    // get list of files in resource directory
    std::vector<std::filesystem::path> resource_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(resource_directory))
    {
        if (entry.is_regular_file()) {
            resource_files.push_back(entry.path());
        }
    }

    resource_pack_file res_pack;
    std::memcpy(res_pack.header.magic, "GLTR", 4);
    res_pack.header.resource_count = resource_files.size();
    res_pack.entries = new resource_entry[res_pack.header.resource_count];

    size_t total_size = sizeof(res_pack.header) + (res_pack.header.resource_count * sizeof(resource_entry));

    for (size_t i = 0; i < resource_files.size(); i++) {
        const auto& path = resource_files[i];
        std::cout << "Found resource file: " << path << "\n";

        resource_entry entry;
        entry.path_length = std::filesystem::relative(path, resource_directory).string().length();
        entry.path = new char[entry.path_length];
        std::memcpy(entry.path, std::filesystem::relative(path, resource_directory).string().c_str(), entry.path_length);
        entry.data_size = std::filesystem::file_size(path);
        entry.data = read_file(path.string().c_str(), nullptr);
        res_pack.entries[i] = entry;

        total_size += entry.path_length + entry.data_size;
    }

    if (out_size) *out_size = total_size;
    return res_pack;
}

int main(int argc, char const *argv[])
{

    // exec_create: create a new executable file from a given binary and resource directory
    // usage: exec_create <input_binary> <title_config>

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_binary> <title_config>\n";
        return -1;
    }

    //printf("Arguments:\nInput binary: %s\nTitle config: %s\nOutput executable: %s\n", argv[1], argv[2], argv[3]);

    // read exec file
    executable_file exec_file;
    std::memcpy(exec_file.header.magic, "GLTE", 4);
    exec_file.header.executable_size = std::filesystem::file_size(argv[1]);
    
    const void* input_binary = read_file(argv[1], nullptr);
    if (!input_binary) {
        std::cerr << "Failed to read input binary: " << argv[1] << "\n";
        return -1;
    }

    //printf("Loaded input binary: %s (size: %u bytes)\n", argv[1], exec_file.header.executable_size);

    // read title config
    void* resource_data = nullptr;
    bool copy_to_directory = false;
    const char* directory_to_copy = "";

    cJSON *root = cJSON_Parse((const char*)read_file(argv[2], nullptr));
    

    // title config
    cJSON *title = cJSON_GetObjectItem(root, "title");
    {
        const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(title,"id"));
        const char* name = cJSON_GetStringValue(cJSON_GetObjectItem(title,"name"));
        const char* description = cJSON_GetStringValue(cJSON_GetObjectItem(title,"description"));
        const char* icon_path = cJSON_GetStringValue(cJSON_GetObjectItem(title,"icon"));

        cJSON* tag_array = cJSON_GetObjectItem(title, "tags");
        int tag_array_size = cJSON_GetArraySize(tag_array);
        const char** tags = new const char*[tag_array_size];

        for (int i = 0; i < tag_array_size; i++)
        {
            tags[i] = cJSON_GetStringValue(cJSON_GetArrayItem(tag_array,i));
        }

        std::string resource_path = cJSON_GetStringValue(cJSON_GetObjectItem(title, "resources"));
        
        resource_path = (std::filesystem::current_path() / resource_path).string();

        // setup title config
        std::strcpy(exec_file.title_info.id, id);
        std::strcpy(exec_file.title_info.name, name);
        std::strcpy(exec_file.title_info.description, description);
        
        for (int i = 0; i < tag_array_size; i++)
        {
            std::strncpy(exec_file.title_info.tags[i], tags[i], sizeof(exec_file.title_info.tags[i]) - 1);
            exec_file.title_info.tags[i][sizeof(exec_file.title_info.tags[i]) - 1] = '\0';
        }

        for (int i = tag_array_size; i < 3; i++)
        {
            exec_file.title_info.tags[i][0] = '\0';
        }

        //printf("Title name: %s\nTitle description: %s\n", exec_file.title_info.name, exec_file.title_info.description);

        // load resources
        size_t res_size = 0;
        resource_pack_file rpack = create_resource_pack(resource_path.c_str(), &res_size);

        exec_file.header.resource_size = res_size;

        resource_data = new char[res_size];
        std::memcpy(resource_data, &rpack, res_size);
        
        //printf("Created resource pack from directory: %s (size: %u bytes)\n", resource_path.c_str(), res_size);
        
    }


    // debug config
    cJSON *debug = cJSON_GetObjectItem(root, "dbg");
    if (debug) {
        copy_to_directory = cJSON_IsTrue(cJSON_GetObjectItem(debug,"direct_copy"));
        directory_to_copy = cJSON_GetStringValue(cJSON_GetObjectItem(debug,"copy_directory"));
    }

    // Write the executable file

    std::string output_path = (std::filesystem::current_path() / (std::string(exec_file.title_info.id) + ".glt")).string();

    std::ofstream output_file(output_path, std::ios::binary);
    output_file.write((const char*)&exec_file.header, sizeof(executable_header));
    output_file.write((const char*)&exec_file.title_info, sizeof(executable_title_info));
    output_file.write((const char*)input_binary, exec_file.header.executable_size);
    output_file.write((const char*)resource_data, exec_file.header.resource_size);
    output_file.close();

    std::cout << "Executable file created successfully: " << output_path << "\n";

    // copy to directory
    if (copy_to_directory && std::filesystem::exists(directory_to_copy)) {
        std::filesystem::copy_file(output_path, std::filesystem::path(directory_to_copy) / std::filesystem::path(output_path).filename(), std::filesystem::copy_options::overwrite_existing);
        std::cout << "Executable copied to: " << directory_to_copy << "\n";
    }

    return 0;
}
