#pragma once

void* glSetup();
bool glRunning();
void glAttach(void* ctx);
void glPresent();
float glGetTime();
float glGetDeltaTime();

int glGenerateShader(const char* vertexSrc, const char* fragmentSrc);
int glGenerateTexture(int width, int height, const unsigned char* data, int desiredChannels);
int glGenerateTexture(const unsigned char* data, int dataSize, int desiredChannels=3);
int glGenerateTexture(const char* filePath, int desiredChannels=3);

void glDebugText(const char* text);
void glDebugTextFmt(const char* format, ...);
void glDebugTextFmt(unsigned long color, unsigned long bg, const char* format, ...);

void glDebugText(unsigned long color, unsigned long bg,const char* text);

void* glGetContext();

void glQuadDraw(float x, float y, float width, float height, int shader);