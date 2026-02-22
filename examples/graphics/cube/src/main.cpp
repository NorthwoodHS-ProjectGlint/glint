
#include <glint/glint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern "C" void glattach(void* ctx) {

    glAttach(ctx);


}


extern "C" void app_setup() {



}

extern "C" int app_present() {

    glPresent();
    return glRunning();
}

extern "C" int app_cycle() {
    glEnable(GL_DEPTH_TEST);  

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glCameraSetPerspective(45.0f, 480.0f/272.0f, 0.1f, 100.0f);
    glCameraSetView(vec3(1,1,1), vec3(0,0,0), vec3(0,1,0));

    mat4 model = mat4::identity();
    model *= mat4::rotationY((float)glGetTime());
    model *= mat4::rotationX((float)glGetTime() * 0.5f);


    glCubeDraw(model, 0); // Using shader 0 for now, will need to set up a proper shader later

    if (hidIsButtonPressed(GLFW_KEY_ESCAPE)) {
        return 2; // Pause to home screen
    }
    
    /*
    if (game_over) {
        return 1; // Exit application
    }
    */
    

    return 0; // Continue running
}

extern "C" void app_shutdown() {

}