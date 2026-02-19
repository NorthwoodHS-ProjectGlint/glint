# Glint Application Framework

## Overview

Glint applications are shared libraries that implement specific C-style functions and link against the Glint library. Applications run in separate threads managed by the system's title launcher.

## Required Functions

Every Glint application **must** implement these five exported C functions:

### 1. `glattach(void* ctx)`

Attaches the application to the OpenGL context.

```cpp
extern "C" void glattach(void* ctx) {
    glAttach(ctx);
}
```

**Called**: Once when the application is launched, before `app_setup()`

**Purpose**: Initializes OpenGL context for this thread

**Parameters**:
- `ctx`: OpenGL context provided by the system

**Important**: Always call `glAttach(ctx)` or the application won't render correctly

---

### 2. `app_setup()`

Initialize application resources and state.

```cpp
extern "C" void app_setup() {
    // Load resources
    // Initialize game state
    // Setup OpenGL state
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}
```

**Called**: Once after `glattach()`, before the main loop

**Purpose**: Load resources, initialize variables, setup rendering

**Return**: void

**Use for**:
- Loading textures and models
- Initializing game/app state
- Allocating buffers
- Setting up shaders

---

### 3. `app_cycle()`

Main application loop - update logic and handle input.

```cpp
extern "C" int app_cycle() {
    // Update game logic
    // Handle input
    // Render frame
    
    if (hidIsButtonPressed(GLFW_KEY_ESCAPE)) {
        return 2; // Pause to home screen
    }
    
    if (game_over) {
        return 1; // Exit application
    }
    
    return 0; // Continue running
}
```

**Called**: Repeatedly while the application is running

**Purpose**: Update logic, process input, render graphics

**Return values**:
- `0`: Continue running normally
- `1`: Exit application and return to home screen
- `2`: Pause application and show home screen (app stays in memory)

**Important**: Do not call `glPresent()` here - use `app_present()` instead

---

### 4. `app_present()`

Present the rendered frame to the screen.

```cpp
extern "C" int app_present() {
    glPresent();
    return glRunning();
}
```

**Called**: After every `app_cycle()` call

**Purpose**: Swap buffers and display the rendered frame

**Return values**:
- `1`: Window is still open
- `0`: Window closed (user closed the window)

**Best practice**: Use the template above unless you have specific needs

---

### 5. `app_shutdown()`

Clean up resources before exit.

```cpp
extern "C" void app_shutdown() {
    // Free allocated memory
    // Close file handles
    // Delete textures/buffers
}
```

**Called**: Once when the application exits

**Purpose**: Release resources, save state, cleanup

**Return**: void

**Use for**:
- Deleting OpenGL resources
- Freeing allocated memory
- Saving user data/progress
- Closing files

## Application Lifecycle

```
Launch → glattach() → app_setup() → ┬→ app_cycle() → app_present() ┐
                                     │                              │
                                     └──────────────────────────────┘
                                               (loop while running)
                                                       │
                                                       ↓
                                              app_shutdown() → Exit
```

### Lifecycle States

1. **Loading**: System loads the `.glt` file
2. **Attaching**: `glattach()` called with GL context
3. **Setup**: `app_setup()` initializes the application
4. **Running**: `app_cycle()` and `app_present()` loop
5. **Paused**: Application paused (returns 2), context detached
6. **Resumed**: Context reattached, loop continues
7. **Shutdown**: `app_shutdown()` called before exit

## Pause/Resume System

When an application returns `2` from `app_cycle()`:

1. GL context is detached from the application thread
2. Application thread waits
3. Home screen is shown
4. User can select another app or resume
5. When resumed:
   - GL context is reattached
   - Application loop continues from next `app_cycle()` call

**Important**: Don't perform heavy operations while paused - the thread is simply waiting.

## Complete Example Application

```cpp
#include <glint/glint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Game state
float player_x = 0.0f;
float player_y = 0.0f;
bool game_running = true;
int playerTexture = 0;
int backgroundTexture = 0;

extern "C" void glattach(void* ctx) {
    glAttach(ctx);
}

extern "C" void app_setup() {
    // Initialize rendering
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Load textures from embedded resources
    playerTexture = glGenerateTexture("H:/sprites/player.png", 4);
    backgroundTexture = glGenerateTexture("H:/backgrounds/level1.png", 3);
    
    // Initialize game state
    player_x = 240.0f;  // Center X
    player_y = 136.0f;  // Center Y
    game_running = true;
    
    ioDebugPrint("Game initialized\n");
}

extern "C" int app_cycle() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Handle input
    if (hidIsButtonDown(GLFW_KEY_W)) player_y += 0.01f;
    if (hidIsButtonDown(GLFW_KEY_S)) player_y -= 0.01f;
    if (hidIsButtonDown(GLFW_KEY_A)) player_x -= 0.01f;
    if (hidIsButtonDown(GLFW_KEY_D)) player_x += 0.01f;
    
    // Pause on escape
    if (hidIsButtonPressed(GLFW_KEY_ESCAPE)) {
        ioDebugPrint("Pausing game\n");
        return 2;
    }
    
    // Exit on Q
    if (hidIsButtonPressed(GLFW_KEY_Q)) {
        ioDebugPrint("Exiting game\n");
        return 1;
    }
    
    // Render game
    glDebugTextFmt("Player: (%.2f, %.2f)", player_x, player_y);
    glDebugTextFmt("Press WASD to move, ESC to pause, Q to quit");
    
    // Continue running
    return 0;
}

extern "C" int app_present() {
    glPresent();
    return glRunning();
}

extern "C" void app_shutdown() {
    ioDebugPrint("Game shutting down\n");
    // Free resources
    // Save game state
}
```

## Limitations and Constraints

### Threading Limitations

- **Single thread per title**: Each application runs in its own thread
- **No thread creation**: Creating additional threads is not supported
- **Context switching**: GL context is switched when pausing/resuming

### Memory Limitations

- **Shared library constraints**: Applications are loaded via `dlopen()`
- **No static constructors**: May not work reliably - use `app_setup()` instead
- **Resource cleanup**: Must clean up in `app_shutdown()` - no automatic cleanup

### Rendering Limitations

- **OpenGL ES 3.0**: Uses OpenGL ES 3.0 (not desktop OpenGL)
- **Context managed by system**: Don't create your own GL context
- **Single window**: Applications share the main window
- **Fixed resolution**: 480x272 pixels (PSP-like resolution)
- **Screen coordinates**: Origin at top-left (0,0 to 480,272)

### Input Limitations

- **Polling only**: No event callbacks for input
- **GLFW key codes**: Use GLFW constants for button/key codes
- **Limited axis support**: Check documentation for supported axes

### File System Limitations

- **Virtual filesystem**: Use Glint FS API, not standard C/C++ file I/O
- **Sandboxed access**: Applications can only access their own directories
- **Read-only resources**: Packaged resources are read-only
- **Resource mounting**: Resources must be mounted before use (typically done by system)
- **Mount points**: Resources accessed via virtual drive letters (e.g., "H:/", "S:/")

### API Constraints

- **C linkage required**: All five functions must use `extern "C"`
- **Specific signatures**: Function signatures must match exactly
- **No main() function**: Applications are libraries, not executables
- **Return value semantics**: Must follow documented return values

## Best Practices

### 1. Always Clean Up

```cpp
extern "C" void app_shutdown() {
    // Delete all resources
    if (texture_id) glDeleteTextures(1, &texture_id);
    if (buffer_id) glDeleteBuffers(1, &buffer_id);
    delete[] game_data;
}
```

### 2. Handle Pause/Resume Gracefully

```cpp
extern "C" int app_cycle() {
    // Pause on home button
    if (hidIsButtonPressed(GLFW_GAMEPAD_BUTTON_START)) {
        save_game_state();
        return 2; // Pause
    }
    return 0;
}
```

### 3. Use Debug Output

```cpp
ioDebugPrint("Loading level %d\n", level_id);
glDebugTextFmt("Score: %d", score);
```

### 4. Check Button States Correctly

```cpp
// For single press detection
if (hidIsButtonPressed(GLFW_KEY_SPACE)) {
    player_jump();
}

// For continuous input
if (hidIsButtonDown(GLFW_KEY_W)) {
    player_move_forward();
}
```

### 5. Initialize in Setup, Not Globally

```cpp
// DON'T:
GLuint texture = load_texture(); // May fail or be too early

// DO:
GLuint texture = 0;
extern "C" void app_setup() {
    texture = load_texture(); // Correct timing
}
```

## Debugging Tips

1. **Use ioDebugPrint**: Output to system console for debugging
2. **Check return values**: Verify `app_cycle()` returns correct values
3. **Test pause/resume**: Make sure your app handles pausing correctly
4. **Monitor resources**: Check that resources are freed in `app_shutdown()`
5. **Validate .titleconfig**: Ensure title configuration is correct

## Migration Checklist

Converting a standard application to Glint:

- [ ] Build as shared library, not executable
- [ ] Remove `main()` function
- [ ] Implement all five required functions with `extern "C"`
- [ ] Replace standard file I/O with Glint FS API
- [ ] Move initialization from constructors to `app_setup()`
- [ ] Move cleanup from destructors to `app_shutdown()`
- [ ] Add pause/resume handling to `app_cycle()`
- [ ] Call `glAttach()` in `glattach()` function
- [ ] Call `glPresent()` and `glRunning()` in `app_present()`
- [ ] Create `.titleconfig` file
- [ ] Add resources to resource directory
- [ ] Update CMakeLists.txt to build shared library
- [ ] Add custom target for `glt_execcreate`
