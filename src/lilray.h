#ifndef LILRAY_H
#define LILRAY_H

#include <cstdint>

namespace lilray {
    struct Frame {
        int32_t width, height;
        uint32_t *pixels;

        Frame(int32_t width, int32_t height);

        ~Frame();

        void clear(uint32_t clearColor);

        void drawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, uint32_t color);
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

    void render(Frame &frame, Camera &camera, Map &map);
}

#endif