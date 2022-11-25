#ifndef LILRAY_H
#define LILRAY_H

#include <cstdint>

namespace lilray {
    struct Image {
        int32_t width, height;
        uint32_t *pixels;

        explicit Image(const char *file);

        Image(int32_t width, int32_t height, const uint32_t *pixels = nullptr);

        ~Image();

        Image *getRegion(int32_t x, int32_t y, int32_t w, int32_t h);

        void clear(uint32_t clearColor);

        void drawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, uint32_t color);

        void drawVerticalTextureSlice(int32_t x, int32_t yStart, int32_t yEnd, Image &texture, int32_t textureX,
                                      uint32_t lightness);
    };

    struct Map {
        int32_t width, height;
        int32_t *cells;

        Map(int32_t width, int32_t height, int32_t *cells);

        ~Map();

        void setCell(int32_t x, int32_t y, int32_t value);

        int32_t getCell(int32_t x, int32_t y);

        int32_t
        raycast(float rayX, float rayY, float rayDirX, float rayDirY, float maxDistance, float &hitX, float &hitY,
                float &distance);
    };

    struct Camera {
        float x, y, angle, fieldOfView;

        Camera(float x, float y, float angle, float fieldOfView);

        void move(Map &map, float distance);

        void rotate(float degrees);
    };

    struct Sprite {
        float x, y, height;
        Image *image;
        float distance;

        Sprite(float x, float y, float height, Image *image): x(x), y(y), height(height), image(image) {}
    };

    void
    render(Image &frame, float zbuffer[], Camera &camera, Map &map, Sprite *sprites[], int32_t numSprites, Image *walls[], Image *floor, Image *ceiling, float lightDistance);

    void argb_to_rgba(uint32_t *argb, uint32_t *rgba, int32_t numPixels);
}

#endif