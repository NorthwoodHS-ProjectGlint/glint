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

Read entire file into memory (supports resource paths).

**Parameters**:
- `path`: File path to read (can include mount points like "H:/")
- `out_size`: Pointer to receive file size in bytes

**Returns**: Pointer to file data, or `nullptr` on failure

**Example**:
```cpp
// Read from filesystem
size_t size;
const void* data = fsReadFile("saves/progress.dat", &size);
if (data) {
    SaveData* save = (SaveData*)data;
    ioDebugPrint("Loaded level: %d\n", save->level);
}

// Read from embedded resources
const void* imageData = fsReadFile("H:/textures/player.png", &size);
int texture = glGenerateTexture((const unsigned char*)imageData, size, 4);
```

**Important**: The returned data is managed by the system - don't free it manually.

**Resource Paths**: Paths starting with a mount point (e.g., "H:/", "S:/") are automatically loaded from mounted resource packs.

#### `fsFileExists`
```cpp
bool fsFileExists(const char* path);
```

Check if a file exists (supports resource paths).

**Parameters**:
- `path`: File path to check (can include mount points like "H:/")

**Returns**: `true` if file exists, `false` otherwise

**Example**:
```cpp
if (fsFileExists("saves/autosave.dat")) {
    load_autosave();
}

// Check for embedded resource
if (fsFileExists("H:/config.json")) {
    load_config();
}
```

**Note**: Automatically checks mounted resource packs when path starts with a mount point.

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

### Texture Functions

#### `glGenerateTexture` (From Raw Data)
```cpp
int glGenerateTexture(int width, int height, const unsigned char* data, int desiredChannels);
```

Create an OpenGL texture from raw pixel data.

**Parameters**:
- `width`: Texture width in pixels
- `height`: Texture height in pixels
- `data`: Pointer to raw pixel data
- `desiredChannels`: Number of channels (3 for RGB, 4 for RGBA)

**Returns**: Texture ID

**Example**:
```cpp
unsigned char pixels[128 * 128 * 3]; // RGB data
// ... fill pixels ...
int texture = glGenerateTexture(128, 128, pixels, 3);
glBindTexture(GL_TEXTURE_2D, texture);
```

#### `glGenerateTexture` (From Memory)
```cpp
int glGenerateTexture(const unsigned char* data, int dataSize, int desiredChannels = 3);
```

Create a texture from an image file in memory (PNG, JPG, TGA, etc.).

**Parameters**:
- `data`: Pointer to image file data in memory
- `dataSize`: Size of the image data in bytes
- `desiredChannels`: Number of channels to load (default: 3 for RGB)

**Returns**: Texture ID, or 0 on failure

**Example**:
```cpp
size_t size;
const unsigned char* imageData = fsReadFile("H:/texture.png", &size);
int texture = glGenerateTexture(imageData, size, 4); // Load as RGBA
```

#### `glGenerateTexture` (From File)
```cpp
int glGenerateTexture(const char* filePath, int desiredChannels = 3);
```

Create a texture from an image file path (supports resource paths).

**Parameters**:
- `filePath`: Path to image file (can use resource mount points like "H:/")
- `desiredChannels`: Number of channels to load (default: 3 for RGB)

**Returns**: Texture ID, or 0 on failure

**Example**:
```cpp
// Load from embedded resources
int texture = glGenerateTexture("H:/sprites/player.png", 4);

// Load from filesystem
int background = glGenerateTexture("backgrounds/menu.png", 3);
```

**Supported Formats**: PNG, JPG, TGA, BMP, PSD, GIF, HDR, PIC

### Rendering Helpers

#### `glQuadDraw`
```cpp
void glQuadDraw(float x, float y, float width, float height, int shader);
```

Draw a textured quad at the specified position.

**Parameters**:
- `x`: X position in screen coordinates (0-480)
- `y`: Y position in screen coordinates (0-272)
- `width`: Quad width in pixels
- `height`: Quad height in pixels
- `shader`: Shader program to use

**Example**:
```cpp
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, myTexture);
glUniform1i(glGetUniformLocation(shader, "tex"), 0);

glQuadDraw(100, 50, 64, 64, shader);
```

**Note**: Texture must be bound before calling. Position is in screen space (top-left origin).

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

## UI2D Module

### Header
```cpp
#include <glint/gl/ui2d.h>
```

### Overview

The UI2D module provides a complete 2D user interface framework for building interactive menus, HUDs, and UI elements. Features include hierarchical frame management, flexible layout systems, visual effects, and event handling.

### Core UI Types

#### `UiFrame`

The main UI element type. Represents a rectangular UI component with positioning, appearance, layout, and effects.

**Members**:
- `float x, y`: Position on screen
- `float width, height`: Dimensions
- `float alpha`: Opacity (0.0-1.0)
- `ColorRGB color`: Color tint (default white)
- `int texture`: Texture ID (-1 for untextured)
- `int shader`: Custom shader ID (-1 for default)
- `bool canSelect`: If true, frame can be selected/focused
- `std::function<void(UiFrame&)> onRender`: Custom render callback
- `std::function<void(UiFrame&)> onClick`: Click handler callback
- `std::vector<UiFrame*> children`: Child frames
- `UiFrame* parent`: Parent frame

**Key Methods**:

##### `setParent(UiFrame& parent)`
Set this frame's parent for hierarchical organization.

```cpp
UiFrame& buttonFrame = ui2dAddFrame(10, 10, 100, 30);
UiFrame& panelFrame = ui2dAddFrame(0, 0, 320, 240);
buttonFrame.setParent(panelFrame); // Now button is child of panel
```

##### `setAsMainFrame()`
Make this frame the root UI frame.

```cpp
UiFrame& mainPanel = ui2dAddFrame(0, 0, 480, 272);
mainPanel.setAsMainFrame();
```

##### `isSelected()` / `isSelectable()`
Check selection state.

```cpp
if (frame.isSelectable()) {
    frame.select();
}
```

##### `select()`
Set this frame as the currently selected frame.

```cpp
buttonFrame.select(); // Give focus to button
```

##### `getLayoutSettings()`
Access and modify layout configuration.

```cpp
UiLayoutSettings& layout = frame.getLayoutSettings();
layout.type = UiLayoutSettings::Vertical;
layout.spacingY = 10.0f;
frame.applyLayout();
```

##### `getEffectSettings()`
Access and modify visual effects.

```cpp
UiEffectSettings& effects = frame.getEffectSettings();
effects.blurEnabled = true;
effects.blurRadius = 5.0f;
```

##### `applyLayout()`
Recalculate layout for this frame and children.

```cpp
frame.applyLayout(); // Update positions based on layout settings
```

### Layout System

#### `UiLayoutSettings`

Configures how child frames are arranged within a parent frame.

**Layout Types**:
- `None`: Children positioned manually
- `Vertical`: Stack children vertically
- `Horizontal`: Stack children horizontally
- `Grid`: Arrange in rows and columns

**Alignment**:
- `Start`: Top/left alignment
- `Center`: Center alignment
- `End`: Bottom/right alignment

**Example - Vertical Menu**:
```cpp
UiFrame& menu = ui2dAddFrame(100, 50, 200, 400);
UiLayoutSettings& layout = menu.getLayoutSettings();
layout.type = UiLayoutSettings::Vertical;
layout.alignmentX = UiLayoutSettings::Center;
layout.alignmentY = UiLayoutSettings::Start;
layout.spacingY = 15.0f;
layout.paddingTop = 20;
layout.paddingLeft = 10;
layout.paddingRight = 10;

// Add buttons as children - they'll auto-arrange
UiFrame& button1 = ui2dAddFrame(0, 0, 150, 40);
button1.setParent(menu);

UiFrame& button2 = ui2dAddFrame(0, 0, 150, 40);
button2.setParent(menu);

menu.applyLayout();
```

**Example - Grid Layout**:
```cpp
UiFrame& grid = ui2dAddFrame(50, 50, 300, 300);
UiLayoutSettings& layout = grid.getLayoutSettings();
layout.type = UiLayoutSettings::Grid;
layout.columns = 3;
layout.rows = 3;
layout.spacingX = 10.0f;
layout.spacingY = 10.0f;

for (int i = 0; i < 9; i++) {
    UiFrame& cell = ui2dAddFrame(0, 0, 80, 80);
    cell.setParent(grid);
}

grid.applyLayout();
```

### Effects System

#### `UiEffectSettings`

Applies visual effects to frames.

**Blur Effect**:
```cpp
UiFrame& blurredBox = ui2dAddFrame(50, 50, 200, 150);
UiEffectSettings& effects = blurredBox.getEffectSettings();
effects.blurEnabled = true;
effects.blurRadius = 10.0f;
blurredBox.applyLayout();
```

**Drop Shadow**:
```cpp
UiFrame& shadow = ui2dAddFrame(100, 100, 150, 80);
UiEffectSettings& effects = shadow.getEffectSettings();
effects.dropShadowEnabled = true;
effects.dropShadowOffsetX = 5.0f;
effects.dropShadowOffsetY = 5.0f;
effects.dropShadowBlur = 8.0f;
effects.dropShadowColor = {0.0f, 0.0f, 0.0f, 0.5f};
```

**Inner Shadow**:
```cpp
UiFrame& inset = ui2dAddFrame(200, 100, 100, 100);
UiEffectSettings& effects = inset.getEffectSettings();
effects.innerShadowEnabled = true;
effects.innerShadowOffsetX = 2.0f;
effects.innerShadowOffsetY = 2.0f;
effects.innerShadowBlur = 4.0f;
```

### Initialization and Update Functions

#### `ui2dInit()`
```cpp
void ui2dInit();
```

Initialize the UI2D system (called internally, but can reinitialize if needed).

#### `ui2dUpdate()`
```cpp
void ui2dUpdate();
```

Update UI state, handle input, and update selected frame. Call once per frame.

```cpp
extern "C" int app_cycle() {
    ui2dUpdate();
    // Game logic...
    return 0;
}
```

#### `ui2dDraw()`
```cpp
void ui2dDraw();
```

Render all UI frames. Call once per frame after `ui2dUpdate()`.

```cpp
extern "C" int app_cycle() {
    ui2dUpdate();
    ui2dDraw();
    return 0;
}
```

### Main Frame Management

#### `ui2dGetMainFrame()`
```cpp
UiFrame& ui2dGetMainFrame();
```

Get the root UI frame.

**Returns**: Reference to main frame

**Example**:
```cpp
UiFrame& mainFrame = ui2dGetMainFrame();
mainFrame.width = 480;
mainFrame.height = 272;
mainFrame.color = {0.2f, 0.2f, 0.2f};
```

#### `ui2dAddFrame(float x, float y, float width, float height)`
```cpp
UiFrame& ui2dAddFrame(float x, float y, float width, float height);
```

Create a new UI frame at the specified position and size.

**Parameters**:
- `x`: X position
- `y`: Y position
- `width`: Frame width
- `height`: Frame height

**Returns**: Reference to newly created frame

**Example**:
```cpp
UiFrame& button = ui2dAddFrame(50, 200, 100, 40);
button.color = {0.2f, 0.4f, 0.8f};
button.onClick = [](UiFrame& self) {
    ioDebugPrint("Button clicked!\n");
};
```

### Complete UI Example

```cpp
extern "C" void glattach(void* ctx) {
    glAttach(ctx);
}

extern "C" int app_setup() {
    ui2dInit();
    
    // Set up background
    UiFrame& background = ui2dGetMainFrame();
    background.color = {0.1f, 0.1f, 0.15f};
    
    // Create main menu panel
    UiFrame& menu = ui2dAddFrame(160, 60, 160, 180);
    menu.color = {0.2f, 0.2f, 0.25f};
    menu.setParent(background);
    
    // Configure vertical layout
    UiLayoutSettings& layout = menu.getLayoutSettings();
    layout.type = UiLayoutSettings::Vertical;
    layout.alignmentX = UiLayoutSettings::Center;
    layout.spacingY = 15.0f;
    layout.paddingTop = 20;
    
    // Create buttons
    for (int i = 0; i < 3; i++) {
        UiFrame& btn = ui2dAddFrame(0, 0, 120, 30);
        btn.color = {0.3f, 0.5f, 0.8f};
        btn.canSelect = true;
        btn.setParent(menu);
        btn.onClick = [i](UiFrame& self) {
            ioDebugPrint("Button %d clicked\n", i + 1);
        };
    }
    
    menu.applyLayout();
    return 0;
}

extern "C" int app_cycle() {
    ui2dUpdate();
    glClear(GL_COLOR_BUFFER_BIT);
    ui2dDraw();
    return 0;
}

extern "C" int app_present() {
    glPresent();
    return glRunning();
}
```

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
    uint8_t icon_data[128*128*3];  // RGB data
    char tags[3][16];
    int icon_texture;  // OpenGL texture ID (runtime)
};
```

Contains metadata about a title/application.

**Fields**:
- `icon_data`: Raw RGB pixel data (128x128 pixels, 3 bytes per pixel)
- `icon_texture`: OpenGL texture ID created automatically when loading

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

**Returns**: `TitleInfo` structure with metadata and icon texture

**Example**:
```cpp
TitleInfo info = titleLoadInfo("titles/000400000000001.glt");
ioDebugPrint("Title: %s\n", info.name);
ioDebugPrint("Description: %s\n", info.description);

// Icon texture is automatically created
glBindTexture(GL_TEXTURE_2D, info.icon_texture);
```

**Note**: Automatically creates an OpenGL texture from the icon data.

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

#### `execMountResource`
```cpp
void execMountResource(const Executable* exec, char mountPoint[3] = "H:/");
```

Mount an executable's resource pack to a virtual drive letter.

**Parameters**:
- `exec`: Pointer to loaded executable
- `mountPoint`: 3-character mount point (e.g., "H:/", "S:/")

**Example**:
```cpp
Executable exec = execLoad("my_game.glt");
execMountResource(&exec, "H:/");

// Now resources can be accessed via H:/ prefix
const void* data = execGetResource("H:/sprites/player.png", &size);
```

**Note**: System resources are typically mounted to "S:/" by the bootloader.

#### `execGetResource`
```cpp
const void* execGetResource(const char* path, size_t* out_size = nullptr);
```

Get a resource from a mounted resource pack.

**Parameters**:
- `path`: Resource path with mount point (e.g., "H:/textures/sprite.png")
- `out_size`: Optional pointer to receive resource size

**Returns**: Pointer to resource data, or `nullptr` if not found

**Example**:
```cpp
size_t size;
const void* imageData = execGetResource("H:/images/logo.png", &size);
if (imageData) {
    int texture = glGenerateTexture((const unsigned char*)imageData, size, 4);
}
```

**Note**: Resources are read-only and managed by the system.

---

## OpenGL Support

Glint includes OpenGL support via GLAD. Include the OpenGL headers:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
```

All standard OpenGL functions are available. Use the `glGenerateShader()` and texture generation helpers, or use OpenGL directly.

The system uses **OpenGL ES 3.0** with a fixed resolution of **480x272 pixels**.

---

## Resource System

### Mount Points

Glint uses a virtual file system with mount points for accessing packaged resources:

- **H:/** - Your application's resources (mounted automatically by system)
- **S:/** - System resources (homescreen assets, fonts, etc.)

### Resource Workflow

1. **Package resources** in your app's `res/` directory
2. **Resources are mounted** automatically when your app starts
3. **Access via mount point**: Use `fsReadFile("H:/texture.png")` or `glGenerateTexture("H:/texture.png")`

### Example Resource Usage

```cpp
extern "C" void app_setup() {
    // Load texture from embedded resources
    int playerSprite = glGenerateTexture("H:/sprites/player.png", 4);
    
    // Load config file from resources
    size_t size;
    const void* configData = fsReadFile("H:/config.json", &size);
    
    // Resources are read-only and managed by the system
}
```

### Benefits

- **No file I/O overhead**: Resources loaded directly from memory
- **Self-contained apps**: Everything packaged in a single `.glt` file  
- **Fast access**: No disk reads during gameplay
- **Organized**: Clear separation between app resources (H:/) and system resources (S:/)

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
