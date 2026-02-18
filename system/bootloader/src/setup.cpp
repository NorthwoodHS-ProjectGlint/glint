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
    if (!fsFileExists("sys/apps/home_screen.glt")) {

        // extract the default homescreen app from the binary and write it to the filesystem
        fsCreateFile("sys/apps/home_screen.glt");

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

    void* window = glSetup();

    hidInit();

    execCallGlAttach(handle, window);

    execCallHandle(handle, "app_setup");

    while (execCallHandleWithResult(handle, "app_present") == (int)true) {
        execCallHandle(handle, "app_cycle");

    }

    execCallHandle(handle, "app_shutdown");
    
}

