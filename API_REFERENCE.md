# Glint API Reference

## Overview

The Glint API provides a comprehensive set of functions for developing games and applications for the Glint handheld operating system. All APIs are accessible by including the main header:

```cpp
#include <glint/glint.h>
```

Individual modules can also be included separately.

---

## I/O Module

### Header
```cpp
#include <glint/io/io.h>
```

### Functions

#### `ioDebugPrint`
```cpp
void ioDebugPrint(const char* format, ...);
```

Print formatted debug messages to the system console.

**Parameters**:
- `format`: Printf-style format string
- `...`: Variable arguments for formatting

**Example**:
```cpp
ioDebugPrint("Player health: %d\n", health);
ioDebugPrint("Loading level: %s\n", level_name);
```

**Notes**:
- Output appears in the system console, not in-game
- Useful for debugging and logging
- Supports all standard printf format specifiers

---

## Filesystem Module

### Header
```cpp
#include <glint/io/fs.h>
```

### Directory Functions

#### `fsCreateDirectory`
```cpp
void fsCreateDirectory(const char* path);
```

Creates a directory at the specified path.

**Parameters**:
- `path`: Directory path to create

**Example**:
```cpp
fsCreateDirectory("saves/");
fsCreateDirectory("saves/profile1/");
```

#### `fsDirectoryExists`
```cpp
bool fsDirectoryExists(const char* path);
```

Check if a directory exists.

**Parameters**:
- `path`: Directory path to check

**Returns**: `true` if directory exists, `false` otherwise

**Example**:
```cpp
if (!fsDirectoryExists("saves/")) {
    fsCreateDirectory("saves/");
}
```

#### `fsListDirectory`
```cpp
const char** fsListDirectory(const char* path, size_t* out_count);
```

List all files and subdirectories in a directory.

**Parameters**:
- `path`: Directory to list
- `out_count`: Pointer to receive the count of entries

**Returns**: Array of file/directory names

**Example**:
```cpp
size_t count;
const char** files = fsListDirectory("saves/", &count);
for (size_t i = 0; i < count; i++) {
    ioDebugPrint("Found: %s\n", files[i]);
}
```

### File Functions

#### `fsCreateFile`
```cpp
void fsCreateFile(const char* path);
```

Create an empty file.

**Parameters**:
- `path`: File path to create

**Example**:
```cpp
fsCreateFile("saves/savegame.dat");
```

#### `fsWriteFile`
```cpp
void fsWriteFile(const char* path, const void* data, size_t size);
```

Write data to a file (overwrites existing content).

**Parameters**:
- `path`: File path to write to
- `data`: Pointer to data to write
- `size`: Number of bytes to write

**Example**:
```cpp
struct SaveData {
    int level;
    float health;
};

SaveData save = {5, 100.0f};
fsWriteFile("saves/progress.dat", &save, sizeof(SaveData));
```

#### `fsReadFile`
```cpp
const void* fsReadFile(const char* path, size_t* out_size);
```

Read entire file into memory.

**Parameters**:
- `path`: File path to read
- `out_size`: Pointer to receive file size in bytes

**Returns**: Pointer to file data, or `nullptr` on failure

**Example**:
```cpp
size_t size;
const void* data = fsReadFile("saves/progress.dat", &size);
if (data) {
    SaveData* save = (SaveData*)data;
    ioDebugPrint("Loaded level: %d\n", save->level);
}
```

**Important**: The returned data is managed by the system - don't free it manually.

#### `fsFileExists`
```cpp
bool fsFileExists(const char* path);
```

Check if a file exists.

**Parameters**:
- `path`: File path to check

**Returns**: `true` if file exists, `false` otherwise

**Example**:
```cpp
if (fsFileExists("saves/autosave.dat")) {
    load_autosave();
}
```

---

## Graphics Module

### Header
```cpp
#include <glint/gl/gl.h>
```

### Core Graphics Functions

#### `glSetup`
```cpp
void* glSetup();
```

Initialize the graphics system (called internally by the system).

**Returns**: OpenGL context pointer

**Note**: Applications don't typically call this - the system handles it.

#### `glAttach`
```cpp
void glAttach(void* ctx);
```

Attach the current thread to the OpenGL context.

**Parameters**:
- `ctx`: Context pointer from `glattach()` callback

**Example**:
```cpp
extern "C" void glattach(void* ctx) {
    glAttach(ctx);  // Required in every application
}
```

#### `glRunning`
```cpp
bool glRunning();
```

Check if the graphics window is still open.

**Returns**: `true` if running, `false` if window closed

**Example**:
```cpp
extern "C" int app_present() {
    glPresent();
    return glRunning(); // Standard pattern
}
```

#### `glPresent`
```cpp
void glPresent();
```

Swap buffers and present the rendered frame.

**Example**:
```cpp
extern "C" int app_present() {
    glPresent();
    return glRunning();
}
```

**Note**: Only call this in `app_present()`, not in `app_cycle()`.

#### `glGetContext`
```cpp
void* glGetContext();
```

Get the current OpenGL context.

**Returns**: Current GL context pointer

**Usage**: Rarely needed in applications - mainly for internal use.

### Shader Functions

#### `glGenerateShader`
```cpp
int glGenerateShader(const char* vertexSrc, const char* fragmentSrc);
```

Compile and link a shader program.

**Parameters**:
- `vertexSrc`: Vertex shader GLSL source code
- `fragmentSrc`: Fragment shader GLSL source code

**Returns**: Shader program ID, or -1 on failure

**Example**:
```cpp
const char* vertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos, 1.0);
    }
)";

const char* fragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    }
)";

int shader = glGenerateShader(vertexShader, fragmentShader);
if (shader != -1) {
    glUseProgram(shader);
}
```

### Debug Text Functions

#### `glDebugText` (Basic)
```cpp
void glDebugText(const char* text);
```

Display debug text on screen (white text, black background).

**Parameters**:
- `text`: Text to display

**Example**:
```cpp
glDebugText("Hello, Glint!");
```

#### `glDebugText` (With Colors)
```cpp
void glDebugText(unsigned long color, unsigned long bg, const char* text);
```

Display debug text with custom colors.

**Parameters**:
- `color`: Text color (RGB as 0xRRGGBB)
- `bg`: Background color (RGB as 0xRRGGBB)
- `text`: Text to display

**Example**:
```cpp
glDebugText(0xFF0000, 0x000000, "Red text on black");
glDebugText(0x00FF00, 0xFFFFFF, "Green text on white");
```

#### `glDebugTextFmt` (Formatted)
```cpp
void glDebugTextFmt(const char* format, ...);
```

Display formatted debug text (white on black).

**Parameters**:
- `format`: Printf-style format string
- `...`: Variable arguments

**Example**:
```cpp
glDebugTextFmt("Score: %d", score);
glDebugTextFmt("Position: (%.2f, %.2f)", x, y);
```

#### `glDebugTextFmt` (Formatted with Colors)
```cpp
void glDebugTextFmt(unsigned long color, unsigned long bg, const char* format, ...);
```

Display formatted debug text with custom colors.

**Parameters**:
- `color`: Text color (0xRRGGBB)
- `bg`: Background color (0xRRGGBB)
- `format`: Printf-style format string
- `...`: Variable arguments

**Example**:
```cpp
glDebugTextFmt(0xFFFF00, 0x000080, "Health: %d%%", health);
```

**Note**: Debug text is rendered on top of everything and stacks vertically.

---

## Human Interface Device (HID) Module

### Header
```cpp
#include <glint/hid/hid.h>
```

### Initialization

#### `hidInit`
```cpp
void hidInit();
```

Initialize the HID system (called internally by the system).

**Note**: Applications don't need to call this.

#### `hidFlush`
```cpp
void hidFlush();
```

Update HID state/flush input buffers (called internally each frame).

**Note**: Applications don't need to call this.

### Button Input Functions

#### `hidIsButtonPressed`
```cpp
bool hidIsButtonPressed(int button);
```

Check if a button was **just pressed** this frame (single trigger).

**Parameters**:
- `button`: GLFW key/button code

**Returns**: `true` if button was just pressed, `false` otherwise

**Example**:
```cpp
if (hidIsButtonPressed(GLFW_KEY_SPACE)) {
    player_jump();
}

if (hidIsButtonPressed(GLFW_KEY_ENTER)) {
    start_game();
}
```

**Use for**: Actions that should trigger once per press (jump, shoot, menu selection).

#### `hidIsButtonDown`
```cpp
bool hidIsButtonDown(int button);
```

Check if a button is currently **held down**.

**Parameters**:
- `button`: GLFW key/button code

**Returns**: `true` if button is currently down, `false` otherwise

**Example**:
```cpp
if (hidIsButtonDown(GLFW_KEY_W)) {
    player_move_forward();
}

if (hidIsButtonDown(GLFW_KEY_LEFT_SHIFT)) {
    player_sprint();
}
```

**Use for**: Continuous actions (movement, acceleration, holding to charge).

#### `hidIsButtonUp`
```cpp
bool hidIsButtonUp(int button);
```

Check if a button is currently **not pressed**.

**Parameters**:
- `button`: GLFW key/button code

**Returns**: `true` if button is up, `false` otherwise

**Example**:
```cpp
if (hidIsButtonUp(GLFW_KEY_W)) {
    player_stop_moving();
}
```

**Use for**: Detecting when a button is released.

### Axis Input Functions

#### `hidGetAxis`
```cpp
float hidGetAxis(int axis);
```

Get analog axis value.

**Parameters**:
- `axis`: Axis identifier (GLFW axis constants)

**Returns**: Axis value (typically -1.0 to 1.0)

**Example**:
```cpp
float horizontal = hidGetAxis(GLFW_GAMEPAD_AXIS_LEFT_X);
float vertical = hidGetAxis(GLFW_GAMEPAD_AXIS_LEFT_Y);

player_x += horizontal * speed;
player_y += vertical * speed;
```

### Common GLFW Key Codes

#### Keyboard Keys
```cpp
GLFW_KEY_SPACE
GLFW_KEY_ESCAPE
GLFW_KEY_ENTER
GLFW_KEY_TAB
GLFW_KEY_BACKSPACE

GLFW_KEY_A ... GLFW_KEY_Z
GLFW_KEY_0 ... GLFW_KEY_9

GLFW_KEY_UP
GLFW_KEY_DOWN
GLFW_KEY_LEFT
GLFW_KEY_RIGHT

GLFW_KEY_LEFT_SHIFT
GLFW_KEY_LEFT_CONTROL
GLFW_KEY_LEFT_ALT
```

#### Gamepad Buttons
```cpp
GLFW_GAMEPAD_BUTTON_A
GLFW_GAMEPAD_BUTTON_B
GLFW_GAMEPAD_BUTTON_X
GLFW_GAMEPAD_BUTTON_Y

GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER

GLFW_GAMEPAD_BUTTON_START
GLFW_GAMEPAD_BUTTON_BACK

GLFW_GAMEPAD_BUTTON_DPAD_UP
GLFW_GAMEPAD_BUTTON_DPAD_DOWN
GLFW_GAMEPAD_BUTTON_DPAD_LEFT
GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
```

#### Gamepad Axes
```cpp
GLFW_GAMEPAD_AXIS_LEFT_X
GLFW_GAMEPAD_AXIS_LEFT_Y
GLFW_GAMEPAD_AXIS_RIGHT_X
GLFW_GAMEPAD_AXIS_RIGHT_Y
GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
```

---

## Title/Executable Module

### Header
```cpp
#include <glint/sys/title.h>
#include <glint/sys/exec.h>
```

### Data Structures

#### `TitleInfo`
```cpp
struct TitleInfo {
    char id[16];
    char name[32];
    char description[128];
    uint8_t icon_data[128*128];
    char tags[3][16];
};
```

Contains metadata about a title/application.

#### `Executable`
```cpp
struct Executable {
    const void* executable;
    const void* resource;
    uint32_t executable_size;
    uint32_t resource_size;
};
```

Represents a loaded executable with its resources.

### Functions

#### `titleLoadInfo`
```cpp
TitleInfo titleLoadInfo(const char* path);
```

Load title metadata from a `.glt` file without executing it.

**Parameters**:
- `path`: Path to `.glt` file

**Returns**: `TitleInfo` structure with metadata

**Example**:
```cpp
TitleInfo info = titleLoadInfo("titles/000400000000001.glt");
ioDebugPrint("Title: %s\n", info.name);
ioDebugPrint("Description: %s\n", info.description);
```

**Note**: Used by the homescreen to display available titles.

#### `execLoad`
```cpp
Executable execLoad(const char* path);
```

Load an executable from a `.glt` file.

**Parameters**:
- `path`: Path to `.glt` file

**Returns**: `Executable` structure

**Example**:
```cpp
Executable exec = execLoad("titles/000400000000001.glt");
if (exec.executable) {
    // Successfully loaded
}
```

**Note**: Primarily for internal/system use. Applications don't typically load other executables.

---

## OpenGL Support

Glint includes OpenGL support via GLAD. Include the OpenGL headers:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
```

All standard OpenGL functions are available. Use the `glGenerateShader()` helper for shader compilation or use OpenGL directly.

---

## Usage Examples

### Complete Input Handling
```cpp
extern "C" int app_cycle() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Movement (continuous)
    if (hidIsButtonDown(GLFW_KEY_W)) player_y += speed;
    if (hidIsButtonDown(GLFW_KEY_S)) player_y -= speed;
    if (hidIsButtonDown(GLFW_KEY_A)) player_x -= speed;
    if (hidIsButtonDown(GLFW_KEY_D)) player_x += speed;
    
    // Jump (single press)
    if (hidIsButtonPressed(GLFW_KEY_SPACE)) {
        player_jump();
    }
    
    // Pause
    if (hidIsButtonPressed(GLFW_KEY_ESCAPE)) {
        return 2;
    }
    
    return 0;
}
```

### Save/Load System
```cpp
struct SaveData {
    int level;
    float health;
    int score;
};

void save_game() {
    SaveData save = {current_level, player_health, score};
    
    if (!fsDirectoryExists("saves/")) {
        fsCreateDirectory("saves/");
    }
    
    fsWriteFile("saves/slot1.dat", &save, sizeof(SaveData));
    ioDebugPrint("Game saved!\n");
}

void load_game() {
    if (!fsFileExists("saves/slot1.dat")) {
        ioDebugPrint("No save file found\n");
        return;
    }
    
    size_t size;
    const void* data = fsReadFile("saves/slot1.dat", &size);
    
    if (data && size == sizeof(SaveData)) {
        SaveData* save = (SaveData*)data;
        current_level = save->level;
        player_health = save->health;
        score = save->score;
        ioDebugPrint("Game loaded!\n");
    }
}
```

### Debug HUD
```cpp
extern "C" int app_cycle() {
    // Game logic...
    
    // Display debug info
    glDebugTextFmt(0xFFFFFF, 0x000000, "FPS: %d", fps);
    glDebugTextFmt(0x00FF00, 0x000000, "Health: %.0f", player_health);
    glDebugTextFmt(0xFFFF00, 0x000000, "Score: %d", score);
    glDebugTextFmt(0xFF0000, 0x000000, "Enemies: %d", enemy_count);
    
    return 0;
}
```
