#ifndef LILRAY_H
#define LILRAY_H

#include <cstdint>

#define LILRAY_ARGB(r, g, b, a) (uint32_t)(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))

namespace lilray {
    struct Texture {
        int32_t width, height;
        uint32_t *pixels;

        Texture(const char* file);
        Texture(int32_t width, int32_t height, uint32_t *pixels);
        Texture(int32_t width, int32_t height, uint8_t *pixels, uint32_t *palette, int32_t numColors);
        ~Texture();

        uint32_t getTexel(int32_t x, int32_t y);
    };

    struct Frame {
        int32_t width, height;
        uint32_t *pixels;

        Frame(int32_t width, int32_t height);

        ~Frame();

        void clear(uint32_t clearColor);

        void drawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, uint32_t color);

        void drawVerticalTextureSlice(int32_t x, int32_t yStart, int32_t yEnd, Texture &texture, int32_t u);
    };

    struct Map {
        int32_t width, height;
        int32_t *cells;

        Map(int32_t width, int32_t height, int32_t *cells);

        ~Map();

        void setCell(int32_t x, int32_t y, int32_t value);

        int32_t getCell(int32_t x, int32_t y);
    };

    struct Camera {
        float x, y, angle, fieldOfView;

        Camera(float x, float y, float angle, float fieldOfView);

        void move(float distance);

        void rotate(float degrees);
    };

    void render(Frame &frame, Camera &camera, Map &map, Texture &texture);

    void argb_to_rgba(uint8_t *argb, uint8_t *rgba, int32_t numPixels);
}

#endif