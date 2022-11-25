#include <lilray.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace lilray;

#define DEG_TO_RAD (3.14159265359f / 180.f)

Texture::Texture(const char* file) {
    int n;
    pixels = (uint32_t*)stbi_load(file, &width, &height, &n, 4);
    argb_to_rgba((uint8_t*)pixels, (uint8_t*)pixels, width * height);
}

Texture::Texture(int32_t width, int32_t height, uint32_t *pixels) : width(width),
                                                                    height(height) {
    this->pixels = (uint32_t *) malloc(sizeof(uint32_t) * width * height);
    memcpy(this->pixels, pixels, sizeof(uint32_t) * width * height);
}

Texture::Texture(int32_t width, int32_t height, uint8_t *pixels, uint32_t *palette, int32_t numColors) : width(width),
                                                                                                         height(height) {
    this->pixels = (uint32_t *) malloc(sizeof(uint32_t) * width * height);
    for (int i = 0, n = width * height; i < n; i++) {
        this->pixels[i] = palette[pixels[i]];
    }
}

Texture::~Texture() {
    delete pixels;
}

uint32_t Texture::getTexel(int32_t x, int32_t y) {
    if (x < 0 || x >= width) return 0;
    if (y < 0 || y >= height) return 0;
    return pixels[x + y * width];
}

Frame::Frame(int32_t width, int32_t height) : width(width), height(height) {
    pixels = (uint32_t *) malloc(sizeof(uint32_t) * width * height);
}

Frame::~Frame() {
    free(pixels);
}

void Frame::clear(uint32_t clearColor) {
    for (int i = 0, n = width * height; i < n; i++) {
        pixels[i] = clearColor;
    }
}

void Frame::drawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, uint32_t color) {
    if (yEnd < yStart) {
        float tmp = yEnd;
        yEnd = yStart;
        yStart = tmp;
    }
    if (x < 0 || x >= width) return;
    if (yEnd < 0) return;
    if (yStart >= height) return;
    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;
    int numPixels = yEnd - yStart + 1;
    int offset = x + yStart * width;
    for (int i = 0; i < numPixels; i++) {
        pixels[offset] = color;
        offset += width;
    }
}

void Frame::drawVerticalTextureSlice(int32_t x, int32_t yStart, int32_t yEnd, Texture &texture, int32_t u) {
    if (yEnd < yStart) {
        float tmp = yEnd;
        yEnd = yStart;
        yStart = tmp;
    }
    if (x < 0 || x >= width) return;
    if (yEnd < 0) return;
    if (yStart >= height) return;

    int textureX = u;
    float textureStepY = float(texture.height) / float(yEnd - yStart + 1);
    float textureY = yStart < 0 ? -yStart * textureStepY : 0;

    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;
    if (u < 0) u = 0;
    if (u >= texture.width) u = texture.width;

    int numPixels = yEnd - yStart + 1;
    int offset = x + yStart * width;
    for (int i = 0; i < numPixels; i++) {
        pixels[offset] = texture.getTexel(textureX, int32_t(textureY));
        textureY += textureStepY;
        offset += width;
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

Camera::Camera(float x, float y, float angle, float fieldOfView) : x(x), y(y), angle(angle), fieldOfView(fieldOfView) {
}

void Camera::move(float distance) {
    x += distance * cosf(angle * DEG_TO_RAD);
    y += distance * sinf(angle * DEG_TO_RAD);
}

void Camera::rotate(float degrees) {
    angle += degrees;
}

void lilray::render(Frame &frame, Camera &camera, Map &map, Texture &texture) {
    float rayAngle = camera.angle - camera.fieldOfView / 2;
    float rayAngleStep = camera.fieldOfView / frame.width;
    int32_t frameHalfHeight = frame.height / 2;
    float maxDistance = sqrtf(map.width * map.width + map.height * map.height) * 10;
    float camDirX = cosf(camera.angle * DEG_TO_RAD);
    float camDirY = sinf(camera.angle * DEG_TO_RAD);
    float rightX = -camDirY;
    float rightY = camDirX;
    float halfWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);

    for (int x = 0; x < frame.width; x++) {
        float rayX = camera.x;
        float rayY = camera.y;
        float offset = ((x * 2.0 / (frame.width - 1.0)) - 1.0) * halfWidth;
        float rayDirX = camDirX + offset * rightX;
        float rayDirY = camDirY + offset * rightY;
        float rayDirLen = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);
        rayDirX /= rayDirLen;
        rayDirY /= rayDirLen;
        float rayStepX = sqrtf(1 + (rayDirY / rayDirX) * (rayDirY / rayDirX));
        float rayStepY = sqrtf(1 + (rayDirX / rayDirY) * (rayDirX / rayDirY));
        int mapX = int(rayX), mapY = int(rayY);
        int mapStepX = 0, mapStepY = 0;
        float rayLengthX = 0, rayLengthY = 0;

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
        float distance = 0;
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
            cell = map.getCell(mapX, mapY);
        }
        if (cell == 0) {
            rayAngle += rayAngleStep;
            continue;
        }

        rayX += rayDirX * distance;
        rayY += rayDirY * distance;
        distance = distance * cosf((rayAngle - camera.angle) * DEG_TO_RAD);
        int32_t u = int32_t((rayX + rayY) * texture.width) % texture.width;
        int32_t cellHeight = frameHalfHeight / distance;
        frame.drawVerticalTextureSlice(x, frameHalfHeight - cellHeight, frameHalfHeight + cellHeight, texture, u);

        rayAngle += rayAngleStep;
    }
}

void lilray::argb_to_rgba(uint8_t *argb, uint8_t *rgba, int32_t numPixels) {
    numPixels <<= 2;
    for (size_t i = 0; i < numPixels; i += 4) {
        uint8_t b = argb[i];
        uint8_t g = argb[i + 1];
        uint8_t r = argb[i + 2];
        uint8_t a = argb[i + 3];
        rgba[i] = r;
        rgba[i + 1] = g;
        rgba[i + 2] = b;
        rgba[i + 3] = a;
    }
}
