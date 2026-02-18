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
    int dbg_pageIndex = 0;

    std::vector<const char*> dbg_tabs = {};
    std::vector<TitleInfo> titles = {};

    TitleThread* currentThread = nullptr;
    bool isPauseMenuVisible = false;

    void renderTab(const char* text, int index);
    void refreshTabs();

    void onGameExit();
    void onGameReturn();
};