#include "glint/glint.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


static UiFrame& mainFrame = *new UiFrame();
static std::vector<UiFrame*> allFrames;
static UiFrame* selectedFrame = nullptr;

static int uiDefaultShader = 0;
static int uiBlurShader = 0;
static int uiShadowShader = 0;
static int uiInnerShadowShader = 0;

static void ui2dEnsureBlurTargets(UiEffectSettings& effects, int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    if (effects.blurPassesSetup && effects.blurTextureWidth == width && effects.blurTextureHeight == height) {
        return;
    }

    if (!effects.blurPassesSetup) {
        glGenFramebuffers(2, reinterpret_cast<GLuint*>(effects.blurFramebuffers));
        glGenTextures(2, reinterpret_cast<GLuint*>(effects.blurTextures));
        effects.blurPassesSetup = true;
    }

    effects.blurTextureWidth = width;
    effects.blurTextureHeight = height;

    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, effects.blurTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, effects.blurFramebuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, effects.blurTextures[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void ui2dRunBlurPasses(UiFrame& frame, UiEffectSettings& effects, float alpha)
{
    int targetWidth = std::max(1, static_cast<int>(std::round(frame.width)));
    int targetHeight = std::max(1, static_cast<int>(std::round(frame.height)));
    ui2dEnsureBlurTargets(effects, targetWidth, targetHeight);

    GLint prevFramebuffer = 0;
    GLint prevViewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    glUseProgram(uiBlurShader);
    glUniform1f(glGetUniformLocation(uiBlurShader, "blurRadius"), effects.blurRadius);
    glUniform1f(glGetUniformLocation(uiBlurShader, "alphaValue"), 1.0f);
    glUniform3f(glGetUniformLocation(uiBlurShader, "color"), 1.0f, 1.0f, 1.0f);
    glUniform2f(glGetUniformLocation(uiBlurShader, "targetSize"), static_cast<float>(targetWidth), static_cast<float>(targetHeight));

    glViewport(0, 0, targetWidth, targetHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, effects.blurFramebuffers[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame.texture);
    glUniform1i(glGetUniformLocation(uiBlurShader, "tex"), 0);
    glUniform2f(glGetUniformLocation(uiBlurShader, "direction"), 1.0f, 0.0f);
    glQuadDraw(0.0f, 0.0f, frame.width, frame.height, uiBlurShader);

    glBindFramebuffer(GL_FRAMEBUFFER, effects.blurFramebuffers[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, effects.blurTextures[0]);
    glUniform1i(glGetUniformLocation(uiBlurShader, "tex"), 0);
    glUniform2f(glGetUniformLocation(uiBlurShader, "direction"), 0.0f, 1.0f);
    glQuadDraw(0.0f, 0.0f, frame.width, frame.height, uiBlurShader);

    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    glUseProgram(uiDefaultShader);
    glUniform1f(glGetUniformLocation(uiDefaultShader, "alphaValue"), alpha);
    glUniform3f(glGetUniformLocation(uiDefaultShader, "color"), frame.color.r, frame.color.g, frame.color.b);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, effects.blurTextures[1]);
    glUniform1i(glGetUniformLocation(uiDefaultShader, "tex"), 0);
    glQuadDraw(frame.x, frame.y, frame.width, frame.height, uiDefaultShader);
}

static void ui2dEnsureShaders()
{
    if (uiDefaultShader != 0) {
        return;
    }

    uiDefaultShader = glGenerateShader(
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
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / vec2(SCREEN_WIDTH, SCREEN_HEIGHT)) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y;
            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
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

    uiBlurShader = glGenerateShader(
        R"(
        #version 300 es
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        uniform vec2 position;
        uniform vec2 scale;
        uniform vec2 targetSize;

        void main() {
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / targetSize) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y;
            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
        R"(
        #version 300 es
        precision mediump float;

        in vec2 TexCoord;
        uniform sampler2D tex;
        uniform float alphaValue;
        uniform vec3 color;
        uniform float blurRadius;
        uniform vec2 direction;

        out vec4 FragColor;

        void main() {
            vec4 baseColor = texture(tex, TexCoord);
            if (blurRadius <= 0.01) {
                baseColor.rgb *= color;
                baseColor.a *= alphaValue;
                FragColor = baseColor;
                return;
            }

            vec2 texel = blurRadius * (1.0 / vec2(textureSize(tex, 0))) * direction;
            float w0 = 0.227027;
            float w1 = 0.1945946;
            float w2 = 0.1216216;

            vec4 result = texture(tex, TexCoord) * w0;
            result += texture(tex, TexCoord + texel) * w1;
            result += texture(tex, TexCoord - texel) * w1;
            result += texture(tex, TexCoord + texel * 2.0) * w2;
            result += texture(tex, TexCoord - texel * 2.0) * w2;

            result.rgb *= color;
            result.a *= alphaValue;
            FragColor = result;
        }
        )"
    );

    uiShadowShader = glGenerateShader(
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
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / vec2(SCREEN_WIDTH, SCREEN_HEIGHT)) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y;
            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
        R"(
        #version 300 es
        precision mediump float;

        in vec2 TexCoord;
        uniform sampler2D tex;
        uniform float alphaValue;
        uniform float blurRadius;
        uniform vec4 shadowColor;

        out vec4 FragColor;

        float blurredAlpha(vec2 uv) {
            if (blurRadius <= 0.01) {
                return texture(tex, uv).a;
            }

            vec2 texOffset = blurRadius * (1.0 / vec2(textureSize(tex, 0)));
            float kernel[5] = float[](1.0, 4.0, 7.0, 4.0, 1.0);
            float result = 0.0;
            float weightSum = 0.0;

            for (int x = -2; x <= 2; ++x) {
                float wx = kernel[x + 2];
                for (int y = -2; y <= 2; ++y) {
                    float wy = kernel[y + 2];
                    float w = wx * wy;
                    vec2 offset = vec2(float(x), float(y)) * texOffset;
                    result += texture(tex, uv + offset).a * w;
                    weightSum += w;
                }
            }

            return result / weightSum;
        }

        void main() {
            float a = blurredAlpha(TexCoord);
            float outAlpha = a * shadowColor.a * alphaValue;
            FragColor = vec4(shadowColor.rgb, outAlpha);
        }
        )"
    );

    uiInnerShadowShader = glGenerateShader(
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
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / vec2(SCREEN_WIDTH, SCREEN_HEIGHT)) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y;
            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
        R"(
        #version 300 es
        precision mediump float;

        in vec2 TexCoord;
        uniform sampler2D tex;
        uniform float alphaValue;
        uniform float blurRadius;
        uniform vec2 shadowOffset;
        uniform vec4 shadowColor;

        out vec4 FragColor;

        float blurredAlpha(vec2 uv) {
            if (blurRadius <= 0.01) {
                return texture(tex, uv).a;
            }

            vec2 texOffset = blurRadius * (1.0 / vec2(textureSize(tex, 0)));
            float kernel[5] = float[](1.0, 4.0, 7.0, 4.0, 1.0);
            float result = 0.0;
            float weightSum = 0.0;

            for (int x = -2; x <= 2; ++x) {
                float wx = kernel[x + 2];
                for (int y = -2; y <= 2; ++y) {
                    float wy = kernel[y + 2];
                    float w = wx * wy;
                    vec2 offset = vec2(float(x), float(y)) * texOffset;
                    result += texture(tex, uv + offset).a * w;
                    weightSum += w;
                }
            }

            return result / weightSum;
        }

        void main() {
            vec2 texel = 1.0 / vec2(textureSize(tex, 0));
            vec2 offset = shadowOffset * texel;
            float baseAlpha = texture(tex, TexCoord).a;
            float blurred = blurredAlpha(TexCoord + offset);
            float inner = clamp(blurred - baseAlpha, 0.0, 1.0);
            float outAlpha = inner * shadowColor.a * alphaValue;
            FragColor = vec4(shadowColor.rgb, outAlpha);
        }
        )"
    );
}

void ui2dInit()
{

    ui2dEnsureShaders();
    mainFrame = ui2dAddFrame(0, 0, 480, 272);

    ioDebugPrint("Initializing 2D UI system\n");

}

// spatial neighbor navigation for UI selection
void navigateUI(int direction) {
    // direction: 0 = up, 1 = down, 2 = left, 3 = right
    /*
    
    // Moving RIGHT: B's left edge must be to the right of A's center
    bool isRight = B.x > (A.x + A.width / 2)

    // Moving LEFT: B's right edge must be left of A's center  
    bool isLeft = (B.x + B.width) < (A.x + A.width / 2)

    // Similar for up/down using y axis

    primaryDist   = distance along the axis you're moving (prefer closer)
    secondaryDist = distance along the perpendicular axis (prefer aligned)

    score = primaryDist + (secondaryDist * weight)  // weight ~2.0-3.0

    function navigate(focused, direction, allElements):
        candidates = filter elements by:
            - not focused itself
            - not disabled/hidden
            - is in the correct direction from focused

        if candidates is empty:
            return focused  // or wrap around

        return min(candidates, by score)
    */

    if (!selectedFrame) {
        selectedFrame = allFrames.empty() ? nullptr : allFrames[0];
        return;
    }

    mainFrame.applyLayout();

    std::vector<UiFrame*> candidates;
    for (UiFrame* frame : allFrames) {

        if (frame == selectedFrame) continue;
        if (!frame->isSelectable()) continue;

        if (direction == 0) { // up
            if (frame->y + frame->height <= selectedFrame->y) {
                // candidate for up
                candidates.push_back(frame);
            }
        } else if (direction == 1) { // down
            if (frame->y >= selectedFrame->y + selectedFrame->height) {
                // candidate for down
                candidates.push_back(frame);
            }
        } else if (direction == 2) { // left
            if (frame->x + frame->width <= selectedFrame->x) {
                // candidate for left
                candidates.push_back(frame);
            }
        } else if (direction == 3) { // right
            if (frame->x >= selectedFrame->x + selectedFrame->width) {
                // candidate for right
                candidates.push_back(frame);
            }
        }

    }

    mainFrame.revertLayout();

    if (candidates.empty()) {
        return; // no candidates, do nothing
    }

    mainFrame.revertLayout();

    UiFrame* bestCandidate = nullptr;
    float bestScore = std::numeric_limits<float>::max();

    for (UiFrame* candidate : candidates) {

        float primaryDist, secondaryDist;

        if (direction == 0 || direction == 1) { // up or down
            primaryDist = std::abs(candidate->y - selectedFrame->y);
            secondaryDist = std::abs((candidate->x + candidate->width / 2) - (selectedFrame->x + selectedFrame->width / 2));
        } else { // left or right
            primaryDist = std::abs(candidate->x - selectedFrame->x);
            secondaryDist = std::abs((candidate->y + candidate->height / 2) - (selectedFrame->y + selectedFrame->height / 2));
        }

        float score = primaryDist + (secondaryDist * 2.0f); // weight of 2.0 for secondary distance

        if (score < bestScore) {
            bestScore = score;
            bestCandidate = candidate;
        }

    }

    mainFrame.revertLayout();

    if (bestCandidate) {
        selectedFrame = bestCandidate;
    }
}

void ui2dUpdate()
{
    mainFrame.calculateAutoSize();

    if (hidIsButtonPressed(GLFW_KEY_UP)) {
        navigateUI(0);
    } else if (hidIsButtonPressed(GLFW_KEY_DOWN)) {
        navigateUI(1);
    } else if (hidIsButtonPressed(GLFW_KEY_LEFT)) {
        navigateUI(2);
    } else if (hidIsButtonPressed(GLFW_KEY_RIGHT)) {
        navigateUI(3);
    } else if (hidIsButtonPressed(GLFW_KEY_ENTER)) {
        if (selectedFrame && selectedFrame->onClick) {
            selectedFrame->onClick(*selectedFrame);
        }
    }
}

void ui2dDraw()
{

    mainFrame.draw_internal();
}

UiFrame &ui2dGetMainFrame()
{
    return mainFrame;
}
UiFrame &ui2dAddFrame(float x, float y, float width, float height)
{
    UiFrame* frame = new UiFrame();
    frame->x = x;
    frame->y = y;
    frame->width = width;
    frame->height = height;


    frame->setParent(mainFrame);

    return *mainFrame.children.back();

}
void UiFrame::setAsMainFrame()
{
    mainFrame = *this;
    selectedFrame = nullptr;

    allFrames.clear();
    allFrames = getAllFrames();
}

bool UiFrame::isSelected()
{
    return selectedFrame == this;
}

void UiFrame::select()
{
    selectedFrame = this;
}
void UiFrame::draw_internal()
{
    if (!allFrames.empty() && std::find(allFrames.begin(), allFrames.end(), this) == allFrames.end()) {
        allFrames.push_back(this);
    }

    std::tuple<float,float> layoutPos = getLayoutPosition();

    float px = x;
    float py = y;
    float pa = alpha;

    x += std::get<0>(layoutPos);
    y += std::get<1>(layoutPos);
    alpha *= parent ? parent->alpha : 1.0f;

    const UiEffectSettings& effects = effectSettings;
    bool hasTexture = texture >= 0;
    bool hasExplicitRender = (onRender != nullptr) || hasTexture || (shader != -1);

    if (effects.dropShadowEnabled && hasTexture) {
        glUseProgram(uiShadowShader);
        glUniform1f(glGetUniformLocation(uiShadowShader, "alphaValue"), alpha);
        glUniform1f(glGetUniformLocation(uiShadowShader, "blurRadius"), effects.dropShadowBlur);
        glUniform4f(glGetUniformLocation(uiShadowShader, "shadowColor"),
            effects.dropShadowColor.r,
            effects.dropShadowColor.g,
            effects.dropShadowColor.b,
            effects.dropShadowColor.a);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(uiShadowShader, "tex"), 0);

        glQuadDraw(x + effects.dropShadowOffsetX, y + effects.dropShadowOffsetY, width, height, uiShadowShader);
    }

    if (this->onRender) {
        int shaderToUse = (shader != -1) ? shader : uiDefaultShader;
        glUseProgram(shaderToUse);
        glUniform1f(glGetUniformLocation(shaderToUse, "alphaValue"), alpha);
        glUniform3f(glGetUniformLocation(shaderToUse, "color"), color.r, color.g, color.b);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderToUse, "tex"), 0);

        this->onRender(*this);
    } else if (hasExplicitRender) {
        if (effects.blurEnabled && hasTexture) {
            ui2dRunBlurPasses(*this, effectSettings, alpha);
        } else {
            int shaderToUse = (shader != -1) ? shader : uiDefaultShader;
            glUseProgram(shaderToUse);
            glUniform1f(glGetUniformLocation(shaderToUse, "alphaValue"), alpha);
            glUniform3f(glGetUniformLocation(shaderToUse, "color"), color.r, color.g, color.b);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(glGetUniformLocation(shaderToUse, "tex"), 0);

            glQuadDraw(x, y, width, height, shaderToUse);
        }
    }

    if (effects.innerShadowEnabled && hasTexture) {
        glUseProgram(uiInnerShadowShader);
        glUniform1f(glGetUniformLocation(uiInnerShadowShader, "alphaValue"), alpha);
        glUniform1f(glGetUniformLocation(uiInnerShadowShader, "blurRadius"), effects.innerShadowBlur);
        glUniform2f(glGetUniformLocation(uiInnerShadowShader, "shadowOffset"),
            effects.innerShadowOffsetX,
            effects.innerShadowOffsetY);
        glUniform4f(glGetUniformLocation(uiInnerShadowShader, "shadowColor"),
            effects.innerShadowColor.r,
            effects.innerShadowColor.g,
            effects.innerShadowColor.b,
            effects.innerShadowColor.a);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(uiInnerShadowShader, "tex"), 0);
        glQuadDraw(x, y, width, height, uiInnerShadowShader);
    }

    for (UiFrame* child : children) {
        child->draw_internal();
    }


    x = px;
    y = py;
    alpha = pa;
}

std::tuple<float, float> UiFrame::getLayoutPosition()
{

    float px = parent->x;
    float py = parent->y;


    if (parent == nullptr || parent->layoutSettings.type == UiLayoutSettings::None) {

        if (parent && parent->layoutSettings.relativeChildren) {
            return std::make_tuple(px, py);
        }

        return std::make_tuple(0, 0);
    }


    const auto& ls = parent->layoutSettings;

    // layout
    if (ls.type == UiLayoutSettings::Grid) {
        if (ls.columns <= 0 || ls.rows <= 0) {
            return std::make_tuple(px, py);
        }

        int index = 0;
        for (UiFrame* child : parent->children) {
            if (child == this) break;
            index++;
        }

        int col = index % ls.columns;
        int row = index / ls.columns;

        float availableWidth  = parent->width  - ls.paddingLeft - ls.paddingRight;
        float availableHeight = parent->height - ls.paddingTop  - ls.paddingBottom;

        float cellWidth  = (availableWidth  - ls.spacingX * (ls.columns - 1)) / ls.columns;
        float cellHeight = (availableHeight - ls.spacingY * (ls.rows    - 1)) / ls.rows;

        px += ls.paddingLeft + col * (cellWidth  + ls.spacingX);
        py += ls.paddingTop  + row * (cellHeight + ls.spacingY);

        // alignment within cell
        if (ls.alignmentX == UiLayoutSettings::Center)
            px += (cellWidth - width) / 2.0f;
        else if (ls.alignmentX == UiLayoutSettings::End)
            px += cellWidth - width;

        if (ls.alignmentY == UiLayoutSettings::Center)
            py += (cellHeight - height) / 2.0f;
        else if (ls.alignmentY == UiLayoutSettings::End)
            py += cellHeight - height;

    } else if (ls.type == UiLayoutSettings::Vertical) {
    py += ls.paddingTop;

    for (UiFrame* child : parent->children) {
        if (child == this) break;
        py += child->height + ls.spacingY;
    }

    if (ls.alignmentX == UiLayoutSettings::Start) {
        px += ls.paddingLeft;
    } else if (ls.alignmentX == UiLayoutSettings::Center) {
        px += (parent->width - width) / 2.0f;
    } else if (ls.alignmentX == UiLayoutSettings::End) {
        px += parent->width - width - ls.paddingRight;
    }

    // missing: y alignment of the whole block within parent
    float totalHeight = 0;
    for (UiFrame* child : parent->children) {
        totalHeight += child->height + ls.spacingY;
    }
    totalHeight -= ls.spacingY; // remove trailing spacing

    if (ls.alignmentY == UiLayoutSettings::Center) {
        py += (parent->height - ls.paddingTop - ls.paddingBottom - totalHeight) / 2.0f;
    } else if (ls.alignmentY == UiLayoutSettings::End) {
        py += (parent->height - ls.paddingTop - ls.paddingBottom - totalHeight);
    }

} else if (ls.type == UiLayoutSettings::Horizontal) {
    px += ls.paddingLeft;

    for (UiFrame* child : parent->children) {
        if (child == this) break;
        px += child->width + ls.spacingX;
    }

    if (ls.alignmentY == UiLayoutSettings::Start) {
        py += ls.paddingTop;
    } else if (ls.alignmentY == UiLayoutSettings::Center) {
        py += (parent->height - height) / 2.0f;
    } else if (ls.alignmentY == UiLayoutSettings::End) {
        py += parent->height - height - ls.paddingBottom;
    }

    // missing: x alignment of the whole block within parent
    float totalWidth = 0;
    for (UiFrame* child : parent->children) {
        totalWidth += child->width + ls.spacingX;
    }
    totalWidth -= ls.spacingX; // remove trailing spacing

    if (ls.alignmentX == UiLayoutSettings::Center) {
        px += (parent->width - ls.paddingLeft - ls.paddingRight - totalWidth) / 2.0f;
    } else if (ls.alignmentX == UiLayoutSettings::End) {
        px += (parent->width - ls.paddingLeft - ls.paddingRight - totalWidth);
    }
}

    return std::make_tuple(px, py);
}

void UiFrame::calculateAutoSize()
{
    // recurse children first (bottom-up)
    for (UiFrame* child : children) {
        child->calculateAutoSize();
    }

    const auto& ls = layoutSettings;

    if (!ls.autoSizeWidth && !ls.autoSizeHeight) return;
    if (ls.type == UiLayoutSettings::None) return;

    float contentWidth  = 0;
    float contentHeight = 0;

    if (ls.type == UiLayoutSettings::Vertical) {
        for (UiFrame* child : children) {
            contentWidth   = std::max(contentWidth, child->width);
            contentHeight += child->height + ls.spacingY;
        }
        if (!children.empty()) contentHeight -= ls.spacingY; // remove trailing

    } else if (ls.type == UiLayoutSettings::Horizontal) {
        for (UiFrame* child : children) {
            contentWidth  += child->width + ls.spacingX;
            contentHeight  = std::max(contentHeight, child->height);
        }
        if (!children.empty()) contentWidth -= ls.spacingX;

    } else if (ls.type == UiLayoutSettings::Grid) {
        if (ls.columns <= 0 || ls.rows <= 0) return;

        // find the largest cell
        float maxCellW = 0, maxCellH = 0;
        for (UiFrame* child : children) {
            maxCellW = std::max(maxCellW, child->width);
            maxCellH = std::max(maxCellH, child->height);
        }

        contentWidth  = ls.columns * maxCellW + ls.spacingX * (ls.columns - 1);
        contentHeight = ls.rows    * maxCellH + ls.spacingY * (ls.rows    - 1);
    }

    if (ls.autoSizeWidth)
        width  = contentWidth  + ls.paddingLeft + ls.paddingRight;

    if (ls.autoSizeHeight) {
        height = contentHeight + ls.paddingTop  + ls.paddingBottom;
    }
}

void UiFrame::setParent(UiFrame& parent)
{
    // remove from old parent if exists
    if (this->parent) {
        auto& children = this->parent->children;
        children.erase(std::remove(children.begin(), children.end(), this), children.end());
    }

    parent.children.push_back(this);
    this->parent = &parent;


}

void UiFrame::applyLayout()
{
    preLayout = std::make_tuple(x, y);

    std::tuple<float,float> layoutPos = getLayoutPosition();

    x += std::get<0>(layoutPos);
    y += std::get<1>(layoutPos);

    for (UiFrame* child : children) {
        child->applyLayout();
    }
}

void UiFrame::revertLayout()
{
    x = std::get<0>(preLayout);
    y = std::get<1>(preLayout);

    for (UiFrame* child : children) {
        child->revertLayout();
    }
}

bool UiFrame::isSelectable()
{
    return canSelect && onRender != nullptr && onClick != nullptr;
}