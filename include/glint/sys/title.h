#pragma once
#include <cstdint>
#include "exec.h"
#include <thread>

struct TitleInfo {
    char id[16];
    char name[32];
    char description[128];
    uint8_t icon_data[128*128];
    char tags[3][16];
};

typedef void (*FuncPtr)();

struct TitleThread {
    FuncPtr onReturn;
    FuncPtr onExit;

    bool isRunning = false;
    bool isPaused = false;
    int presentResult = (int)true;

    std::thread threadHandle;
};

TitleInfo titleLoadInfo(const char* path);

TitleThread* titleLaunch(Executable* exec);