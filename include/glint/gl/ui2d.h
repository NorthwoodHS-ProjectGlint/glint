#pragma once
#include <functional>
#include "glint/types/graphics.h"

struct UiFrame;

struct UiLayoutSettings {
    enum Type {
        None,
        Vertical,
        Horizontal,
        Grid
    } type;

    enum Alignment {
        Start,
        Center,
        End
    } alignmentX, alignmentY;

    // for grid layout
    int columns;
    int rows;

    // spacing between elements
    float spacingX;
    float spacingY;  
    
    int paddingLeft, paddingRight, paddingTop, paddingBottom;

    bool autoSizeWidth;
    bool autoSizeHeight;

    bool relativeChildren;
};

struct UiEffectSettings {

    // blur
    bool blurEnabled = false;
    float blurRadius = 0.0f;
    bool blurPassesSetup = false;
    int blurFramebuffers[2];
    int blurTextures[2];
    int blurTextureWidth, blurTextureHeight;


    // drop shadow
    bool dropShadowEnabled = false;
    float dropShadowOffsetX = 0.0f;
    float dropShadowOffsetY = 0.0f;
    float dropShadowBlur = 0.0f;
    ColorRGBA dropShadowColor = {0.0f, 0.0f, 0.0f, 0.4f};

    // inner shadow
    bool innerShadowEnabled = false;
    float innerShadowOffsetX = 0.0f;
    float innerShadowOffsetY = 0.0f;
    float innerShadowBlur = 0.0f;
    ColorRGBA innerShadowColor = {0.0f, 0.0f, 0.0f, 0.35f};

private:
    friend UiFrame;


};

struct UiFrame {
    float x, y, width, height;
    float alpha = 1.0f;
    ColorRGB color = {1,1,1};
    int texture = -1;

    bool forceAlpha = false;
    bool visible = true;

    int shader = -1;
    bool canSelect = true;

    std::function<void(UiFrame&)> onRender;
    std::function<void(UiFrame&)> onClick;

    std::vector<UiFrame*> children;
    UiFrame* parent = nullptr;
    void setParent(UiFrame& parent);

    void setAsMainFrame();
    bool isSelected();
    bool isSelectable();
    void select();

    void calculateAutoSize();


    void draw_internal();

    UiLayoutSettings& getLayoutSettings() { return layoutSettings; }
    UiEffectSettings& getEffectSettings() { return effectSettings; }

    void applyLayout();
    void revertLayout();

private:

    UiLayoutSettings layoutSettings;
    UiEffectSettings effectSettings;

    std::tuple<float,float> getLayoutPosition();

    std::tuple<float,float> preLayout;

    std::vector<UiFrame*> getAllFrames() {
        std::vector<UiFrame*> allFrames;
        allFrames.push_back(this);
        for (UiFrame* child : children) {
            auto childFrames = child->getAllFrames();
            allFrames.insert(allFrames.end(), childFrames.begin(), childFrames.end());
        }
        return allFrames;
    }
    
};

void ui2dInit();
void ui2dUpdate();
void ui2dDraw();

UiFrame& ui2dGetMainFrame();

UiFrame& ui2dAddFrame(float x, float y, float width, float height);