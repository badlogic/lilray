#include <lilray.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

using namespace lilray;

#define DEG_TO_RAD (3.14159265359f / 180.f)

struct argb_pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

static inline void _darken(const uint32_t *src, uint32_t *dst, uint32_t lightness) {
    argb_pixel *srcPixel = (argb_pixel *) src;
    argb_pixel *dstPixel = (argb_pixel *) dst;
    dstPixel->r = (srcPixel->r * lightness) >> 8;
    dstPixel->g = (srcPixel->g * lightness) >> 8;
    dstPixel->b = (srcPixel->b * lightness) >> 8;
}

Image::Image(const char *file) {
    int w, h, n;
    pixels = (uint32_t *) stbi_load(file, &w, &h, &n, 4);
    width = w;
    height = h;
    argb_to_rgba(pixels, pixels, width * height);
}

Image::Image(int32_t width, int32_t height, const uint32_t *pixels) : width(width),
                                                                      height(height) {
    this->pixels = (uint32_t *) malloc(sizeof(uint32_t) * width * height);
    if (pixels) memcpy(this->pixels, pixels, sizeof(uint32_t) * width * height);
}

Image::~Image() {
    free(pixels);
}

Image *Image::getRegion(int32_t x, int32_t y, int32_t w, int32_t h) {
    Image *region = new Image(w, h);
    for (int dy = 0; dy < h; y++, dy++) {
        for (int dx = 0; dx < w; x++, dx++) {
            if (x < 0 || x >= width) continue;
            if (y < 0 || y >= height) continue;
            region->pixels[dx + dy * h] = pixels[x + y * width];
        }
        x -= w;
    }
    return region;
}

void Image::clear(uint32_t clearColor) {
    for (int i = 0, n = width * height; i < n; i++) {
        pixels[i] = clearColor;
    }
}

void Image::drawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, uint32_t color) {
    if (yEnd < yStart) {
        int32_t tmp = yEnd;
        yEnd = yStart;
        yStart = tmp;
    }
    if (x < 0 || x >= width) return;
    if (yEnd < 0) return;
    if (yStart >= height) return;
    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;
    uint32_t *dst = pixels + x + yStart * width;
    for (int i = 0, n = yEnd - yStart + 1; i < n; i++) {
        *dst = color;
        dst += width;
    }
}

void Image::drawVerticalTextureSlice(int32_t x, int32_t yStart, int32_t yEnd, Image &texture, int32_t textureX,
                                     uint32_t lightness) {
    if (yEnd < yStart) {
        int32_t tmp = yEnd;
        yEnd = yStart;
        yStart = tmp;
    }
    if (x < 0 || x >= width) return;
    if (yEnd < 0) return;
    if (yStart >= height) return;
    if (textureX < 0 || textureX >= texture.width) return;

    float textureStepY = float(texture.height) / float(yEnd - yStart + 1);
    float textureY = yStart < 0 ? float(-yStart) * textureStepY : 0;

    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;

    const uint32_t *texturePixels = texture.pixels + textureX;
    const int32_t textureWidth = texture.width;
    uint32_t *framePixels = pixels + x + yStart * width;
    const int32_t frameWidth = width;
    for (int i = 0, n = yEnd - yStart + 1; i < n; i++) {
        _darken(texturePixels + (uint32_t(textureY) * textureWidth), framePixels, lightness);
        textureY += textureStepY;
        framePixels += frameWidth;
    }
}

Map::Map(int32_t width, int32_t height, int32_t *cells) : width(width), height(height) {
    this->cells = (int32_t *) malloc(sizeof(int32_t) * width * height);
    memcpy(this->cells, cells, sizeof(int32_t) * width * height);
}

Map::~Map() {
    free(cells);
}

void Map::setCell(int32_t x, int32_t y, int32_t value) {
    if (x < 0 || x >= width) return;
    if (y < 0 || y >= height) return;
    cells[x + y * width] = value;
}

int32_t Map::getCell(int x, int y) {
    if (x < 0 || x >= width) return 0;
    if (y < 0 || y >= height) return 0;
    return cells[x + y * width];
}

int32_t Map::raycast(float rayX, float rayY, float rayDirX, float rayDirY, float maxDistance, float &hitX, float &hitY,
                     float &distance) {
    float rayStepX = sqrtf(1 + (rayDirY / rayDirX) * (rayDirY / rayDirX));
    float rayStepY = sqrtf(1 + (rayDirX / rayDirY) * (rayDirX / rayDirY));
    int mapX = int(rayX), mapY = int(rayY), mapStepX, mapStepY;
    float rayLengthX, rayLengthY;

    if (rayDirX < 0) {
        mapStepX = -1;
        rayLengthX = (rayX - float(mapX)) * rayStepX;
    } else {
        mapStepX = 1;
        rayLengthX = (float(mapX + 1) - rayX) * rayStepX;
    }

    if (rayDirY < 0) {
        mapStepY = -1;
        rayLengthY = (rayY - float(mapY)) * rayStepY;
    } else {
        mapStepY = 1;
        rayLengthY = (float(mapY + 1) - rayY) * rayStepY;
    }

    int cell = 0;
    distance = 0;
    while (!cell && distance < maxDistance) {
        if (rayLengthX < rayLengthY) {
            mapX += mapStepX;
            distance = rayLengthX;
            rayLengthX += rayStepX;
        } else {
            mapY += mapStepY;
            distance = rayLengthY;
            rayLengthY += rayStepY;
        }
        cell = getCell(mapX, mapY);
    }
    if (cell == 0) return 0;

    hitX = rayX + rayDirX * distance;
    hitY = rayY + rayDirY * distance;
    return cell;
}

Camera::Camera(float x, float y, float angle, float fieldOfView) : x(x), y(y), angle(angle), fieldOfView(fieldOfView) {
}

float sign(float v) {
    return v < 0 ? -1.0f : 1.0f;
}

void Camera::move(Map &map, float distance) {
    float maxDistance = sqrtf(float(map.width * map.width) + float(map.height * map.height));
    float rayDirX = sign(distance) * cosf(angle * DEG_TO_RAD);
    float rayDirY = sign(distance) * sinf(angle * DEG_TO_RAD);
    if (map.getCell(x + rayDirX * 0.1f, y) <= 0) x += rayDirX * fabs(fmin(0.1f, distance));
    if (map.getCell(x, y + rayDirY * 0.1f) <= 0) y += rayDirY * fabs(fmin(0.1f, distance));
}

void Camera::rotate(float degrees) {
    angle += degrees;
}

void lilray::argb_to_rgba(uint32_t *argb, uint32_t *rgba, int32_t numPixels) {
    numPixels <<= 2;
    uint8_t *src = (uint8_t *) argb;
    uint8_t *dst = (uint8_t *) rgba;
    for (size_t i = 0; i < numPixels; i += 4) {
        uint8_t b = src[i], g = src[i + 1], r = src[i + 2], a = src[i + 3];
        dst[i] = r;
        dst[i + 1] = g;
        dst[i + 2] = b;
        dst[i + 3] = a;
    }
}

void lilray::render(Image &frame, Camera &camera, Map &map, Image *walls[], Image *floor, Image *ceiling,
                    float lightDistance) {
    const float frameHalfHeight = float(frame.height) / 2.0f;
    const float maxDistance = sqrtf(float(map.width * map.width) + float(map.height * map.height));
    const float camDirX = cosf(camera.angle * DEG_TO_RAD);
    const float camDirY = sinf(camera.angle * DEG_TO_RAD);
    const float rightX = -camDirY;
    const float rightY = camDirX;
    const float halfWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);

    if (floor && ceiling) {
        const float rayDirXLeft = camDirX + -halfWidth * rightX;
        const float rayDirYLeft = camDirY + -halfWidth * rightY;
        const float rayDirXRight = camDirX + halfWidth * rightX;
        const float rayDirYRight = camDirY + halfWidth * rightY;
        const float posZ = 0.5f * frame.height;
        const float floorScaleX = (rayDirXRight - rayDirXLeft) / frame.width;
        const float floorScaleY = (rayDirYRight - rayDirYLeft) / frame.width;
        const int32_t floorWidth = floor->width;
        const int32_t floorWidthMinusOne = floorWidth - 1;
        const int32_t floorHeight = floor->height;
        const int32_t floorHeightMinusOne = floor->height - 1;
        const int32_t ceilingWidth = ceiling->width;
        const int32_t ceilingWidthMinusOne = ceiling->width - 1;
        const int32_t ceilingHeight = ceiling->height;
        const int32_t ceilingHeightMinusOne = ceiling->height - 1;

        for (int y = frame.height / 2 + 1; y < frame.height; y++) {
            int32_t p = y - (frame.height >> 1);
            float rowDistance = posZ / p;
            float floorStepX = rowDistance * floorScaleX;
            float floorStepY = rowDistance * floorScaleY;
            float floorX = camera.x + rowDistance * rayDirXLeft;
            float floorY = camera.y + rowDistance * rayDirYLeft;
            uint32_t lightness = uint32_t((1 - fmin(rowDistance, lightDistance) / lightDistance) * 255);
            uint32_t *dstFloor = frame.pixels + y * frame.width;
            uint32_t *dstCeiling = frame.pixels + (frame.height - 1 - y) * frame.width;
            for (int x = 0; x < frame.width; x++, dstFloor++, dstCeiling++) {
                int32_t cellX = int32_t(floorX);
                int32_t cellY = int32_t(floorY);
                float distX = floorX - cellX;
                float distY = floorY - cellY;
                int32_t floorTx = int32_t(floorWidth * distX) & floorWidthMinusOne;
                int32_t floorTy = int32_t(floorHeight * distY) & floorHeightMinusOne;
                int32_t ceilingTx = int32_t(ceilingWidth * distX) & ceilingWidthMinusOne;
                int32_t ceilingTy = int32_t(ceilingHeight * distY) & ceilingHeightMinusOne;

                floorX += floorStepX;
                floorY += floorStepY;

                _darken(&ceiling->pixels[ceilingTx + floorWidth * ceilingTy], dstCeiling, lightness);
                _darken(&floor->pixels[floorTx + floorWidth * floorTy], dstFloor, lightness);
            }
        }
    }

    for (int x = 0; x < frame.width; x++) {
        float rayX = camera.x;
        float rayY = camera.y;
        float offset = ((float(x) * 2.0f / (float(frame.width) - 1.0f)) - 1.0f) * halfWidth;
        float rayDirX = camDirX + offset * rightX;
        float rayDirY = camDirY + offset * rightY;
        float rayDirLen = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);
        rayDirX /= rayDirLen;
        rayDirY /= rayDirLen;

        float distance, hitX, hitY;
        int32_t cell = map.raycast(rayX, rayY, rayDirX, rayDirY, maxDistance, hitX, hitY, distance);
        if (cell == 0) continue;
        distance = distance * (rayDirX * camDirX + rayDirY * camDirY);
        float cellHeight = frameHalfHeight / distance;
        Image *texture = walls[cell - 1];
        int32_t textureX = int32_t((hitX + hitY) * float(texture->width)) % texture->width;
        uint32_t modulate = uint32_t((1 - fmin(distance, lightDistance) / lightDistance) * 255);
        frame.drawVerticalTextureSlice(x, int32_t(frameHalfHeight - cellHeight),
                                       int32_t(frameHalfHeight + cellHeight),
                                       *texture, textureX, modulate);
    }
}
