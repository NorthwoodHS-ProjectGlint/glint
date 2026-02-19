#include "homescreen.h"
#include "glint/glint.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstring>

void HomeScreen::init() {
    // Set background color (dark navy, like a console UI)

    size_t gamesLoaded = 0;
    const char** titles = fsListDirectory("titles/", &gamesLoaded);

    for (size_t i = 0; i < gamesLoaded; i++)
    {
        ioDebugPrint("Found title: %s\n", titles[i]);

        char full_path[256];
        strcpy(full_path, "titles/");
        strcat(full_path, titles[i]);

        TitleInfo info = titleLoadInfo(full_path);

        ioDebugPrint("Title icon index: %d\n", info.icon_texture);

        this->titles.push_back(info);
    }

    ioDebugPrint("Total titles loaded: %zu\n", this->titles.size());
    
    appIcon_empty = glGenerateTexture("S:/AppEmpty.png", 4);
    appIcon_filled = glGenerateTexture("S:/AppFilled.png", 4);
    sidebar = glGenerateTexture("S:/HS_Sidebar.png", 4);
    sidebar_button = glGenerateTexture("S:/HS_SidebarButton.png", 4);
    appIcon_select = glGenerateTexture("S:/AppSelectOverlay.png", 4);



    ioDebugPrint("Assets loaded\n");

    uiShader = glGenerateShader(
        // Vertex shader
        R"(
        #version 300 es
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;

        uniform vec2 position;
        uniform vec2 scale;

        #define SCREEN_WIDTH 480.0
        #define SCREEN_HEIGHT 272.0
        
        void main() {

            // generate screen-space position
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / vec2(SCREEN_WIDTH, SCREEN_HEIGHT)) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y; // Flip Y for OpenGL coordinate system

            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
        // Fragment shader
        R"(
        #version 300 es
        precision mediump float;
        
        in vec2 TexCoord;
        uniform sampler2D tex;
        uniform sampler2D overlay;
        uniform bool hasOverlay;
        
        out vec4 FragColor;
        
        void main() {
            vec4 baseColor = texture(tex, TexCoord);

            vec2 adjustedTexCoord = TexCoord; // Adjust as needed for overlay texture
            adjustedTexCoord *= vec2(95,95);
            adjustedTexCoord -= vec2(10,6);
            adjustedTexCoord /= vec2(75,75);

            vec4 overlayColor = texture(overlay, adjustedTexCoord);
            if (hasOverlay) {
                FragColor = overlayColor * baseColor;
                FragColor.a = baseColor.a;
            } else {
                FragColor = baseColor;
            }
        }

        )"
    );

    ioDebugPrint("Home screen initialized\n");

}

void HomeScreen::onGameExit()
{
    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);

    isPauseMenuVisible = false;

    ioDebugPrint("Game thread exited\n");
}

void HomeScreen::onGameReturn()
{

    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);

    // show pause menu
    isPauseMenuVisible = true;

    ioDebugPrint("Game thread paused\n");
}


void HomeScreen::update() {
    // Game logic goes here later

    if (currentThread) {
        // if the game thread is running, we should probably pause the home screen and wait for the game to exit before allowing input again


        if (currentThread->isRunning && currentThread->isPaused && !isPauseMenuVisible) {

            onGameReturn();
            
            ioDebugPrint("Exiting game thread\n");
            currentThread->isRunning = false;
            currentThread = nullptr;

        } else if (!currentThread->isRunning && currentThread->isPaused) {

            onGameExit();

            currentThread = nullptr;
        } else if (currentThread->isRunning && !currentThread->isPaused) {
            if (glfwGetCurrentContext() != nullptr) {
                glfwMakeContextCurrent(nullptr);
            }
        }

        if (!isPauseMenuVisible)
            return;
    }

    if (hidIsButtonPressed(GLFW_KEY_LEFT)) {
        if (dbg_tabIndex > 0) dbg_tabIndex--;
    } else if (hidIsButtonPressed(GLFW_KEY_RIGHT)) {
        if (dbg_tabIndex < 11) dbg_tabIndex++;
    }




    if (hidIsButtonPressed(GLFW_KEY_ENTER)) {
        // TODO: handle tab selection

        if (!currentThread) {
            int gameIndex = dbg_tabIndex;

            if (gameIndex >= 0 && gameIndex < this->titles.size()) {
                // path is at titles/<title_id>.glt
                const char* gamePath = nullptr;

                {
                    char full_path[256];
                    strcpy(full_path, "titles/");
                    strcat(full_path, this->titles[gameIndex].id);
                    strcat(full_path, ".glt");
                    gamePath = full_path;
                }

                ioDebugPrint("Launching game: %s\n", gamePath);

                // this would pause the home screen frame and load the game within the same thread
                // for a real implementation, we would probably want to launch the game in a separate thread and make the homescreen return to the background while the game is running

                Executable exec = execLoad(gamePath);

                currentThread = titleLaunch(&exec);

                ioDebugPrint("Game thread launched\n");

            } else {
                ioDebugPrint("No game assigned to this tab\n");
            }
        } else {

        }

    }

}




void HomeScreen::render() {

    if (currentThread && currentThread->isRunning && !currentThread->isPaused) {
        // if the game thread is running, we should probably pause the home screen and wait for the game to exit before allowing input again
        isPauseMenuVisible = false;
        return;
    }



    if (!currentThread || (currentThread->isPaused)) {

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 1, 1, 1.0f);


        if (!currentThread) {
            for (int x = 0; x < 4; x++)
            {
                for (int y = 0; y < 3; y++)
                {

                    int index = x + (y * 4);

                    bool hasTitle = index < titles.size();

                    glUseProgram(uiShader);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, hasTitle ? appIcon_filled : appIcon_empty);
                    glUniform1i(glGetUniformLocation(uiShader, "tex"),0);


                    if (hasTitle) {
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, titles[index].icon_texture);
                    } else {
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, appIcon_empty);
                    }
                    glUniform1i(glGetUniformLocation(uiShader, "overlay"), 1);
                    glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), hasTitle ? 1 : 0);


                    glQuadDraw(5 + (x * 85), 5 + (y * 85), 95, 95, uiShader);

                    if (index == dbg_tabIndex) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, appIcon_select);
                        glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);
                        glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);
                        glQuadDraw(15 + (x * 85), 11 + (y * 85), 75, 75, uiShader);
                    }
                }
                
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sidebar);
            glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);
            glQuadDraw(355,0,125,272, uiShader);
        } else {

        }

    } else if (currentThread) {

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.00f, 0.00f, 0.00f, 1.0f);

        glDebugText("Loading game..");
    }


}

int HomeScreen::present()
{
    if (currentThread && currentThread->isRunning && !currentThread->isPaused) {
        // if the game thread is running, we should probably pause the home screen and wait for the game to exit before allowing input again
        return currentThread->presentResult;
    }
    glPresent();
    return glRunning();
}

