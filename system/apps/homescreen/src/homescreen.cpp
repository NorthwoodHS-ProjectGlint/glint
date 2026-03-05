#include "homescreen.h"
#include "glint/glint.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>

namespace {
inline void hsProbe(const char* step)
{
    ioDebugPrint("[HS_PROBE] %s\n", step);
}
}


void HomeScreen::init() {
    hsProbe("init: begin");
    hsProbe("init: loadTitles begin");
    loadTitles();
    hsProbe("init: loadTitles end");

    hsProbe("init: loadTextures begin");
    loadTextures();
    hsProbe("init: loadTextures end");

    hsProbe("init: buildShaders begin");
    buildShaders();
    hsProbe("init: buildShaders end");

    hsProbe("init: buildUi begin");
    buildUi();
    hsProbe("init: buildUi end");
    hsProbe("init: done");
}

UiFrame& HomeScreen::addFrame(float x, float y, float width, float height, UiFrame* parent)
{
    UiFrame& frame = ui2dAddFrame(x, y, width, height);
    if (parent) {
        frame.setParent(*parent);
    }
    return frame;
}

void HomeScreen::setSimpleQuad(UiFrame& frame, int shader, int texture)
{
    frame.shader = shader;
    frame.texture = texture;
}

void HomeScreen::loadTitles()
{
    hsProbe("loadTitles: begin");
    ioDebugPrint("Loading titles from filesystem...\n");
    size_t gamesLoaded = 0;
    const char** title_paths = fsListDirectory("titles/", &gamesLoaded);

    for (size_t i = 0; i < gamesLoaded; i++) {
        hsProbe("loadTitles: iter begin");
        ioDebugPrint("Found title: %s\n", title_paths[i]);

        char full_path[256];
        int written = snprintf(full_path, sizeof(full_path), "titles/%s", title_paths[i]);
        if (written < 0 || written >= (int)sizeof(full_path)) {
            ioDebugPrint("Skipping title with too-long path: %s\n", title_paths[i]);
            continue;
        }

        hsProbe("loadTitles: titleLoadInfo begin");
        TitleInfo info = titleLoadInfo(full_path);
        hsProbe("loadTitles: titleLoadInfo end");

        ioDebugPrint("Title icon index: %d\n", info.icon_texture);

        titles.push_back(info);
        hsProbe("loadTitles: push_back end");
    }

    ioDebugPrint("Total titles loaded: %zu\n", titles.size());
    hsProbe("loadTitles: end");
}

void HomeScreen::loadTextures()
{
    hsProbe("loadTextures: begin");
    ioDebugPrint("Loading textures...\n");

    hsProbe("loadTextures: appIcon_empty");
    appIcon_empty = glGenerateTexture("S:/AppEmpty.png", 4);

    hsProbe("loadTextures: appIcon_filled");
    appIcon_filled = glGenerateTexture("S:/AppFilled.png", 4);

    hsProbe("loadTextures: appIcon_select");
    appIcon_select = glGenerateTexture("S:/AppSelectOverlay.png", 4);

    hsProbe("loadTextures: sidebar_frame");
    sidebar_frame = glGenerateTexture("S:/SidebarFrame.png", 4);

    hsProbe("loadTextures: sidebar_button");
    sidebar_button = glGenerateTexture("S:/SidebarButton.png", 4);

    hsProbe("loadTextures: sidebar_mii_icon");
    sidebar_mii_icon = glGenerateTexture("S:/UserIconButton.png", 4);

    hsProbe("loadTextures: achievement_frame");
    achievement_frame = glGenerateTexture("S:/AchievementFrame.png", 4);

    hsProbe("loadTextures: achievement_container");
    achievement_container = glGenerateTexture("S:/AchievementContainer.png", 4);

    hsProbe("loadTextures: background_gradient");
    background_gradient = glGenerateTexture("S:/BackgroundFade.png", 4);

    hsProbe("loadTextures: background_test_image");
    background_test_image = glGenerateTexture("S:/BackgroundTest.png", 4);

    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        background_screenshot = tex;


    }

    ioDebugPrint("Textures loaded: app icons, sidebar, achievements, background\n");
    hsProbe("loadTextures: end");

}

void HomeScreen::buildShaders()
{
    ioDebugPrint("Building shaders...\n");
    uiAppShader = glGenerateShader(
        // Vertex shader
        R"(
        #version 300 es
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;

        uniform vec2 position;
        uniform vec2 scale;

        #define SCREEN_WIDTH 800.0
        #define SCREEN_HEIGHT 480.0
        
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
        
        uniform float alphaValue;

        out vec4 FragColor;
        
        void main() {
            vec4 baseColor = texture(tex, TexCoord);

            vec4 overlayColor = texture(overlay, TexCoord);
            if (hasOverlay) {
                FragColor = overlayColor * baseColor.r;
                FragColor.a = baseColor.a;
            } else {
                FragColor = baseColor;
            }

            FragColor.a *= alphaValue;
        }

        )"
    );

    uiShader = glGenerateShader(
        // Vertex shader
        R"(
        #version 300 es
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;

        uniform vec2 position;
        uniform vec2 scale;

        #define SCREEN_WIDTH 800.0
        #define SCREEN_HEIGHT 480.0
        
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
        uniform float alphaValue;
        uniform vec3 color;
        
        out vec4 FragColor;
        
        void main() {
            FragColor = texture(tex, TexCoord);

            FragColor.rgb *= color;
            FragColor.a *= alphaValue;
        }

        )"
    );

    ioDebugPrint("Shaders built successfully\n");

}

void HomeScreen::buildUi()
{
    ioDebugPrint("Building UI...\n");

    ui2dInit();

    homeFrame = addFrame(0, 0, 800, 480);
    homeFrame.setAsMainFrame();

    buildBackground();
    buildSidebar();
    buildCarousel();
    buildGameInfo();

    ioDebugPrint("UI built successfully\n");
}

void HomeScreen::buildBackground()
{
    UiFrame& bgFrame = addFrame(0, 0, 800, 480);
    //bgFrame.color = {0.25f, 0.25f, 0.25f};
    bgFrame.shader = uiShader;
    bgFrame.texture = background_test_image;
    auto& bgEffects = bgFrame.getEffectSettings();
    bgEffects.blurEnabled = true;
    bgEffects.blurRadius = 3;
    backgroundFrame = &bgFrame;

    UiFrame& bgImageFrame = addFrame(0, 0, 800, 480, backgroundFrame);
    setSimpleQuad(bgImageFrame, uiShader, background_gradient);
}

void HomeScreen::buildSidebar()
{
    UiFrame& sidebarFrame = addFrame(0, 24, 68, 232);
    setSimpleQuad(sidebarFrame, uiShader, sidebar_frame);

    UiFrame& sidebar = addFrame(0, 27, 62, 224, &sidebarFrame);
    sidebar.visible = true;

    auto& layout = sidebar.getLayoutSettings();
    layout.type = UiLayoutSettings::Vertical;
    layout.alignmentX = UiLayoutSettings::Center;
    layout.alignmentY = UiLayoutSettings::End;
    layout.spacingY = 5;
    layout.paddingTop = 10;
    layout.paddingBottom = 10;

    for (int i = 0; i < 5; i++) {
        UiFrame& button = addFrame(0, 0, 35, 35, &sidebar);
        int texture = (i == 0) ? sidebar_mii_icon : sidebar_button;
        setSimpleQuad(button, uiShader, texture);
    }
}

void HomeScreen::buildCarousel()
{
    UiFrame& carousel = addFrame(300, 0, 125, 480);
    gameCarousel = &carousel;

    auto& layout = carousel.getLayoutSettings();
    layout.type = UiLayoutSettings::Vertical;
    layout.alignmentX = UiLayoutSettings::Center;
    layout.alignmentY = UiLayoutSettings::Start;
    layout.spacingY = 15;
    layout.autoSizeHeight = true;

    // exit/shutdown button
    {
        float size = 100;
        UiFrame& button = addFrame(0, 0, size, size, &carousel);
        button.shader = uiShader;
        button.getEffectSettings().dropShadowEnabled = true;
        button.getEffectSettings().dropShadowOffsetX = 5;
        button.getEffectSettings().dropShadowOffsetY = 5;
        button.getEffectSettings().dropShadowBlur = 5;
        int i = carousel.children.size() - 1;

        button.onRender = [this, i](UiFrame& frame) {
            if (frame.isSelected()) {
                gameCarouselIndex = i;
            }

            if (gameCarouselIndex == i) {
                frame.width = lerp(frame.width, 125, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 125, glGetDeltaTime() * 15);
            } else {
                frame.width = lerp(frame.width, 100, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 100, glGetDeltaTime() * 15);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, appIcon_empty);
            glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, appIcon_empty);
            glUniform1i(glGetUniformLocation(uiShader, "overlay"), 1);
            glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);

            glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiShader);

            if (frame.isSelected()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, appIcon_select);
                glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);
                glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);
                glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiShader);
            }
        };

        button.onClick = [this](UiFrame& frame) {
            if (currentThread) {
                onGameExit();

                currentThread = nullptr;
            } else {
                // exit
                exit(0);
            }
        };
    }

    for (int i = 0; i < titles.size(); i++) {
        float size = 100;
        UiFrame& button = addFrame(0, 0, size, size, &carousel);
        button.shader = uiAppShader;

        if (i == 0) {
            button.select();
        }

        button.onRender = [this, i](UiFrame& frame) {
            if (frame.isSelected()) {
                gameCarouselIndex = i + 1;
            }

            if (i + 1 == gameCarouselIndex) {
                frame.width = lerp(frame.width, 125, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 125, glGetDeltaTime() * 15);
            } else {
                frame.width = lerp(frame.width, 100, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 100, glGetDeltaTime() * 15);
            }

            bool hasTitle = i < titles.size();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, hasTitle ? appIcon_filled : appIcon_empty);
            glUniform1i(glGetUniformLocation(uiAppShader, "tex"), 0);

            if (hasTitle) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, titles[i].icon_texture);
            } else {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, appIcon_empty);
            }
            glUniform1i(glGetUniformLocation(uiAppShader, "overlay"), 1);
            glUniform1i(glGetUniformLocation(uiAppShader, "hasOverlay"), hasTitle ? 1 : 0);

            glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiAppShader);

            if (frame.isSelected()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, appIcon_select);
                glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);
                glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);
                glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiShader);
            }
        };

        button.onClick = [this, i](UiFrame& frame) {
            int gameIndex = i;
            if (gameIndex >= 0 && gameIndex < titles.size()) {
                if (currentThread) {
                    if (currentTitleIndex == gameIndex) {
                        // game is already running, return to it
                        isReturningToGame = true;
                        return;
                    }
                    // add a prompt to confirm exiting the current game
                    return;
                } else {
                    const char* gamePath = nullptr;

                    {
                        char full_path[256];
                        int written = snprintf(full_path, sizeof(full_path), "titles/%s.glt", titles[gameIndex].id);
                        if (written < 0 || written >= (int)sizeof(full_path)) {
                            ioDebugPrint("Game path too long for title id: %s\n", titles[gameIndex].id);
                            return;
                        }
                        gamePath = full_path;
                    }

                    ioDebugPrint("Launching game: %s\n", gamePath);

                    Executable exec = execLoad(gamePath);
                    currentThread = titleLaunch(&exec);
                    currentTitleIndex = gameIndex;

                    ioDebugPrint("Game thread launched\n");
                }
            } else {
                ioDebugPrint("No game assigned to this tab\n");
            }
        };
    }


    {
        float size = 100;
        UiFrame& button = addFrame(0, 0, size, size, &carousel);
        button.shader = uiShader;
        button.getEffectSettings().dropShadowEnabled = true;
        button.getEffectSettings().dropShadowOffsetX = 5;
        button.getEffectSettings().dropShadowOffsetY = 5;
        button.getEffectSettings().dropShadowBlur = 5;
        int i = carousel.children.size() - 1;

        button.onRender = [this, i](UiFrame& frame) {
            if (frame.isSelected()) {
                gameCarouselIndex = i;
            }

            if (gameCarouselIndex == i) {
                frame.width = lerp(frame.width, 125, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 125, glGetDeltaTime() * 15);
            } else {
                frame.width = lerp(frame.width, 100, glGetDeltaTime() * 15);
                frame.height = lerp(frame.height, 100, glGetDeltaTime() * 15);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, appIcon_empty);
            glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, appIcon_empty);
            glUniform1i(glGetUniformLocation(uiShader, "overlay"), 1);
            glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);

            glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiShader);

            if (frame.isSelected()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, appIcon_select);
                glUniform1i(glGetUniformLocation(uiShader, "tex"), 0);
                glUniform1i(glGetUniformLocation(uiShader, "hasOverlay"), 0);
                glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiShader);
            }
        };

        button.onClick = [](UiFrame& frame) {
        };
    }
}

void HomeScreen::buildGameInfo()
{
    UiFrame& infoFrame = addFrame(73, 42, 215, 230);
    infoFrame.visible = true;

    //UiFrame& titleText = addFrame(72, 42, 215, 27, &infoFrame);
    //setSimpleQuad(titleText, uiShader, 0);

    //UiFrame& timeText = addFrame(72, 73, 215, 14, &infoFrame);
    //setSimpleQuad(timeText, uiShader, 0);

    UiFrame& achievContainer = addFrame(80.07f, 400, 201.88f, 181, &infoFrame);
    achievContainer.getLayoutSettings().relativeChildren = true;
    setSimpleQuad(achievContainer, uiShader, achievement_container);
    gameAchievementPanel = &achievContainer;

    UiFrame& achievFrameContainer = addFrame(5, 5, 0, 0, &achievContainer);
    achievFrameContainer.visible = true;
    auto& layout = achievFrameContainer.getLayoutSettings();
    layout.type = UiLayoutSettings::Vertical;
    layout.paddingTop = 5;
    layout.paddingBottom = 5;
    layout.paddingLeft = 5;
    layout.paddingRight = 5;
    layout.spacingY = 5;
    layout.autoSizeHeight = true;
    layout.autoSizeWidth = true;

    for (int i = 0; i < 1; i++) {
        UiFrame& achievFrame = addFrame(0, 0, 180, 37.5f, &achievFrameContainer);
        achievFrame.getLayoutSettings().relativeChildren = true;
        achievFrame.visible = true;

        UiFrame& achievFrameBG = addFrame(0, 0, 180, 37.5f, &achievFrame);
        setSimpleQuad(achievFrameBG, uiShader, achievement_frame);

        //UiFrame& achievText = addFrame(8, 5, 104, 11, &achievFrame);
        //setSimpleQuad(achievText, uiShader, 0);

        //UiFrame& achievDesc = addFrame(8, 19, 104, 14, &achievFrame);
        //setSimpleQuad(achievDesc, uiShader, 0);

        //UiFrame& achievProg = addFrame(115, 5, 55, 28, &achievFrame);
        //setSimpleQuad(achievProg, uiShader, 0);
    }
}


void HomeScreen::onGameExit()
{
    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);

    glDisable(GL_DEPTH_TEST);

    isPauseMenuVisible = false;

    ioDebugPrint("Game thread exited\n");
}

void HomeScreen::onGameReturn()
{

    GLFWwindow* window = (GLFWwindow*)glGetContext();
    glfwMakeContextCurrent(window);


    glDisable(GL_DEPTH_TEST);

    int width = 800, height = 480, channels = 4;
    GLubyte* pixels = new GLubyte[channels * width * height];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Flip vertically
    int rowSize = width * channels;
    GLubyte* temp = new GLubyte[rowSize];
    for (int y = 0; y < height / 2; y++) {
        GLubyte* top    = pixels + y * rowSize;
        GLubyte* bottom = pixels + (height - 1 - y) * rowSize;
        memcpy(temp, top, rowSize);
        memcpy(top, bottom, rowSize);
        memcpy(bottom, temp, rowSize);
    }
    delete[] temp;

    glBindTexture(GL_TEXTURE_2D, background_screenshot);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete[] pixels;
    glBindTexture(GL_TEXTURE_2D, 0);

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


    // autoscroll game carousel to selected game

    float targetY = -1 * gameCarouselIndex * (100 + 15); // item size + spacing

    targetY += (480/2) - (125/2); // center the carousel items vertically

    gameCarousel->y = lerp(gameCarousel->y, targetY, glGetDeltaTime() * 15);

    backgroundFrame->forceAlpha = false;

    if (isReturningToGame) {
        backgroundFrame->forceAlpha = true;
        // if returning to game, fade out home screen
         ui2dGetMainFrame().alpha = lerp(ui2dGetMainFrame().alpha, 0, glGetDeltaTime() * 10);
         backgroundFrame->children[0]->alpha = lerp(backgroundFrame->children[0]->alpha, 0, glGetDeltaTime() * 10);
         if (ui2dGetMainFrame().alpha < 0.01f) {
                isReturningToGame = false;
                glfwMakeContextCurrent(nullptr);
                currentThread->isPaused = false;
                ioDebugPrint("Returning to game thread\n");
         }
    } else {
        ui2dGetMainFrame().alpha = lerp(ui2dGetMainFrame().alpha, 1, glGetDeltaTime() * 10);
         backgroundFrame->children[0]->alpha = lerp(backgroundFrame->children[0]->alpha, 1, glGetDeltaTime() * 10);

        if (isPauseMenuVisible) {
            backgroundFrame->forceAlpha = true;
        }
    }


    // show achievement panel
    if (gameCarouselIndex > 0 && gameCarouselIndex < titles.size()+1) {
        gameAchievementPanel->visible = true;
        backgroundFrame->texture = background_test_image;

        gameAchievementPanel->y = lerp(gameAchievementPanel->y, 91, glGetDeltaTime() * 10);
        gameAchievementPanel->alpha = lerp(gameAchievementPanel->alpha, 1, glGetDeltaTime() * 15);
        backgroundFrame->alpha = lerp(backgroundFrame->alpha, 1, glGetDeltaTime() * 10);

        if (isPauseMenuVisible) {
            
            if (gameCarouselIndex - 1 == currentTitleIndex) {
                gameAchievementPanel->visible = false;
                backgroundFrame->texture = background_screenshot;
            gameAchievementPanel->y = lerp(gameAchievementPanel->y, 200, glGetDeltaTime() * 10);
            gameAchievementPanel->alpha = lerp(gameAchievementPanel->alpha, 0, glGetDeltaTime() * 15);

            }
        }

    } else {
        gameAchievementPanel->y = lerp(gameAchievementPanel->y, 200, glGetDeltaTime() * 10);
        gameAchievementPanel->alpha = lerp(gameAchievementPanel->alpha, 0, glGetDeltaTime() * 15);
        backgroundFrame->alpha = lerp(backgroundFrame->alpha, 0, glGetDeltaTime() * 10);
    }


    ui2dUpdate();


}




void HomeScreen::render() {

    if (currentThread && currentThread->isRunning && !currentThread->isPaused) {
        // if the game thread is running, we should probably pause the home screen and wait for the game to exit before allowing input again
        isPauseMenuVisible = false;
        return;
    }




    if (!currentThread || (currentThread->isPaused)) {

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 0.98f, 0.97f, 1.0f);

    } else if (currentThread) {

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1.0f);

        // if bootign into game, fade 
        ui2dGetMainFrame().alpha = lerp(ui2dGetMainFrame().alpha, 0, glGetDeltaTime() * 10);

    }

    glDebugTextFmt("Hello, Glint! Time: %.2f", glGetTime());
    // delta time
    glDebugTextFmt("Delta Time: %.4f", glGetDeltaTime());


    ui2dDraw();
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

