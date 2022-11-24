#include <lilray.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace lilray;

#define DEG_TO_RAD (3.14159265359f / 180.f)

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

void lilray::render(Frame &frame, Camera &camera, Map &map) {
    float rayAngle = camera.angle - camera.fieldOfView / 2;
    float rayAngleStep = camera.fieldOfView / frame.width;
    float frameHalfHeight = frame.height / 2;
    float maxDistance = sqrtf(map.width * map.width + map.height * map.height) * 10;

    for (int x = 0; x < frame.width; x++) {
        float rayX = camera.x;
        float rayY = camera.y;
        float rayDirX = cosf(rayAngle * DEG_TO_RAD);
        float rayDirY = sinf(rayAngle * DEG_TO_RAD);
        float rayStepX = sqrtf(1 + (rayDirY / rayDirX) * (rayDirY / rayDirX));
        float rayStepY = sqrtf(1 + (rayDirX / rayDirY) * (rayDirX / rayDirY));
        int mapX = (int) rayX, mapY = (int) rayY;
        float mapStepX = 0, mapStepY = 0;
        float rayLengthX = 0, rayLengthY = 0;

        if (rayDirX < 0) {
            mapStepX = -1;
            rayLengthX = (rayX - mapX) * rayStepX;
        } else {
            mapStepX = 1;
            rayLengthX = ((mapX + 1) - rayX) * rayStepX;
        }

        if (rayDirY < 0) {
            mapStepY = -1;
            rayLengthY = (rayY - mapY) * rayStepY;
        } else {
            mapStepY = 1;
            rayLengthY = ((mapY + 1) - rayY) * rayStepY;
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

        distance = distance * cos((rayAngle - camera.angle) * DEG_TO_RAD);
        int32_t cellHeight = frameHalfHeight / distance;
        frame.drawVerticalLine(x, frameHalfHeight - cellHeight, frameHalfHeight + cellHeight, 0xffff0000);

        rayAngle += rayAngleStep;
    }
}
