#include "homescreen.h"
#include "glint/glint.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstring>

void HomeScreen::init() {
    // Set background color (dark navy, like a console UI)

    refreshTabs();

    size_t gamesLoaded = 0;
    const char** titles = fsListDirectory("titles/", &gamesLoaded);

    for (size_t i = 0; i < gamesLoaded; i++)
    {
        ioDebugPrint("Found title: %s\n", titles[i]);

        char full_path[256];
        strcpy(full_path, "titles/");
        strcat(full_path, titles[i]);

        TitleInfo info = titleLoadInfo(full_path);
        this->titles.push_back(info);
    }
    

}

void HomeScreen::onGameExit()
{
    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);

    isPauseMenuVisible = false;
    dbg_pageIndex = 0;
    dbg_tabIndex = 0;
    refreshTabs();

    ioDebugPrint("Game thread exited\n");
}

void HomeScreen::onGameReturn()
{

    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);

    // show pause menu
    isPauseMenuVisible = true;
    dbg_pageIndex = 2;
    dbg_tabIndex = 0;
    refreshTabs();

    ioDebugPrint("Game thread paused\n");
}


void HomeScreen::update() {
    // Game logic goes here later

    if (currentThread) {
        // if the game thread is running, we should probably pause the home screen and wait for the game to exit before allowing input again


        if (currentThread->isRunning && currentThread->isPaused && !isPauseMenuVisible) {

            onGameReturn();

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

    if (hidIsButtonPressed(GLFW_KEY_DOWN)) {
        dbg_tabIndex = (dbg_tabIndex + 1) % dbg_tabs.size();
    }

    if (hidIsButtonPressed(GLFW_KEY_UP)) {
        dbg_tabIndex = (dbg_tabIndex + dbg_tabs.size() - 1) % dbg_tabs.size();
    }

    if (hidIsButtonPressed(GLFW_KEY_ENTER)) {
        // TODO: handle tab selection

        if (dbg_pageIndex == 0 && dbg_tabIndex == 0) {
            dbg_pageIndex = 1; // launch game page
        } else if (dbg_pageIndex == 1 && dbg_tabIndex == 0) {
            dbg_pageIndex = 0; // back to home page
        } else if (dbg_pageIndex == 1 && dbg_tabIndex != 0) {
            // launch game

            int gameIndex = dbg_tabIndex - 1;

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

            }

        } else if (dbg_pageIndex == 2 && dbg_tabIndex == 0) {
            ioDebugPrint("Returning to game thread\n");
            glfwMakeContextCurrent(nullptr);
            currentThread->isPaused = false;

        } else if (dbg_pageIndex == 2 && dbg_tabIndex == 1) {
            // exit game
            ioDebugPrint("Exiting game thread\n");
            currentThread->isRunning = false;
            dbg_pageIndex = 0; // back to home
            currentThread = nullptr;
        }

        refreshTabs();
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
        glClearColor(0.15f, 0.05f, 0.15f, 1.0f);
        // Drawing code goes here

        for (size_t i = 0; i < dbg_tabs.size(); ++i)
        {
            renderTab(dbg_tabs[i], i);
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

void HomeScreen::renderTab(const char *text, int index)
{

    unsigned long color = 0xFFFFFFFF; // White text
    unsigned long bg = (index == dbg_tabIndex) ? 0xFF0000FF : 0xAAAAAAFF;


    glDebugText(color, bg, text);
}

void HomeScreen::refreshTabs()
{
    dbg_tabIndex = 0;

    switch (dbg_pageIndex) {
        case 0: // home
            dbg_tabs = {
                "Launch",
            };
            break;
        case 1: // launch game
        {
            dbg_tabs = {
                "Back\n",
            };

            int gamesLoaded = this->titles.size();

            if (gamesLoaded == 0) {
                dbg_tabs[0] = "Back\n(No games found)";
            }

            for (int i = 0; i < gamesLoaded; i++)
            {
                dbg_tabs.push_back(this->titles[i].name);
            }

            break;
        }
        case 2: // pause menu
        {
            dbg_tabs = {
                "Return to Game\n",
                "Exit Game\n"
            };
            break;
        }
        default:
            dbg_tabs = {};
    }
}

