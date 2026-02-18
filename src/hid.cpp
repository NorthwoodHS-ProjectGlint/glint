#include "glint/hid/hid.h"
#include <GLFW/glfw3.h>
#include <map>
#include <glint/gl/gl.h>

static std::map<int, int> buttonState;

void hidInit() {
    // Initialize button states to released
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        buttonState[key] = GLFW_RELEASE;
    }
}

void hidFlush()
{
    glfwPollEvents();
}

void hidRefreshButtonState(int button)
{
    if (glGetContext() == nullptr) {
        return; // No context, cannot refresh button state
    }

    buttonState[button] = glfwGetKey((GLFWwindow*)glGetContext(), button);
}

bool hidIsButtonPressed(int button) 
{

    bool wasNotPressed = false;
    if (buttonState.find(button) == buttonState.end()) {
        wasNotPressed = true; // Button state not tracked yet, consider it as not pressed
    } else {
        wasNotPressed = buttonState[button] == GLFW_RELEASE; // Check if it was previously released
    }

    hidRefreshButtonState(button);
    return wasNotPressed && buttonState[button] == GLFW_PRESS;
}

bool hidIsButtonDown(int button)
{
    hidRefreshButtonState(button);
    return buttonState[button] == GLFW_PRESS;
}

bool hidIsButtonUp(int button)
{
    hidRefreshButtonState(button);
    return buttonState[button] == GLFW_RELEASE;
}

float hidGetAxis(int axis)
{
    return 0.0f; // Placeholder, implement actual axis reading logic based on your input handling
}
