#include <lilray.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

using namespace lilray;

#define DEG_TO_RAD (3.14159265359f / 180.f)
#define RAD_TO_DEG (180.f / 3.14159265359f)

static inline float signum(float v) {
    return v < 0 ? -1.0f : 1.0f;
}

static inline uint32_t darken(uint32_t color, uint8_t lightness) {
    uint64_t mask = 0x0000FF0000FF00FF;
    uint64_t expand = (((uint64_t) color) << 32) | color;
    uint64_t x = (((expand & mask) * lightness) >> 8) & mask;
    return (uint32_t) ((x >> 32) | x) | (color & 0xFF000000);
}

static inline float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static int spriteCompare(const void *a, const void *b) {
    return int(signum((*(Sprite **) b)->distance - (*(Sprite **) a)->distance));
}

Image::Image(const char *file) {
    int w, h, n;
    pixels = (uint32_t *) stbi_load(file, &w, &h, &n, 4);
    width = w; height = h;
    argb_to_rgba(pixels, pixels, width * height);
}

Image::Image(int32_t width, int32_t height, const uint32_t *pixels) : width(width), height(height) {
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
                                     uint8_t lightness) {
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

    uint32_t *texturePixels = texture.pixels + textureX;
    int32_t textureWidth = texture.width;
    uint32_t *framePixels = pixels + x + yStart * width;
    int32_t frameWidth = width;
    for (int i = 0, n = yEnd - yStart + 1; i < n; i++) {
        uint32_t texelColor = texturePixels[(uint32_t(textureY) * textureWidth)];
        *framePixels = darken(texelColor, lightness);
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

void Camera::move(Map &map, float distance) {
    float rayDirX = signum(distance) * cosf(angle * DEG_TO_RAD);
    float rayDirY = signum(distance) * sinf(angle * DEG_TO_RAD);
    int32_t cellX = int32_t(x + rayDirX * 0.1f), cellY = int32_t(y + rayDirY * 0.1f);
    if (map.getCell(cellX, int32_t(y)) <= 0) x += rayDirX * fabs(fmin(0.1f, distance));
    if (map.getCell(int32_t(x), cellY) <= 0) y += rayDirY * fabs(fmin(0.1f, distance));
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

Renderer::Renderer(int32_t width, int32_t height, Image **wallTextures, Image *floorTexture, Image *ceilingTexture)
        : frame(width, height), zbuffer(new float[width]), wallTextures(wallTextures), floorTexture(floorTexture),
          ceilingTexture(ceilingTexture) {
}

void Renderer::render(Camera &camera, Map &map, Sprite **sprites, int32_t numSprites, float lightDistance) {
    float frameHalfWidth = float(frame.width) / 2.0f;
    float frameHalfHeight = float(frame.height) / 2.0f;
    float maxDistance = sqrtf(float(map.width * map.width) + float(map.height * map.height));
    float camDirX = cosf(camera.angle * DEG_TO_RAD), camDirY = sinf(camera.angle * DEG_TO_RAD);
    float camRightX = -camDirY, camRightY = camDirX;
    float projectionPlaneWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);

    if (floorTexture && ceilingTexture) {
        float rayDirXLeft = camDirX + -projectionPlaneWidth * camRightX;
        float rayDirYLeft = camDirY + -projectionPlaneWidth * camRightY;
        float rayDirXRight = camDirX + projectionPlaneWidth * camRightX;
        float rayDirYRight = camDirY + projectionPlaneWidth * camRightY;
        float posZ = frameHalfHeight;
        float floorScaleX = (rayDirXRight - rayDirXLeft) / float(frame.width);
        float floorScaleY = (rayDirYRight - rayDirYLeft) / float(frame.width);
        int32_t floorWidth = floorTexture->width;
        int32_t floorHeigh = floorTexture->height;
        int32_t ceilingWidth = ceilingTexture->width;
        int32_t ceilingHeight = ceilingTexture->height;
        uint32_t *srcFloor = floorTexture->pixels;
        uint32_t *srcCeiling = ceilingTexture->pixels;

        for (int32_t y = frameHalfHeight + 1, n = frame.height; y < n; y++) {
            int32_t p = y - frameHalfHeight;
            float rowDistance = posZ / p;
            float floorStepX = rowDistance * floorScaleX;
            float floorStepY = rowDistance * floorScaleY;
            float floorX = camera.x + rowDistance * rayDirXLeft;
            float floorY = camera.y + rowDistance * rayDirYLeft;
            uint8_t lightness = uint8_t((1 - fmin(rowDistance, lightDistance) / lightDistance) * 255);
            uint32_t *dstFloor = frame.pixels + y * frame.width;
            uint32_t *dstCeiling = frame.pixels + (frame.height - 1 - y) * frame.width;
            for (int32_t x = 0, nn = frame.width; x < nn; x++, dstFloor++, dstCeiling++) {
                float distX = floorX - float(int32_t(floorX)), distY = floorY - float(int32_t(floorY));
                int32_t floorTx = int32_t(floorWidth * distX), floorTy = int32_t(floorHeigh * distY);
                int32_t ceilingTx = int32_t(ceilingWidth * distX), ceilingTy = int32_t(ceilingHeight * distY);
                *dstCeiling = darken(srcCeiling[ceilingTx + ceilingWidth * ceilingTy], lightness);
                *dstFloor = darken(srcFloor[floorTx + floorWidth * floorTy], lightness);
                floorX += floorStepX; floorY += floorStepY;
            }
        }
    }

    for (int32_t x = 0; x < frame.width; x++) {
        float rayX = camera.x, rayY = camera.y;
        float offset = ((float(x) * 2.0f / (float(frame.width) - 1.0f)) - 1.0f) * projectionPlaneWidth;
        float rayDirX = camDirX + offset * camRightX, rayDirY = camDirY + offset * camRightY;
        float rayDirLen = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);
        rayDirX /= rayDirLen, rayDirY /= rayDirLen;

        float distance, hitX, hitY;
        int32_t cell = map.raycast(rayX, rayY, rayDirX, rayDirY, maxDistance, hitX, hitY, distance);
        if (cell == 0) continue;
        distance = distance * (rayDirX * camDirX + rayDirY * camDirY);
        float cellHeight = frameHalfHeight / distance;
        Image *texture = wallTextures[cell - 1];
        int32_t textureX = int32_t((hitX + hitY) * float(texture->width)) % texture->width;
        uint32_t modulate = uint32_t((1 - fmin(distance, lightDistance) / lightDistance) * 255);
        frame.drawVerticalTextureSlice(x, int32_t(frameHalfHeight - cellHeight),
                                       int32_t(frameHalfHeight + cellHeight),
                                       *texture, textureX, modulate);
        zbuffer[x] = distance;
    }

    for (int i = 0; i < numSprites; i++) {
        Sprite *sprite = sprites[i];
        sprite->distance = distance(sprite->x, sprite->y, camera.x, camera.y);
    }
    qsort(sprites, numSprites, sizeof(Sprite *), &spriteCompare);
    for (int i = 0; i < numSprites; i++) {
        Sprite *sprite = sprites[i];
        float viewDirX = sprite->x - camera.x;
        float viewDirY = sprite->y - camera.y;
        if (viewDirX * camDirX + viewDirY * camDirY < 0)
            continue;
        float viewAngle = atan2f(viewDirY, viewDirX) * RAD_TO_DEG - camera.angle;
        float distance = sprite->distance * cosf(viewAngle * DEG_TO_RAD);
        float unitViewHeight = frameHalfHeight / distance;
        int32_t x = int32_t(tan(viewAngle * DEG_TO_RAD) / projectionPlaneWidth * frameHalfWidth + frameHalfWidth);
        int32_t yEnd = int32_t(frameHalfHeight + unitViewHeight);
        int32_t yStart = yEnd - int32_t(unitViewHeight * sprite->height * 2);

        if (x > 0 && x < frame.width) {
            if (zbuffer[x] < distance) continue;
            frame.drawVerticalLine(x, yStart, yEnd, 0xffff0000);
        }
    }
}
