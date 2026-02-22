#pragma once

#include <vector>
#include <glint/glint.h>

class HomeScreen {
public:
    void init();
    void update();
    void render();
    int present();

private:
    int gameCarouselIndex = 0;

    std::vector<TitleInfo> titles = {};

    int currentTitleIndex = -1;
    TitleThread* currentThread = nullptr;
    bool isPauseMenuVisible = false;
    bool isReturningToGame = false;

    void onGameExit();
    void onGameReturn();

    void loadTitles();
    void loadTextures();
    void buildShaders();
    void buildUi();
    void buildBackground();
    void buildSidebar();
    void buildCarousel();
    void buildGameInfo();

    UiFrame& addFrame(float x, float y, float width, float height, UiFrame* parent = nullptr);
    void setSimpleQuad(UiFrame& frame, int shader, int texture);


    // assets
    int appIcon_empty;
    int appIcon_filled;
    int appIcon_select;
    
    int sidebar_frame;
    int sidebar_button;
    int sidebar_mii_icon;

    int achievement_frame;
    int achievement_container;

    int background_gradient;
    int background_test_image;
    int background_screenshot;
    
    int uiAppShader;
    int uiShader;

    // ui
    UiFrame homeFrame;
    UiFrame* gameCarousel;
    UiFrame* gameAchievementPanel;
    UiFrame* backgroundFrame;

    UiFrame pauseFrame;
};