#pragma once

void* glSetup();
bool glRunning();
void glAttach(void* ctx);
void glPresent();

int glGenerateShader(const char* vertexSrc, const char* fragmentSrc);

void glDebugText(const char* text);
void glDebugTextFmt(const char* format, ...);
void glDebugTextFmt(unsigned long color, unsigned long bg, const char* format, ...);

void glDebugText(unsigned long color, unsigned long bg,const char* text);

void* glGetContext();