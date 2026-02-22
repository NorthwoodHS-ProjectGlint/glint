#pragma once
#include <cmath>

// vectors
struct vec2 {
    float x, y;

    vec2() : x(0), y(0) {}
    vec2(float x, float y) : x(x), y(y) {}

    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(float s) const { return vec2(x * s, y * s); }
    vec2 operator/(float s) const { return vec2(x / s, y / s); }
    vec2& operator+=(const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    vec2& operator-=(const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    vec2& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }
    vec2& operator/=(float s) {
        x /= s;
        y /= s;
        return *this;
    }
    float dot(const vec2& v) const { return x * v.x + y * v.y; }
};

struct vec3 {
    float x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    vec3 operator+(const vec3& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
    vec3 operator-(const vec3& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
    vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(float s) const { return vec3(x / s, y / s, z / s); }
    vec3& operator+=(const vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator-=(const vec3& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    vec3& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    vec3& operator/=(float s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    float dot(const vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    vec3 cross(const vec3& v) const { return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
};

struct vec4 {
    float x, y, z, w;

    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    vec4 operator+(const vec4& v) const { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    vec4 operator-(const vec4& v) const { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    vec4 operator*(float s) const { return vec4(x * s, y * s, z * s, w * s); }
    vec4 operator/(float s) const { return vec4(x / s, y / s, z / s, w / s); }
    vec4& operator+=(const vec4& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }
    vec4& operator-=(const vec4& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }
    vec4& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    vec4& operator/=(float s) {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }
    float dot(const vec4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
};

// matrices
struct mat4 {
    float m[16];

    mat4() {
        for (int i = 0; i < 16; i++) {
            m[i] = 0;
        }
    }

    static mat4 identity() {
        mat4 result;
        result.m[0] = 1;
        result.m[5] = 1;
        result.m[10] = 1;
        result.m[15] = 1;
        return result;
    }

    // matrix transformations
    static mat4 translation(float x, float y, float z) {
        mat4 result = identity();
        result.m[3] = x;
        result.m[7] = y;
        result.m[11] = z;
        return result;
    }

    static mat4 scale(float x, float y, float z) {
        mat4 result = identity();
        result.m[0] = x;
        result.m[5] = y;
        result.m[10] = z;
        return result;
    }

    static mat4 rotationX(float angle) {
        mat4 result = identity();
        float c = cos(angle), s = sin(angle);
        result.m[5] = c;
        result.m[6] = s;
        result.m[9] = -s;
        result.m[10] = c;
        return result;
    }

    static mat4 rotationY(float angle) {
        mat4 result = identity();
        float c = cos(angle), s = sin(angle);
        result.m[0] = c;
        result.m[2] = -s;
        result.m[8] = s;
        result.m[10] = c;
        return result;
    }

    static mat4 rotationZ(float angle) {
        mat4 result = identity();
        float c = cos(angle), s = sin(angle);
        result.m[0] = c;
        result.m[1] = s;
        result.m[4] = -s;
        result.m[5] = c;
        return result;
    }

    mat4 operator*(const mat4& other) const {
        mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i * 4 + j] = m[i * 4] * other.m[j] + m[i * 4 + 1] * other.m[4 + j] +
                                      m[i * 4 + 2] * other.m[8 + j] + m[i * 4 + 3] * other.m[12 + j];
            }
        }
        return result;
    }

    vec4 operator*(const vec4& v) const {
        return vec4(
            m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
            m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
            m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
            m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
        );
    }

    mat4 operator+(const mat4& other) const {
        mat4 result;
        for (int i = 0; i < 16; i++) result.m[i] = m[i] + other.m[i];
        return result;
    }

    mat4& operator+=(const mat4& other) {
        for (int i = 0; i < 16; i++) m[i] += other.m[i];
        return *this;
    }

    mat4& operator*=(const mat4& other) {
        *this = *this * other;
        return *this;
    }

    mat4 operator*(float s) const {
        mat4 result;
        for (int i = 0; i < 16; i++) result.m[i] = m[i] * s;
        return result;
    }

    mat4& operator*=(float s) {
        for (int i = 0; i < 16; i++) m[i] *= s;
        return *this;
    }
};