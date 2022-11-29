#include <lilray.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

using namespace lilray;

#define DEG_TO_RAD (3.14159265359f / 180.f)
#define RAD_TO_DEG (180.f / 3.14159265359f)
#define PIXEL_FP_BITS 6
#define PIXEL_FP_ONE (1 << 6)
#define TEXEL_FP_BITS 16
#define FLOOR_FP_BITS 13

static inline float signum(float v) {
    return v < 0 ? -1.0f : 1.0f;
}

static inline uint32_t darken(uint32_t color, uint8_t lightness) {
    uint64_t expand = (((uint64_t) color) << 32) | color;
    uint64_t x = (((expand & 0x0000FF0000FF00FF) * lightness) >> 8) & 0x0000FF0000FF00FF;
    return (uint32_t) ((x >> 32) | x) | (color & 0xFF000000);
}

static inline float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static int spriteCompare(const void *a, const void *b) {
    return int(signum((*(Sprite **) b)->distance - (*(Sprite **) a)->distance));
}


static inline int32_t floatToFixed(float v, int32_t bits) {
    return int32_t(v * (1 << bits));
}

static inline int32_t fixedToInt(int32_t v, int32_t bits) {
    return v >> bits;
}

static inline int32_t fixedRound(int32_t v, int32_t bits) {
    int32_t fpOne = (1 << bits);
    int32_t subMask = fpOne - 1;
    return (v + subMask) & ~subMask;
}

static inline int32_t fixedFloor(int32_t v, int32_t bits) {
    return (v >> bits) << bits;
}

static inline int32_t fixedMultiply(int32_t a, int32_t b, int32_t bits) {
    return int32_t((int64_t(a) * int64_t(b)) >> bits);
}

Image::Image(const char *imageFile) {
    pixels = (uint32_t *) stbi_load(imageFile, &width, &height, nullptr, 4);
    reverseColorChannels();
}

Image::Image(uint8_t *imageBytes, int32_t numBytes) {
    pixels = (uint32_t *) stbi_load_from_memory(imageBytes, numBytes, &width, &height, nullptr, 4);
    reverseColorChannels();
}

Image::Image(int32_t width, int32_t height, const uint32_t *pixels) : width(width), height(height) {
    this->pixels = new uint32_t[width * height];
    if (pixels) memcpy(this->pixels, pixels, sizeof(uint32_t) * width * height);
}

Image::~Image() {
    delete pixels;
}

Image *Image::getRegion(int32_t x, int32_t y, int32_t w, int32_t h) {
    Image *region = new Image(w, h);
    for (int dy = 0; dy < h; y++, dy++, x -= w) {
        for (int dx = 0; dx < w; x++, dx++) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;
            region->pixels[dx + dy * h] = pixels[x + y * width];
        }
    }
    return region;
}

void Image::clear(uint32_t clearColor) {
    for (int i = 0, n = width * height; i < n; i++) {
        pixels[i] = clearColor;
    }
}

void Image::drawVerticalLine(int32_t x, int32_t ys, int32_t ye, uint32_t color) {
    if (x < 0 || x >= width) return;
    if (ye < ys) {
        int32_t tmp = ye;
        ye = ys;
        ys = tmp;
    }
    if (ye < 0 || ys >= height) return;
    if (ys < 0) ys = 0;
    if (ye >= height) ye = height - 1;
    int32_t frameWidth = width;
    uint32_t *dst = pixels + x + ys * width;
    for (int i = 0, n = ye - ys + 1; i < n; i++) {
        *dst = color;
        dst += frameWidth;
    }
}

void Image::drawVerticalImageSlice(Image &texture, int32_t x, int32_t ys, int32_t ye, int32_t tx,
                                   uint8_t lightness) {
    if (x < 0 || x >= width) return;
    if (tx < 0 || tx >= texture.width) return;
    if (ye < ys) {
        int32_t tmp = ye;
        ye = ys;
        ys = tmp;
    }
    if (ye < 0 || ys >= height) return;
    int32_t textureWidth = texture.width;
    int32_t frameWidth = width;
    float stepY = float(texture.height) / float(ye - ys + 1);
    float ty = ys < 0 ? float(-ys) * stepY : 0;
    if (ys < 0) ys = 0;
    if (ye >= height) ye = height - 1;
    uint32_t *src = texture.pixels + tx;
    uint32_t *dst = pixels + x + ys * width;
    for (int i = 0, n = ye - ys + 1; i < n; i++) {
        uint32_t color = src[(uint32_t(ty) * textureWidth)];
        *dst = darken(color, lightness);
        ty += stepY;
        dst += frameWidth;
    }
}

void Image::drawRectangle(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    // Calculate top/left and bottom/right corner
    int32_t dx = x;
    int32_t dy = y;
    int32_t dx2 = dx + w - 1;
    int32_t dy2 = dy + h - 1;

    // Clip
    if (dx < 0) dx = 0;
    if (dy < 0) dy = 0;
    if (dx2 >= width) dx2 = width - 1;
    if (dy2 >= height) dy2 = height - 1;

    // Draw
    uint32_t *dst = pixels + dx + dy * width;
    uint32_t dstPitch = width - (dx2 - dx) - 1;
    for (; dy <= dy2; dy++, dst += dstPitch) {
        for (int32_t rx = dx; rx <= dx2; rx++, dst++) {
            *dst = color;
        }
    }
}

void drawSprite(Image *frame, Image *sprite, float x, float y, float scaledWidth, float scaledHeight, uint8_t lightness,
                const float *zbuffer, float distance) {
    // Calculate sub pixel accurate screen coordinates of screen aligned sprite
    int32_t minX = floatToFixed(x, PIXEL_FP_BITS);
    int32_t minY = floatToFixed(y, PIXEL_FP_BITS);
    int32_t maxX = floatToFixed(x + scaledWidth, PIXEL_FP_BITS);
    int32_t maxY = floatToFixed(y + scaledHeight, PIXEL_FP_BITS);
    int32_t tx = 0, ty = 0;

    // Round top/left corner coordinate -> stabilizes sprite on screen
    // Comment to see effect.
    minX = fixedRound(minX, PIXEL_FP_BITS);
    minY = fixedRound(minY, PIXEL_FP_BITS);

    // Calculate texture x/y gradients based on rounded screen width and height
    // of rectangle -> stabilizes texture point sampling.
    int32_t w = fixedToInt(fixedRound(maxX - minX + 1, PIXEL_FP_BITS), PIXEL_FP_BITS);
    int32_t h = fixedToInt(fixedRound(maxY - minY + 1, PIXEL_FP_BITS), PIXEL_FP_BITS);
    int32_t txStep = floatToFixed(sprite->width / float(w), TEXEL_FP_BITS);
    int32_t tyStep = floatToFixed(sprite->height / float(h), TEXEL_FP_BITS);

    // Clip rectangle and texture coordinates
    if (minX < 0) {
        // This isn't accurate, would need to take into account minX fractional part
        tx = -fixedToInt(minX, PIXEL_FP_BITS) * txStep;
        minX = 0;
    }
    if (minY < 0) {
        // This isn't accurate, would need to take into account minY fractional part
        ty = -fixedToInt(minY, PIXEL_FP_BITS) * tyStep;
        minY = 0;
    }
    if (maxX >= floatToFixed(frame->width, PIXEL_FP_BITS)) maxX = floatToFixed(frame->width - 1, PIXEL_FP_BITS);
    if (maxY >= floatToFixed(frame->height, PIXEL_FP_BITS)) maxY = floatToFixed(frame->height - 1, PIXEL_FP_BITS);

    // Draw the clipped rectangle. Texel color of 0x00000000 -> transparent
    int32_t px, py, ptx, pty;
    for (py = minY, pty = ty; py <= maxY; py += PIXEL_FP_ONE, pty += tyStep) {
        int32_t y = fixedToInt(py, PIXEL_FP_BITS);
        int32_t v = fixedToInt(pty, TEXEL_FP_BITS);
        uint32_t *dst = frame->pixels + y * frame->width;
        uint32_t *src = sprite->pixels + v * sprite->width;
        for (px = minX, ptx = tx; px <= maxX; px += PIXEL_FP_ONE, ptx += txStep) {
            int32_t x = fixedToInt(px, PIXEL_FP_BITS);
            if (zbuffer[x] < distance) continue;
            int32_t u = fixedToInt(ptx, TEXEL_FP_BITS);
            uint32_t color = src[u];
            if (!color) continue;
            dst[x] = darken(color, lightness);
        }
    }
}

void Image::drawText(Font &font, int32_t x, int32_t y, uint32_t color, const char *fmt, ...) {
    char text[1024];
    va_list args;
    va_start (args, fmt);
    vsnprintf(text, 1024, fmt, args);
    va_end (args);

    int32_t lineX = x;
    int32_t lineY = y;
    char *textPtr = text;
    while (*textPtr) {
        char c = *textPtr;
        textPtr++;

        // Handle newline
        if (c == '\n') {
            lineX = x;
            lineY += font.charHeight;
            continue;
        }

        // Check if char is entirely outside of image
        if (lineX + font.charWidth < 0) continue;
        if (lineX >= width) continue;
        if (lineY + font.charHeight < 0) continue;
        if (lineY >= height) break; // Line is "below" screen, exit

        // Check if char is contained in font
        char sc = c;
        c -= ' ';
        if (c < 0) continue;
        if (c > font.charsX * font.charsY - 1) continue;

        // Calculate char offset in font image
        int32_t cx = (c % font.charsX);
        int32_t cy = (c - cx) / font.charsX;
        cx *= font.charWidth;
        cy *= font.charHeight;

        // Calculate destination rectangle
        int32_t dx = lineX;
        int32_t dy = lineY;
        int32_t dx2 = dx + font.charWidth - 1;
        int32_t dy2 = dy + font.charHeight - 1;

        // Clip destination rectangle and char offset
        if (dx < 0) {
            cx -= dx;
            dx = 0;
        }
        if (dy < 0) {
            cy -= dy;
            dy = 0;
        }
        if (dx2 >= width) dx2 = width - 1;
        if (dy2 >= height) dy2 = height - 1;

        // Draw
        uint8_t *src = font.pixels + cx + cy * font.width;
        uint32_t srcPitch = font.width - (dx2 - dx) - 1;
        uint32_t *dst = pixels + dx + dy * width;
        uint32_t dstPitch = width - (dx2 - dx) - 1;
        for (; dy <= dy2; dy++) {
            for (int32_t rx = dx; rx <= dx2; rx++, dst++, src++) {
                if (!*src) continue;
                *dst = color;
            }
            dst += dstPitch;
            src += srcPitch;
        }

        lineX += font.charWidth;
    }
}

void Image::reverseColorChannels() {
    int32_t numPixels = (width * height) << 2;
    uint8_t *src = (uint8_t *) pixels;
    uint8_t *dst = (uint8_t *) pixels;
    for (size_t i = 0; i < numPixels; i += 4) {
        uint8_t b = src[i], g = src[i + 1], r = src[i + 2], a = src[i + 3];
        dst[i] = r;
        dst[i + 1] = g;
        dst[i + 2] = b;
        dst[i + 3] = a;
    }
}

Font::Font(const char *imageFile, int32_t charWidth, int32_t charHeight) :
        charWidth(charWidth), charHeight(charHeight) {
    pixels = (uint8_t *) stbi_load(imageFile, &width, &height, nullptr, 1);
    charsX = width / charWidth;
    charsY = height / charHeight;
}

Font::Font(uint8_t *imageBytes, int32_t numBytes, int32_t charWidth, int32_t charHeight) :
        charWidth(charWidth), charHeight(charHeight) {
    pixels = (uint8_t *) stbi_load_from_memory(imageBytes, numBytes, &width, &height, nullptr, 1);
    charsX = width / charWidth;
    charsY = height / charHeight;
}

void Font::getBounds(int32_t &width, int32_t &height, const char *fmt, ...) {
    char text[1024];
    va_list args;
    va_start (args, fmt);
    vsnprintf(text, 256, fmt, args);
    va_end (args);

    int32_t maxWidth = 0;
    int32_t lineWidth = 0;
    int32_t maxHeight = charHeight;
    char *textPtr = text;
    while (*textPtr) {
        char c = *textPtr;
        textPtr++;

        // Handle newline
        if (c == '\n') {
            maxWidth = lineWidth > maxWidth ? lineWidth : maxWidth;
            lineWidth = 0;
            maxHeight += charHeight;
            continue;
        }

        // Check if char is contained in font
        char sc = c;
        c -= ' ';
        if (c < 0) continue;
        if (c > charsX * charsY - 1) continue;

        lineWidth += charWidth;
    }
    width = maxWidth > lineWidth ? maxWidth : lineWidth;
    height = width > 0 ? maxHeight : 0;
}

Map::Map(int32_t width, int32_t height, int32_t *cells) : width(width), height(height) {
    this->cells = new int32_t[width * height];
    memcpy(this->cells, cells, sizeof(int32_t) * width * height);
}

Map::~Map() {
    delete cells;
}

void Map::setCell(int32_t x, int32_t y, int32_t value) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    cells[x + y * width] = value;
}

int32_t Map::getCell(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    return cells[x + y * width];
}

int32_t Map::raycast(float rayX, float rayY, float rayDirX, float rayDirY, float maxDistance, float &hitX, float &hitY,
                     float &distance) {
    // DDA implementation moving along grid intersections.
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

void Camera::strafe(Map &map, float distance) {
    float rayDirX = signum(distance) * sinf(angle * DEG_TO_RAD);
    float rayDirY = signum(distance) * -cosf(angle * DEG_TO_RAD);
    int32_t cellX = int32_t(x + rayDirX * 0.1f), cellY = int32_t(y + rayDirY * 0.1f);
    if (map.getCell(cellX, int32_t(y)) <= 0) x += rayDirX * fabs(fmin(0.1f, distance));
    if (map.getCell(int32_t(x), cellY) <= 0) y += rayDirY * fabs(fmin(0.1f, distance));
}

void Camera::rotate(float degrees) {
    angle += degrees;
}

Renderer::Renderer(int32_t width, int32_t height, Image **wallTextures, int numWallTextures, Image *floorTexture,
                   Image *ceilingTexture)
        : frame(width, height), zbuffer(new float[width]), wallTextures(wallTextures), numWallTextures(numWallTextures),
          floorTexture(floorTexture), ceilingTexture(ceilingTexture), useFixedPoint(false), drawWalls(true),
          drawFloorAndCeiling(true), drawSprites(true) {
}

void renderFloorAndCeilingFixedPoint(Renderer &renderer, Camera &camera, float lightDistance) {
    Image &frame = renderer.frame;
    float frameHalfHeight = float(frame.height) * 0.5f;
    float camDirX = cosf(camera.angle * DEG_TO_RAD), camDirY = sinf(camera.angle * DEG_TO_RAD);
    float camRightX = -camDirY, camRightY = camDirX;
    float projectionPlaneWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);
    float rayDirXLeft = camDirX + -projectionPlaneWidth * camRightX;
    float rayDirYLeft = camDirY + -projectionPlaneWidth * camRightY;
    float rayDirXRight = camDirX + projectionPlaneWidth * camRightX;
    float rayDirYRight = camDirY + projectionPlaneWidth * camRightY;
    float posZ = frameHalfHeight;
    float scaleX = (rayDirXRight - rayDirXLeft) / float(renderer.frame.width);
    float scaleY = (rayDirYRight - rayDirYLeft) / float(renderer.frame.width);
    int32_t floorWidth = renderer.floorTexture->width;
    int32_t floorHeight = renderer.floorTexture->height;
    int32_t ceilingWidth = renderer.ceilingTexture->width;
    int32_t ceilingHeight = renderer.ceilingTexture->height;
    uint32_t *srcFloor = renderer.floorTexture->pixels;
    uint32_t *srcCeiling = renderer.ceilingTexture->pixels;
    uint32_t *dstFloor = frame.pixels + (frame.height - 1) * frame.width;
    uint32_t *dstCeiling = frame.pixels;
    int32_t frameWidth = frame.width;
    float floorScaleX = scaleX * floorWidth;
    float floorScaleY = scaleY * floorHeight;
    float ceilingScaleX = scaleX * ceilingWidth;
    float ceilingScaleY = scaleY * ceilingHeight;

    for (int32_t y = 0, n = frameHalfHeight; y < n; y++) {
        int32_t p = int32_t(-(y - frameHalfHeight));
        float rowDistance = posZ / p;
        float cx = (camera.x + rowDistance * rayDirXLeft);
        float cy = (camera.y + rowDistance * rayDirYLeft);
        uint32_t floorStepX = floatToFixed(rowDistance * floorScaleX, FLOOR_FP_BITS);
        uint32_t floorStepY = floatToFixed(rowDistance * floorScaleY, FLOOR_FP_BITS);
        uint32_t floorX = floatToFixed(cx * floorWidth, FLOOR_FP_BITS);
        uint32_t floorY = floatToFixed(cy * floorHeight, FLOOR_FP_BITS);
        uint32_t ceilingStepX = floatToFixed(rowDistance * ceilingScaleX, FLOOR_FP_BITS);
        uint32_t ceilingStepY = floatToFixed(rowDistance * ceilingScaleY, FLOOR_FP_BITS);
        uint32_t ceilingX = floatToFixed(cx * ceilingWidth, FLOOR_FP_BITS);
        uint32_t ceilingY = floatToFixed(cy * ceilingHeight, FLOOR_FP_BITS);

        uint8_t lightness = uint8_t((1 - fmin(rowDistance, lightDistance) / lightDistance) * 255);
        for (int32_t x = 0, nn = frameWidth; x < nn; x++, dstFloor++, dstCeiling++) {
            int32_t floorTx = fixedToInt(floorX, FLOOR_FP_BITS) & (floorWidth - 1);
            int32_t floorTy = fixedToInt(floorY, FLOOR_FP_BITS) & (floorHeight - 1);
            *dstFloor = darken(srcFloor[floorTx + floorWidth * floorTy], lightness);

            int32_t ceilingTx = fixedToInt(ceilingX, FLOOR_FP_BITS) & (ceilingWidth - 1);
            int32_t ceilingTy = fixedToInt(ceilingY, FLOOR_FP_BITS) & (ceilingHeight - 1);
            *dstCeiling = darken(srcCeiling[ceilingTx + ceilingWidth * ceilingTy], lightness);

            floorX += floorStepX;
            floorY += floorStepY;
            ceilingX += ceilingStepX;
            ceilingY += ceilingStepY;
        }
        dstFloor -= frameWidth << 1;
    }
}

void renderFloorAndCeiling(Renderer &renderer, Camera &camera, float lightDistance) {
    Image &frame = renderer.frame;
    float frameHalfHeight = float(frame.height) * 0.5f;
    float camDirX = cosf(camera.angle * DEG_TO_RAD), camDirY = sinf(camera.angle * DEG_TO_RAD);
    float camRightX = -camDirY, camRightY = camDirX;
    float projectionPlaneWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);
    float rayDirXLeft = camDirX + -projectionPlaneWidth * camRightX;
    float rayDirYLeft = camDirY + -projectionPlaneWidth * camRightY;
    float rayDirXRight = camDirX + projectionPlaneWidth * camRightX;
    float rayDirYRight = camDirY + projectionPlaneWidth * camRightY;
    float posZ = frameHalfHeight;
    float scaleX = (rayDirXRight - rayDirXLeft) / float(renderer.frame.width);
    float scaleY = (rayDirYRight - rayDirYLeft) / float(renderer.frame.width);
    int32_t floorWidth = renderer.floorTexture->width;
    int32_t floorHeight = renderer.floorTexture->height;
    int32_t ceilingWidth = renderer.ceilingTexture->width;
    int32_t ceilingHeight = renderer.ceilingTexture->height;
    uint32_t *srcFloor = renderer.floorTexture->pixels;
    uint32_t *srcCeiling = renderer.ceilingTexture->pixels;
    uint32_t *dstFloor = frame.pixels + (frame.height - 1) * frame.width;
    uint32_t *dstCeiling = frame.pixels;
    int32_t frameWidth = frame.width;
    float floorScaleX = scaleX * floorWidth;
    float floorScaleY = scaleY * floorHeight;
    float ceilingScaleX = scaleX * ceilingWidth;
    float ceilingScaleY = scaleY * ceilingHeight;

    for (int32_t y = 0, n = frameHalfHeight; y < n; y++) {
        int32_t p = int32_t(-(y - frameHalfHeight));
        float rowDistance = posZ / p;
        float cx = (camera.x + rowDistance * rayDirXLeft);
        float cy = (camera.y + rowDistance * rayDirYLeft);
        float floorStepX = rowDistance * floorScaleX;
        float floorStepY = rowDistance * floorScaleY;
        float floorX = cx * floorWidth;
        float floorY = cy * floorHeight;
        float ceilingStepX = rowDistance * ceilingScaleX;
        float ceilingStepY = rowDistance * ceilingScaleY;
        float ceilingX = cx * ceilingWidth;
        float ceilingY = cy * ceilingHeight;

        uint8_t lightness = uint8_t((1 - fmin(rowDistance, lightDistance) / lightDistance) * 255);
        for (int32_t x = 0, nn = frameWidth; x < nn; x++, dstFloor++, dstCeiling++) {
            int32_t floorTx = int32_t(floorX) & (floorWidth - 1);
            int32_t floorTy = int32_t(floorY) & (floorHeight - 1);
            *dstFloor = darken(srcFloor[floorTx + floorWidth * floorTy], lightness);

            int32_t ceilingTx = int32_t(ceilingX) & (ceilingWidth - 1);
            int32_t ceilingTy = int32_t(ceilingY) & (ceilingHeight - 1);
            *dstCeiling = darken(srcCeiling[ceilingTx + ceilingWidth * ceilingTy], lightness);

            floorX += floorStepX;
            floorY += floorStepY;
            ceilingX += ceilingStepX;
            ceilingY += ceilingStepY;
        }
        dstFloor -= frameWidth << 1;
    }
}

void Renderer::render(Camera &camera, Map &map, Sprite **sprites, int32_t numSprites, float lightDistance) {
    float frameHalfWidth = float(frame.width) / 2.0f;
    float frameHalfHeight = float(frame.height) / 2.0f;
    float maxDistance = sqrtf(float(map.width * map.width) + float(map.height * map.height));
    float camDirX = cosf(camera.angle * DEG_TO_RAD), camDirY = sinf(camera.angle * DEG_TO_RAD);
    float camRightX = -camDirY, camRightY = camDirX;
    float projectionPlaneWidth = tanf(camera.fieldOfView / 2 * DEG_TO_RAD);

    for (int i = 0; i < frame.width; i++) zbuffer[i] = INFINITY;

    if (drawFloorAndCeiling && floorTexture && ceilingTexture) {
        if (!useFixedPoint)
            renderFloorAndCeiling(*this, camera, lightDistance);
        else
            renderFloorAndCeilingFixedPoint(*this, camera, lightDistance);
    }

    if (drawWalls) {
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
            int32_t tx = int32_t((hitX + hitY) * float(texture->width)) % texture->width;
            uint32_t lightness = uint32_t((1 - fmin(distance, lightDistance) / lightDistance) * 255);
            frame.drawVerticalImageSlice(*texture, x, int32_t(frameHalfHeight - cellHeight),
                                         int32_t(frameHalfHeight + cellHeight),
                                         tx, lightness);
            zbuffer[x] = distance;
        }
    }

    if (drawSprites) {
        for (int i = 0; i < numSprites; i++) {
            Sprite *sprite = sprites[i];
            sprite->distance = distance(sprite->x, sprite->y, camera.x, camera.y);
        }
        qsort(sprites, numSprites, sizeof(Sprite *), &spriteCompare);
        for (int i = 0; i < numSprites; i++) {
            Sprite *sprite = sprites[i];
            float viewDirX = sprite->x - camera.x, viewDirY = sprite->y - camera.y;
            if (viewDirX * camDirX + viewDirY * camDirY < 0) continue;
            float viewAngle = atan2f(viewDirY, viewDirX) * RAD_TO_DEG - camera.angle;
            float distance = sprite->distance * cosf(viewAngle * DEG_TO_RAD);
            uint8_t lightness = uint8_t((1 - fmax(0.2, fmin(distance, lightDistance) / lightDistance)) * 255);
            float halfUnitHeight = frameHalfHeight / distance;
            float screenHeight = halfUnitHeight * 2 * sprite->height;
            float screenWidth = screenHeight * (float(sprite->image->width) / float(sprite->image->height));
            float xc = (tanf(viewAngle * DEG_TO_RAD) / projectionPlaneWidth * frameHalfWidth + frameHalfWidth);
            float x = xc - screenWidth / 2;
            float y = frameHalfHeight + halfUnitHeight - screenHeight;
            drawSprite(&frame, sprite->image, x, y, screenWidth, screenHeight, lightness, zbuffer, distance);
        }
    }
}
