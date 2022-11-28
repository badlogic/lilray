#ifndef LILRAY_H
#define LILRAY_H

#include <cstdint>

namespace lilray {
    struct Sprite;

    struct Image {
        int32_t width, height;
        uint32_t *pixels;

        explicit Image(const char *file);

        Image(uint8_t *bytes, int32_t num_bytes);

        Image(int32_t width, int32_t height, const uint32_t *pixels = nullptr);

        ~Image();

        Image *getRegion(int32_t x, int32_t y, int32_t w, int32_t h);

        void clear(uint32_t clearColor);

        void drawVerticalLine(int32_t x, int32_t ys, int32_t ye, uint32_t color);

        void drawVerticalImageSlice(int32_t x, int32_t ys, int32_t ye, Image &texture, int32_t tx,
                                    uint8_t lightness);

        void drawVerticalImageSliceAlpha(int32_t x, int32_t ys, int32_t ye, Image &texture, int32_t tx,
                                         uint8_t lightness);

        void drawImage(float x, float y, float scaledWidth, float scaledHeight, Image *image, uint8_t lightness,
                       float *zbuffer, float distance);

        void toRgba();
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

        void strafe(Map &map, float distance);

        void rotate(float degrees);
    };

    struct Sprite {
        float x, y, height;
        Image *image;
        float distance;

        Sprite(float x, float y, float height, Image *image) : x(x), y(y), height(height), image(image) {}
    };

    struct Renderer {
        Image frame;
        float *zbuffer;
        Image **wallTextures;
        int32_t numWallTextures;
        Image *floorTexture;
        Image *ceilingTexture;

        Renderer(int32_t width, int32_t height, Image *wallTextures[], int32_t numWallTextures,
                 Image *floorTexture = nullptr, Image *ceilingTexture = nullptr);

        void render(Camera &camera, Map &map, Sprite *sprites[], int32_t numSprites, float lightDistance);
    };
}

#endif