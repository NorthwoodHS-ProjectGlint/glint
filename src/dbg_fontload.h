#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma pack(push, 1)
struct BMFHeader { uint8_t type; uint32_t size; };
struct BMFChar {
    uint32_t id; uint16_t x, y, width, height;
    int16_t xoffset, yoffset, xadvance;
    uint8_t page, channel;
};
#pragma pack(pop)



std::map<uint32_t, BMFChar> LoadBMFontBinary(const uint8_t* fileData, size_t fileSize) {
    std::map<uint32_t, BMFChar> fontMap;
    const uint8_t* ptr = fileData;
    const uint8_t* end = fileData + fileSize;

    // Validate magic + version
    if (fileSize < 4 || ptr[0] != 'B' || ptr[1] != 'M' || ptr[2] != 'F' || ptr[3] != 3)
        return fontMap;
    ptr += 4;

    while (ptr + sizeof(BMFHeader) <= end) {
        const BMFHeader* head = reinterpret_cast<const BMFHeader*>(ptr);
        ptr += sizeof(BMFHeader);

        const uint8_t* blockEnd = ptr + head->size;
        if (blockEnd > end) break; // Corrupt block

        if (head->type == 4) {
            int count = head->size / sizeof(BMFChar);
            for (int i = 0; i < count; ++i) {
                const BMFChar* c = reinterpret_cast<const BMFChar*>(ptr);
                fontMap[c->id] = *c;
                ptr += sizeof(BMFChar);
            }
        } else {
            ptr += head->size;
        }
    }
    return fontMap;
}

#pragma pack(push, 1)
struct TGAHeader {
    uint8_t idLen, mapType, imgType;
    uint16_t mapStart, mapLen; uint8_t mapSize;
    uint16_t x, y, width, height;
    uint8_t bpp, desc;
};
#pragma pack(pop)

GLuint LoadTGAFromMemory(const uint8_t* fileData, size_t fileSize) {


    
    int width, height, channels_in_file;
    // Request 4 components (RGBA) for consistency, but you can use 0 to let it decide.
    int desired_channels = 3;

    unsigned char* image_data = stbi_load_from_memory(
        fileData,
        fileSize,
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