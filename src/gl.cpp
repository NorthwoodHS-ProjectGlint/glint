#include "glint/glint.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "internal.h"
#include "dbg_fontload.h"

static GLFWwindow* g_window = nullptr;
static unsigned int g_debugTextTexture = 0;
static std::map<uint32_t, BMFChar> g_debugFontMap;
static unsigned int g_debugQuadVBO = 0;
static unsigned int g_debugQuadVAO = 0;

static unsigned int g_debugCubeVBO = 0;
static unsigned int g_debugCubeEBO = 0;
static unsigned int g_debugCubeVAO = 0;

static unsigned int g_debugTextShader = 0; // Placeholder for shader program ID
static unsigned int g_defaultShader = 0; // Placeholder for default shader program ID
static int cursorY = 0; // Moved cursorY declaration here to avoid unused variable warning

static double lastTimestamp = 0.0;
static double deltaTime = 1.0 / 60.0; // Default to 60 FPS

static mat4 projectionMatrix;
static mat4 viewMatrix;

void* glSetup()
{
    if (!glfwInit()) {
        ioDebugPrint("Failed to initialize GLFW\n");
        return nullptr;
    }

    // Tell GLFW to use OpenGL ES 2.0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // 480x272 is a PSP-like resolution, good starting point
    GLFWwindow* window = glfwCreateWindow(480, 272, "Glint", nullptr, nullptr);
    if (!window) {
        ioDebugPrint("Failed to create GLFW window\n");
        glfwTerminate();
        return nullptr;
    }

    g_window = window;
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        ioDebugPrint("Failed to initialize GLAD\n");
        return nullptr;
    }    

    glfwSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setup debug text texture
    g_debugTextTexture = glGenerateTexture(bmfont_0, sizeof(bmfont_0), 3);
    g_debugFontMap = LoadBMFontBinary(bmfont, sizeof(bmfont));

    ioDebugPrint("Debug text texture generated with ID: %u\n", g_debugTextTexture);

    {
        // create debug text quad
        glGenBuffers(1, &g_debugQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, g_debugQuadVBO);
        
        // Define a simple quad for debug text rendering
        float quadVertices[] = {
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &g_debugQuadVAO);
        glBindVertexArray(g_debugQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, g_debugQuadVBO);
        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); // TexCoords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    {
        glGenBuffers(1, &g_debugCubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, g_debugCubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &g_debugCubeEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_debugCubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &g_debugCubeVAO);
        glBindVertexArray(g_debugCubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, g_debugCubeVBO);
        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); // Normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); // TexCoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // generate shader for debug text rendering
    g_debugTextShader = glGenerateShader(
        // Vertex shader
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

            // generate screen-space position
            vec2 scaledPos = aPos * scale;
            vec2 screenPos = position + scaledPos;
            vec2 ndcPos = (screenPos / vec2(SCREEN_WIDTH, SCREEN_HEIGHT)) * 2.0 - 1.0;
            ndcPos.y = -ndcPos.y; // Flip Y for OpenGL coordinate system

            gl_Position = vec4(ndcPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
        )",
        // Fragment shader
        R"(
        #version 300 es
        precision mediump float;
        
        in vec2 TexCoord;
        uniform sampler2D textTexture;
        
        out vec4 FragColor;

        uniform vec2 char_position;

        uniform vec4 textColor;
        uniform vec4 bgColor;
        
        void main() {
            vec2 adjustedTexCoord = (TexCoord * vec2(8,16) + char_position) / vec2(256, 64);
            float alpha = texture(textTexture, adjustedTexCoord).r;

            FragColor = mix(bgColor, textColor, alpha);

            //FragColor = vec4(adjustedTexCoord, 0.0, 1.0); // Debug: visualize texture coordinates
        }
        )"
    );

    // generate shader for default rendering
    g_defaultShader = glGenerateShader(
        // Vertex shader
        R"(
        #version 300 es
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;
        
        out vec3 Position;
        out vec3 Normal;
        out vec2 TexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            Position = aPos;
            Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal to world space
            TexCoord = aTexCoord;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
        )",
        // Fragment shader
        R"(
        #version 300 es
        precision mediump float;
        
        out vec4 FragColor;

        in vec3 Position;
        in vec3 Normal;
        in vec2 TexCoord;
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(Normal, 1.0);
        }
        )"
    );

    return window;
}

bool glRunning()
{
    return !glfwWindowShouldClose((GLFWwindow*)glGetContext());
}

void glAttach(void *ctx)
{
   ioDebugPrint("glattach called with ctx: %p\n", ctx);


    GLFWwindow* window = static_cast<GLFWwindow*>(ctx);
    if (!window) {
        window = g_window;
    }

    if (!window) {
        ioDebugPrint("glAttach called with no window context\n");
        return;
    }

    ioDebugPrint("Making OpenGL context current for window: %p\n", window);

    glfwMakeContextCurrent(window);

    ioDebugPrint("OpenGL context attached successfully\n");
}

void glPresent()
{
    if (!g_window) {
        return;
    }

    hidFlush(); // Flush input state before checking if we should close
    glfwSwapBuffers(g_window);
    cursorY = 0; // Reset cursor Y position after presenting



    double currentTimestamp = glfwGetTime();
    deltaTime = currentTimestamp - lastTimestamp;
    lastTimestamp = currentTimestamp;
}

double glGetTime()
{
    return glfwGetTime();
}

double glGetDeltaTime()
{
    return deltaTime;
}

int glGenerateShader(const char *vertexSrc, const char *fragmentSrc)
{
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, nullptr);
    glCompileShader(vertex);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        ioDebugPrint("Vertex shader compilation failed: %s\n", infoLog);
        glDeleteShader(vertex);
        return 0;
    }
    
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, nullptr);
    glCompileShader(fragment);
    
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        ioDebugPrint("Fragment shader compilation failed: %s\n", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        ioDebugPrint("Shader program linking failed: %s\n", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        return 0;
    }
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    return program;
}

int glGenerateTexture(int width, int height, const unsigned char *data, int desiredChannels)
{
    // 4. Standard OpenGLES Texture Upload
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    // Critical: Font textures often have non-power-of-two widths
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = (desiredChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return tex;
}
int glGenerateTexture(const unsigned char *data, int dataSize, int desired_channels)
{
    
    int width, height, channels_in_file;
    // Request 4 components (RGBA) for consistency, but you can use 0 to let it decide.

    unsigned char* image_data = stbi_load_from_memory(
        data,
        dataSize,
        &width,
        &height,
        &channels_in_file,
        desired_channels
    );

    if (image_data == NULL) {
        ioDebugPrint("Failed to load TGA image from memory: %s\n", stbi_failure_reason());
        return 0;
    }

    // 4. Standard OpenGLES Texture Upload
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    // Critical: Font textures often have non-power-of-two widths
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = (channels_in_file == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return tex;
}

int glGenerateTexture(const char *filePath, int desiredChannels)
{
    int size;
    const void* fileData = fsReadFile(filePath, (size_t*)&size);
    return glGenerateTexture((const unsigned char*)fileData, size, desiredChannels);
}

void glDebugText(const char *text)
{
    glDebugTextFmt("%s", text);
}

void glDebugTextFmt(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    glDebugText(0xFFFFFFFF, 0x000000FF, buffer);
}

void glDebugTextFmt(unsigned long color, unsigned long bg, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    glDebugText(color, bg, buffer);
}



void glDebugText(unsigned long color, unsigned long bg, const char *text)
{
    // iterate through the text and render each character using g_debugFontMap and g_debugTextTexture

    int cursorX = 0;

    
    float textScale = 2.0f;

    for (const char* p = text; *p; ++p) {
        char c = *p;
        if (c == '\n') {
            // Handle newlines (move to next line)
            // This is a simplified example, you would need to track cursor position
            cursorX = 0;
            cursorY += 16 * textScale; // Move down by one line height (assuming 16px line height)
            continue;
        }

        auto it = g_debugFontMap.find(c);
        if (it != g_debugFontMap.end()) {
            const BMFChar& ch = it->second;
            // Render character 'ch' using g_debugTextTexture and the quad defined in g_debugQuadVBO
            // You would need to set up your shader and draw calls here

            glBindVertexArray(g_debugQuadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_debugTextTexture);

            glUseProgram(g_debugTextShader);
            // Set shader uniforms for position, scale, color, character position, etc.
            glUniform2f(glGetUniformLocation(g_debugTextShader, "position"), cursorX + ch.xoffset, cursorY - ch.yoffset);
            glUniform2f(glGetUniformLocation(g_debugTextShader, "char_position"), ch.x, ch.y);
            glUniform2f(glGetUniformLocation(g_debugTextShader, "scale"), ch.width * textScale, ch.height * textScale);
            glUniform1i(glGetUniformLocation(g_debugTextShader, "texture"), 0);
            glUniform4f(glGetUniformLocation(g_debugTextShader, "textColor"), 
                ((color >> 24) & 0xFF) / 255.0f, 
                ((color >> 16) & 0xFF) / 255.0f, 
                ((color >> 8) & 0xFF) / 255.0f, 
                (color & 0xFF) / 255.0f
            );

                glUniform4f(glGetUniformLocation(g_debugTextShader, "bgColor"), 
                ((bg >> 24) & 0xFF) / 255.0f, 
                ((bg >> 16) & 0xFF) / 255.0f, 
                ((bg >> 8) & 0xFF) / 255.0f, 
                (bg & 0xFF) / 255.0f
            );


            glDrawArrays(GL_TRIANGLES, 0, 6);

            cursorX += ch.xadvance * textScale;
        } else {
            // Character not found in font map, skip or render a placeholder
            cursorX += 8 * textScale; // Move cursor forward by a default width
        }
    }

    cursorY += 16 * textScale; // Move down by one line height (assuming 16px line height)
}

void *glGetContext()
{
    return g_window;
}

void glQuadDraw(float x, float y, float width, float height, int shader)
{
    glBindVertexArray(g_debugQuadVAO);
    glUseProgram(shader);

    // Set shader uniforms for position, scale, color, character position, etc.
    glUniform2f(glGetUniformLocation(shader, "position"), x, y);
    glUniform2f(glGetUniformLocation(shader, "scale"), width, height);


    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void glCubeDraw(mat4 model, int shader)
{

    if (shader == 0) {
        shader = g_defaultShader;
    }

    glBindVertexArray(g_debugCubeVAO);
    glUseProgram(shader);

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, model.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, viewMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, projectionMatrix.m);
    glUniform3f(glGetUniformLocation(shader, "color"), 1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_debugCubeEBO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void glCameraSetOrtho(float left, float right, float bottom, float top)
{
    projectionMatrix = mat4();
    projectionMatrix.m[0] = 2.0f / (right - left);
    projectionMatrix.m[5] = 2.0f / (top - bottom);
    projectionMatrix.m[10] = -1.0f;
    projectionMatrix.m[12] = -(right + left) / (right - left);
    projectionMatrix.m[13] = -(top + bottom) / (top - bottom);
    projectionMatrix.m[15] = 1.0f;
}

void glCameraSetPerspective(float fovY, float aspect, float nearZ, float farZ)
{
    projectionMatrix = mat4();
    float f = 1.0f / tanf(fovY * 0.5f);
    projectionMatrix.m[0] = f / aspect;
    projectionMatrix.m[5] = f;
    projectionMatrix.m[10] = (farZ + nearZ) / (nearZ - farZ);
    projectionMatrix.m[11] = -1.0f;
    projectionMatrix.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
}

void glCameraSetView(vec3 eye, vec3 center, vec3 up)
{
    vec3 f = (center - eye) * (1.0f / sqrtf((center - eye).dot(center - eye)));
    vec3 s = f.cross(up) * (1.0f / sqrtf(f.cross(up).dot(f.cross(up))));
    vec3 u = s.cross(f);
    
    viewMatrix = mat4::identity();
    viewMatrix.m[0] = s.x;
    viewMatrix.m[4] = s.y;
    viewMatrix.m[8] = s.z;
    viewMatrix.m[1] = u.x;
    viewMatrix.m[5] = u.y;
    viewMatrix.m[9] = u.z;
    viewMatrix.m[2] = -f.x;
    viewMatrix.m[6] = -f.y;
    viewMatrix.m[10] = -f.z;
    viewMatrix.m[12] = -s.dot(eye);
    viewMatrix.m[13] = -u.dot(eye);
    viewMatrix.m[14] = f.dot(eye);
}