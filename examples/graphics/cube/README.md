# Glint Application Template

A minimal, ready-to-use template for creating Glint applications.

## Quick Start

```bash
# Copy this template to start your project
cp -r examples/app-template ../my_game
cd ../my_game

# Build and test
sh ./build.sh
```

## Customization

### 1. Update Title Information

Edit `.titleconfig`:

```json
{
  "title": {
    "id": "000400000000001",        // Change to unique ID
    "name": "My Awesome Game",       // Your game's name
    "description": "A fun game!",    // Short description
    "icon": "icon.png",              // Path to your icon
    "tags": ["game", "action"],      // Up to 3 tags
    "resources": "res/"              // Resource directory
  }
}
```

**Important**: Use a unique title ID! Format: `000400000000XXX` for user games.

### 2. Replace the Icon

Replace `icon.png` with your own 128x128 pixel PNG image.

### 3. Add Your Code

Edit `src/main.cpp` - the template includes all required functions:

```cpp
extern "C" void glattach(void* ctx);     // GL context setup
extern "C" void app_setup();              // Initialize your game
extern "C" int app_cycle();               // Update & render loop
extern "C" int app_present();             // Present frame
extern "C" void app_shutdown();           // Cleanup
```

### 4. Add Resources

Place your game assets in `res/`:

```
res/
├── sprites/
│   ├── player.png
│   └── enemy.png
├── sounds/
│   └── music.wav
└── data/
    └── levels.json
```

Access resources in code using the `H:/` mount point:

```cpp
int texture = glGenerateTexture("H:/sprites/player.png", 4);
```

## Building

### Option 1: Using the build script

```bash
sh ./build.sh
```

### Option 2: Manual build

```bash
mkdir -p _build && cd _build
cmake ..
make
cd ..
```

Your `.glt` file will be created in the project directory with the filename `[titleid].glt`.

## Running

Copy your `.glt` file to the Glint device's `titles/` directory:

```bash
cp 000400000000001.glt /path/to/_device/titles/
```

Then launch the Glint bootloader - your game will appear in the homescreen!

## Project Structure

```
my_game/
├── .titleconfig          # Game metadata
├── CMakeLists.txt        # Build configuration
├── build.sh              # Build script
├── icon.png              # Game icon (128x128)
├── src/
│   └── main.cpp          # Your game code
└── res/
    └── file.txt          # Your resources
```

## Next Steps

1. **Read the API docs**: See [API_REFERENCE.md](../../API_REFERENCE.md) for available functions
2. **Study the framework**: Check [APPLICATION_FRAMEWORK.md](../../APPLICATION_FRAMEWORK.md) for lifecycle details
3. **Explore examples**: Look at the homescreen source in `system/apps/homescreen/` for a more complex example

## Common Tasks

### Loading textures

```cpp
extern "C" void app_setup() {
    int sprite = glGenerateTexture("H:/sprites/player.png", 4);
    glBindTexture(GL_TEXTURE_2D, sprite);
}
```

### Handling Input

```cpp
extern "C" int app_cycle() {
    if (hidIsButtonPressed(GLFW_KEY_SPACE)) {
        player_jump();
    }
    
    if (hidIsButtonDown(GLFW_KEY_W)) {
        player_move_forward();
    }
    
    return 0; // Continue running
}
```

### Saving Data

```cpp
void save_game() {
    SaveData data = { level, score, health };
    fsWriteFile("saves/progress.dat", &data, sizeof(SaveData));
}

void load_game() {
    size_t size;
    const void* data = fsReadFile("saves/progress.dat", &size);
    if (data) {
        SaveData* save = (SaveData*)data;
        level = save->level;
        score = save->score;
    }
}
```

### Debug Output

```cpp
ioDebugPrint("Player position: %.2f, %.2f\n", x, y);
glDebugTextFmt("Score: %d", score);
```

## Tips

- **Start simple**: Get something rendering before adding complexity
- **Test frequently**: Use `build.sh` to quickly test changes
- **Check console**: Use `ioDebugPrint()` to debug issues
- **Use resources**: Package assets in `res/` for easy deployment
- **Follow conventions**: Study the homescreen code for best practices

## Troubleshooting

**Build fails with "Package 'glint' not found"**
- Make sure you've built and installed the Glint library first (`sudo make install`)

**Game doesn't appear in homescreen**
- Check that your `.glt` file is in the `titles/` directory
- Verify your title ID is unique and correctly formatted

**Resources not loading**
- Make sure resources are in the `res/` directory
- Use the `H:/` prefix when accessing resources
- Check that `resources` in `.titleconfig` points to the right directory

**Black screen on launch**
- Verify all 5 required functions are implemented
- Check that `glattach()` calls `glAttach(ctx)`
- Ensure `app_present()` returns `glRunning()`

## License

This template is part of the Glint project and is released under the MIT License.

Feel free to use it as a starting point for your own projects!
