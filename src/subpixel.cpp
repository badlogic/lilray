#include "lilray.h"
#include <math.h>
#define _Bool bool
#include <MiniFB.h>

using namespace lilray;

static inline int32_t floatToFixed(float v, int32_t bits) {
	return int32_t(v * (1 << bits));
}

static inline int32_t fixedToInt(int32_t v, int32_t bits) { return v >> bits; }

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

template<typename T>
T orient2d(T ax, T ay, T bx, T by, T cx, T cy) {
	return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

template<typename T>
T min(T a, T b) { return a < b ? a : b; }

template<typename T>
T max(T a, T b) { return a > b ? a : b; }

template<typename T>
T min3(T a, T b, T c) { return min(min(a, b), c); }

template<typename T>
T max3(T a, T b, T c) { return max(max(a, b), c); }

void drawTriangleInt(Image &frame, int32_t x0, int32_t y0, int32_t x1,
					 int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
	// Compute triangle bounding box
	int32_t minX = min3(x0, x1, x2);
	int32_t minY = min3(y0, y1, y2);
	int32_t maxX = max3(x0, x1, x2);
	int32_t maxY = max3(y0, y1, y2);

	// Clip against screen bounds
	minX = max(minX, (int32_t) 0);
	minY = max(minY, (int32_t) 0);
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

void drawTriangleSubPixel(Image &frame, float x0f, float y0f, float x1f,
						  float y1f, float x2f, float y2f, uint32_t color,
						  int32_t fpBits) {
	int32_t subStep = 1 << fpBits;
	int32_t subMask = subStep - 1;
	int32_t x0 = floatToFixed(x0f, fpBits);
	int32_t y0 = floatToFixed(y0f, fpBits);
	int32_t x1 = floatToFixed(x1f, fpBits);
	int32_t y1 = floatToFixed(y1f, fpBits);
	int32_t x2 = floatToFixed(x2f, fpBits);
	int32_t y2 = floatToFixed(y2f, fpBits);

	int32_t minX = min3(x0, x1, x2);
	int32_t minY = min3(y0, y1, y2);
	int32_t maxX = max3(x0, x1, x2);
	int32_t maxY = max3(y0, y1, y2);

	minX = max(minX, (int32_t) 0);
	minY = max(minY, (int32_t) 0);
	maxX = min(maxX, floatToFixed(frame.width - 1, fpBits));
	maxY = min(maxY, floatToFixed(frame.height - 1, fpBits));

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

	Triangle(float x0, float y0, float x1, float y1, float x2, float y2)
		: p0(x0, y0), p1(x1, y1), p2(x2, y2) {}

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

void subPixelTriangles() {
	const int resX = 320, resY = 128;
	const int resScale = 4;
	Image frame(resX, resY);
	mfb_window *window = mfb_open_ex("sub-pixel triangles", resX * resScale,
									 resY * resScale, WF_RESIZABLE);
	if (!window)
		return;
	mfb_timer *deltaTimer = mfb_timer_create();

	Triangle tri(-32, 32, 0, -32, 32, 8);
	do {
		float delta = mfb_timer_delta(deltaTimer);
		frame.clear(0x0);

		// Triangle rasterizer test
		tri.rotate(delta * 5);
		tri.translate(40, resY / 2);
		drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y,
							 tri.p2.x, tri.p2.y, 0xffff0000, 0);
		tri.translate(resX / 4, 0);
		drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y,
							 tri.p2.x, tri.p2.y, 0xffff0000, 2);
		tri.translate(resX / 4, 0);
		drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y,
							 tri.p2.x, tri.p2.y, 0xffff0000, 4);
		tri.translate(resX / 4, 0);
		drawTriangleSubPixel(frame, tri.p0.x, tri.p0.y, tri.p1.x, tri.p1.y,
							 tri.p2.x, tri.p2.y, 0xffff0000, 8);
		tri.translate(-resX / 4 * 3 - 40, -resY / 2);

		if (mfb_update_ex(window, frame.pixels, resX, resY) < 0)
			break;
	} while (true);
}

void subPixelQuad() {
	const int resX = 320, resY = 240;
	const int resScale = 2;
	Image frame(resX, resY);
	mfb_window *window = mfb_open_ex("sub-pixel rectangles", resX * resScale,
									 resY * resScale, WF_RESIZABLE);
	if (!window)
		return;
	mfb_timer *deltaTimer = mfb_timer_create();

	uint32_t b = 0xff000000;
	uint32_t w = 0xffffffff;
	// clang-format off
  uint32_t checkers[] = {
      b, w, b, w, b, w, b, w, w, b, w, b, w, b, w, b, b, w, b, w, b, w,
      b, w, w, b, w, b, w, b, w, b, b, w, b, w, b, w, b, w, w, b, w, b,
      w, b, w, b, b, w, b, w, b, w, b, w, w, b, w, b, w, b, w, b,
  };
	// clang-format on
	// Image sprite(8, 8, checkers);
	Image sprite("assets/grunt.png");
	float objZ = 4, camZ = 0, camDirZ = 1;
	int32_t fpBits = 8;
	int32_t fpOne = 1 << fpBits;
	do {
		float delta = mfb_timer_delta(deltaTimer);
		frame.clear(0xffcccccc);

		// Move camera to/away from sprite
		camZ += camDirZ * delta;
		float distance = objZ - camZ;
		if (distance < 0.1) {
			camZ = objZ - 0.1;
			camDirZ *= -1;
			continue;
		}
		if (camZ < 0) {
			camZ = 0;
			camDirZ *= -1;
		}

		// Project sprite dimensions from camera space to screen space based on
		// distance camera -> sprite. Sprite is assumed to have unit height in
		// world/camera space.
		float projectedHeight = resY * 0.5f / distance;
		float projectedWidth =
				projectedHeight * (float(sprite.width) / float(sprite.height));

		// Calculate sub pixel accurate screen coordinates of screen aligned sprite
		// billboard
		int32_t minX = floatToFixed(resX * 0.5f - projectedWidth * 0.5f, fpBits);
		int32_t minY = floatToFixed(resY * 0.5f - projectedHeight * 0.5f, fpBits);
		int32_t maxX = floatToFixed(resX * 0.5f + projectedWidth * 0.5f, fpBits);
		int32_t maxY = floatToFixed(resY * 0.5f + projectedHeight * 0.5f, fpBits);
		int32_t tx = 0, ty = 0;

		// Round top/left corner coordinate -> stabilizes rectangle on screen
		// Comment to see effect.
		minX = fixedRound(minX, fpBits);
		minY = fixedRound(minY, fpBits);

		// Calculate texture x/y gradients based on rounded screen width and height
		// of rectangle -> stabilized texture point sampling.
		int32_t w = fixedToInt(fixedRound(maxX - minX + 1, fpBits), fpBits);
		int32_t h = fixedToInt(fixedRound(maxY - minY + 1, fpBits), fpBits);
		int32_t txStep = floatToFixed(sprite.width / float(w), 16);
		int32_t tyStep = floatToFixed(sprite.height / float(h), 16);

		// Clip rectangle (and texture sampling start coordinates)
		if (minX < 0) {
			// This isn't accurate, would need to take into account minX fractional
			// part
			tx = -fixedToInt(minX, fpBits) * txStep;
			minX = 0;
		}
		if (minY < 0) {
			// This isn't accurate, would need to take into account minY fractional
			// part
			ty = -fixedToInt(minY, fpBits) * tyStep;
			minY = 0;
		}
		if (maxX >= floatToFixed(resX, fpBits))
			maxX = floatToFixed(resX - 1, fpBits);
		if (maxY >= floatToFixed(resY, fpBits))
			maxY = floatToFixed(resY - 1, fpBits);

		// Draw the clipped rectangle. Texel color of 0x00000000 -> transparent
		// TODO z-test & optimization
		int32_t px, py, ptx, pty;
		for (py = minY, pty = ty; py <= maxY; py += fpOne, pty += tyStep) {
			uint32_t *dst = frame.pixels + fixedToInt(py, fpBits) * frame.width;
			uint32_t *src = sprite.pixels + fixedToInt(pty, 16) * sprite.width;
			for (px = minX, ptx = tx; px <= maxX; px += fpOne, ptx += txStep) {
				int32_t x = fixedToInt(px, fpBits);
				int32_t u = fixedToInt(ptx, 16);
				uint32_t color = src[u];
				if (!color)
					continue;
				dst[x] = color;
			}
		}

		if (mfb_update_ex(window, frame.pixels, resX, resY) < 0)
			break;
	} while (true);
}

int main(int argc, char **argv) {
	// subPixelTriangles();
	subPixelQuad();
}