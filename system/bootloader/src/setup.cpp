#include "setup.h"
#include "glint/gl/gl.h"
#include <unistd.h>

void sysOsInit()
{
    // setup system directories
    {
        fsCreateDirectory("sys/");

        fsCreateDirectory("sys/apps/");
        fsCreateDirectory("sys/settings/");
        
        fsCreateDirectory("titles/");
    }


}



void sysHomeScreenBoot()
{
    ioDebugPrint("Booting home screen\n");

    // setup homescreen
    if (!fsFileExists("sys/apps/000400020000000.glt")) {

        // system is not setup, derive the system files from an image
        ioDebugPrint("Home screen executable not found, setting up system files\n");

        return; // skip setup for now, will implement later

    }

    Executable home_screen_exec = execLoad("sys/apps/000400020000000.glt");
    if (!home_screen_exec.executable) {
        ioDebugPrint("Failed to load home screen executable\n");
        return;
    }

    ioDebugPrint("Home screen executable loaded, size: %u bytes\n", home_screen_exec.executable_size);

    void* handle = execExtract(&home_screen_exec);
    if (!handle) {
        ioDebugPrint("Failed to extract home screen executable\n");
        return;
    }

    execMountResource(&home_screen_exec, "S:/");

    void* window = glSetup();

    hidInit();

    execCallGlAttach(handle, window);

    execCallHandle(handle, "app_setup");

    while (execCallHandleWithResult(handle, "app_present") == (int)true) {
        execCallHandle(handle, "app_cycle");

    }

    execCallHandle(handle, "app_shutdown");
    
}

