#include "glint/glint.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>

namespace fs = std::filesystem;

void fsCreateDirectory(const char *path)
{
    if (!fsDirectoryExists(path)) {
        ioDebugPrint("Creating directory: %s\n", path);
        fs::create_directories(path);
        fs::permissions(path, fs::perms::all);
    }
}

bool fsDirectoryExists(const char *path)
{
    return fs::exists(path) && fs::is_directory(path);
}

void fsCreateFile(const char *path)
{
    if (!fsFileExists(path)) {
        ioDebugPrint("Creating file: %s\n", path);
        std::ofstream file(path);
        fs::permissions(path, fs::perms::all);
    }
}

bool fsFileExists(const char *path)
{
    return fs::exists(path) && fs::is_regular_file(path);
}

const char **fsListDirectory(const char *path, size_t *out_count)
{
    if (!fsDirectoryExists(path)) {
        if (out_count) *out_count = 0;
        return nullptr;
    }

    std::vector<std::string> entries;
    for (const auto &entry : fs::directory_iterator(path)) {
        entries.push_back(entry.path().filename().string());
    }

    char **result = new char *[entries.size()];
    for (size_t i = 0; i < entries.size(); i++) {
        result[i] = new char[entries[i].size() + 1];
        std::strcpy(result[i], entries[i].c_str());
    }

    if (out_count) *out_count = entries.size();
    return const_cast<const char **>(result);
}

const void *fsReadFile(const char *path, size_t *out_size)
{
    if (!fsFileExists(path)) return nullptr;
    
    size_t size = fs::file_size(path);
    char *buffer = new char[size];
    std::ifstream file(path, std::ios::binary);
    file.read(buffer, size);
    
    if (out_size) *out_size = size;
    return buffer;
}