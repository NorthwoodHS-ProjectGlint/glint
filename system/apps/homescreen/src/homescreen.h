#pragma once

#include <vector>
#include <glint/sys/title.h>

class HomeScreen {
public:
    void init();
    void update();
    void render();
    int present();

private:
    int dbg_tabIndex = 0;

    std::vector<TitleInfo> titles = {};

    TitleThread* currentThread = nullptr;
    bool isPauseMenuVisible = false;

    void onGameExit();
    void onGameReturn();


    // assets
    int appIcon_empty;
    int appIcon_filled;
    int sidebar;
    int sidebar_button;

    int appIcon_select;
    
    int uiShader;
};