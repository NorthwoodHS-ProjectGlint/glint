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

