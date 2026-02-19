#include "glint/sys/title.h"
#include "exec_internal.h"
#include <glint/sys/exec.h>
#include <thread>
#include <GLFW/glfw3.h>
#include <glint/gl/gl.h>


TitleInfo titleLoadInfo(const char *path)
{
    executable_file exec_file = loadExeFile(path);

    TitleInfo info;
    std::memcpy(info.name, exec_file.title_info.name, sizeof(info.name));
    std::memcpy(info.id, exec_file.title_info.id, sizeof(info.id));
    std::memcpy(info.description, exec_file.title_info.description, sizeof(info.description));
    std::memcpy(info.icon_data, exec_file.title_info.icon_data, sizeof(info.icon_data));
    std::memcpy(info.tags, exec_file.title_info.tags, sizeof(info.tags));

    info.icon_texture = glGenerateTexture(128,128,info.icon_data, 3);

    return info;
}

// ran on separate thread when a title is launched, responsible for running the title's executable and handling its lifecycle
void p_exeTitle(void* handle, TitleThread* thread, void* glCtx) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    thread->isRunning = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    execCallGlAttach(handle, glCtx);

    execCallHandle(handle, "app_setup");

    ioDebugPrint("Title thread started\n");


    // app cycle
    while (thread->isRunning) {


        int result = execCallHandleWithResult(handle, "app_cycle");
        // 0 is continue
        // 1 is exit
        // 2 is pause

        if (result == 1) {
            ioDebugPrint("Title requested exit\n");
            thread->isRunning = false;
        } else if (result == 2)
        {
            ioDebugPrint("Title requested pause\n");
            glfwMakeContextCurrent(nullptr);
            thread->isPaused = true;
        }
        
        thread->presentResult = execCallHandleWithResult(handle, "app_present");

        // if pausing to home screen, return to the main thread
        if (thread->isPaused) {
            // wait until unpaused
            while (thread->isPaused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            execCallGlAttach(handle, glCtx);

            if (!thread->isRunning) {
                break;
            }
        }


    }


    execCallHandle(handle, "app_shutdown");
    glfwMakeContextCurrent(nullptr);
    thread->isPaused = true;
    

    ioDebugPrint("Title thread exiting\n");
}

TitleThread* titleLaunch(Executable *exec)
{
    TitleThread* thread = new TitleThread;
    void* ctx = glGetContext();


    // create separate thread for the title and run the executable within it
    thread->threadHandle = std::thread(p_exeTitle, execExtract(exec), thread, ctx);

    return thread;
}