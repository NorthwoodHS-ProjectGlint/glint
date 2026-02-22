#pragma once

#include "glint/types/math.h"

void* glSetup();
bool glRunning();
void glAttach(void* ctx);
void glPresent();
double glGetTime();
double glGetDeltaTime();

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
void glCubeDraw(mat4 model, int shader);

void glCameraSetOrtho(float left, float right, float bottom, float top);
void glCameraSetPerspective(float fovY, float aspect, float nearZ, float farZ);
void glCameraSetView(vec3 eye, vec3 center, vec3 up);
