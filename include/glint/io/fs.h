#pragma once
#include <cstddef>

void fsCreateDirectory(const char* path);
bool fsDirectoryExists(const char* path);

void fsCreateFile(const char* path);
void fsWriteFile(const char* path, const void* data, size_t size);
const void* fsReadFile(const char* path, size_t* out_size);
bool fsFileExists(const char* path);

const char** fsListDirectory(const char* path, size_t* out_count);