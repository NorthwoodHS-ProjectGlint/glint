#include "setup.h"

int main(int argc, char const *argv[])
{

    ioDebugPrint("Loading glint handheld\n");

    sysOsInit();
    sysHomeScreenBoot();

    return 0;
}
