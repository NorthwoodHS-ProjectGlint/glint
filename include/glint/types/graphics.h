#pragma once

struct ColorRGBA {
    float r, g, b, a;

    ColorRGBA() : r(0), g(0), b(0), a(1) {}
    ColorRGBA(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

};

struct ColorRGB {
    float r, g, b;

    ColorRGB() : r(0), g(0), b(0) {}
    ColorRGB(float r, float g, float b) : r(r), g(g), b(b) {}

};
