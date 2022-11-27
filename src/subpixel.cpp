#include "lilray.h"
#include <MiniFB.h>
#include <cmath>

using namespace lilray;

template<typename T>
T orient2d(T ax, T ay, T bx, T by, T cx, T cy) { return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax); }

template<typename T>
T min(T a, T b) { return a < b ? a : b; }

template<typename T>
T max(T a, T b) { return a > b ? a : b; }

template<typename T>
T min3(T a, T b, T c) { return min(min(a, b), c); }

template<typename T>
T max3(T a, T b, T c) { return max(max(a, b), c); }

void
drawTriangleInt(Image &frame, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
    // Compute triangle bounding box
    int32_t minX = min3(x0, x1, x2);
    int32_t minY = min3(y0, y1, y2);
    int32_t maxX = max3(x0, x1, x2);
    int32_t maxY = max3(y0, y1, y2);

    // Clip against screen bounds
    minX = max(minX, 0);
    minY = max(minY, 0);
    maxX = min(maxX, frame.width - 1);
    maxY = min(maxY, frame.height - 1);

    // Rasterize
    int32_t px, py;
    for (py = minY; py <= maxY; py++) {
        for (px = minX; px <= maxX; px++) {
            // Determine barycentric coordinates
            int32_t w0 = orient2d(x1, y1, x2, y2, px, py);
            int32_t w1 = orient2d(x2, y2, x0, y0, px, py);
            int32_t w2 = orient2d(x0, y0, x1, y1, px, py);

            // If p is on or inside all edges, render pixel.
            if ((w0 | w1 | w2) >= 0) {
                frame.pixels[px + py * frame.width] = color;
            }
        }
    }
}

int32_t toFixed(float v, int32_t bits) {
    return int32_t(v * (1 << bits));
}

void
drawTriangleSubPixel(Image &frame, float x0f, float y0f, float x1f, float y1f, float x2f, float y2f, uint32_t color, int32_t fpBits) {
    int32_t subStep = 1 << fpBits;
    int32_t subMask = subStep - 1;
    int32_t x0 = toFixed(x0f, fpBits);
    int32_t y0 = toFixed(y0f, fpBits);
    int32_t x1 = toFixed(x1f, fpBits);
    int32_t y1 = toFixed(y1f, fpBits);
    int32_t x2 = toFixed(x2f, fpBits);
    int32_t y2 = toFixed(y2f, fpBits);

    int32_t minX = min3(x0, x1, x2);
    int32_t minY = min3(y0, y1, y2);
    int32_t maxX = max3(x0, x1, x2);
    int32_t maxY = max3(y0, y1, y2);

    minX = max(minX, 0);
    minY = max(minY, 0);
    maxX = min(maxX, toFixed(frame.width - 1, fpBits));
    maxY = min(maxY, toFixed(frame.height - 1, fpBits));

    minX = (minX + subMask) & ~subMask;
    minY = (minY + subMask) & ~subMask;

    int32_t px, py;
    for (py = minY; py <= maxY; py += subStep) {
        for (px = minX; px <= maxX; px += subStep) {
            int32_t w0 = orient2d(x1, y1, x2, y2, px, py);
            int32_t w1 = orient2d(x2, y2, x0, y0, px, py);
            int32_t w2 = orient2d(x0, y0, x1, y1, px, py);

            if ((w0 | w1 | w2) >= 0) {
                frame.pixels[(px >> fpBits) + (py >> fpBits) * frame.width] = color;
            }
        }
    }
}

#define DEG_TO_RAD (3.14159265359f / 180.f)

struct Point {
    float x, y;

    Point(float x, float y) : x(x), y(y) {}

    void rotate(float degrees) {
        float radians = degrees * DEG_TO_RAD;
        float c = cosf(radians), s = sinf(radians);
        float nx = x * c - y * s;
        float ny = y * c + x * s;
        x = nx;
        y = ny;
    }

    void translate(float dx, float dy) {
        x += dx;
        y += dy;
    }
};

struct Triangle {
    Point p0, p1, p2;

    Triangle(float x0, float y0, float x1, float y1, float x2, float y2) : p0(x0, y0), p1(x1, y1), p2(x2, y2) {}

    void rotate(float degrees) {
        p0.rotate(degrees);
        p1.rotate(degrees);
        p2.rotate(degrees);
    }

    void translate(float dx, float dy) {
        p0.translate(dx, dy);
        p1.translate(dx, dy);
        p2.translate(dx, dy);
    }
};

int main(int argc, char **argv) {
    const int resX = 320, resY = 128;
    const int resScale = 4;
    Image frame(resX, resY);
    mfb_window *window = mfb_open_ex("lilray", resX * resScale, resY * resScale, WF_RESIZABLE);
    if (!window) return 0;
    mfb_timer *deltaTimer = mfb_timer_create();

    Triangle tri(-32, 32, 0, -32, 32, 8);
    do {
        float delta = mfb_timer_delta(deltaTimer);
        frame.clear(0x0);
        tri.rotate(delta * 5);
        tri.translate(40, resY / 2);
        drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y, tri.p2.x, tri.p2.y, 0xffff0000, 0);
        tri.translate(resX / 4, 0);
        drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y, tri.p2.x, tri.p2.y, 0xffff0000, 2);
        tri.translate(resX / 4, 0);
        drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y, tri.p2.x, tri.p2.y, 0xffff0000, 4);
        tri.translate(resX / 4, 0);
        drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y, tri.p2.x, tri.p2.y, 0xffff0000, 8);
        tri.translate(-resX / 4 * 3 - 40, -resY / 2);
        if (mfb_update_ex(window, frame.pixels, resX, resY) < 0) break;
    } while (true);
}