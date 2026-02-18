
#include <glint/glint.h>
#include "homescreen.h"
#include <GLFW/glfw3.h>

HomeScreen* homescreen;

extern "C" void glattach(void* ctx) {

    glAttach(ctx);


}


extern "C" void app_setup() {
    homescreen = new HomeScreen();
    homescreen->init();



    ioDebugPrint("Home screen setup complete\n");
}

extern "C" int app_present() {
    return homescreen->present();
}

extern "C" void app_cycle() {
    homescreen->update();
    homescreen->render();
}

extern "C" void app_shutdown() {
    delete homescreen;
}