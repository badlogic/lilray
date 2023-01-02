#ifndef LILRAY_H
#define LILRAY_H

#include <stdint.h>

namespace lilray {
	struct Font;

	struct Image {
		int32_t width, height;
		uint32_t *pixels;

		explicit Image(const char *imageFile);

		Image(uint8_t *imageBytes, int32_t numBytes);

		Image(int32_t width, int32_t height, const uint32_t *pixels = nullptr);

		~Image();

		Image *getRegion(int32_t x, int32_t y, int32_t w, int32_t h);

		void clear(uint32_t clearColor);

		void drawVerticalLine(int32_t x, int32_t ys, int32_t ye, uint32_t color);

		void drawVerticalImageSlice(Image &texture, int32_t x, int32_t ys, int32_t ye, int32_t tx,
									uint8_t lightness);

		void drawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);

		void drawText(Font &font, int32_t x, int32_t y, uint32_t color, const char *fmt, ...);

		void reverseColorChannels();
	};

	struct Font {
		uint8_t *pixels;
		int32_t width;
		int32_t height;
		int32_t charWidth;
		int32_t charHeight;
		int32_t charsX;
		int32_t charsY;

		Font(const char *imageFile, int32_t charWidth, int32_t charHeight);

		Font(uint8_t *imageBytes, int32_t numBytes, int32_t charWidth, int32_t charHeight);

		void getBounds(int32_t &width, int32_t &height, const char *fmt, ...);
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
		bool useFixedPoint;
		bool drawWalls;
		bool drawFloorAndCeiling;
		bool drawSprites;

		Renderer(int32_t width, int32_t height, Image *wallTextures[], int32_t numWallTextures,
				 Image *floorTexture = nullptr, Image *ceilingTexture = nullptr);

		void render(Camera &camera, Map &map, Sprite *sprites[], int32_t numSprites, float lightDistance);
	};

	struct Average {
		double *values;
		int32_t index;
		int32_t windowSize;
		int32_t writtenValues;

		Average(int32_t windowSize) : windowSize(windowSize), index(0), writtenValues(0) {
			values = new double[windowSize];
		}

		~Average() {
			delete values;
		}

		void addValue(double value) {
			values[index] = value;
			index = (index + 1) % windowSize;
			if (writtenValues < windowSize) writtenValues++;
		}

		double getAverage() {
			double sum = 0;
			for (int i = 0; i < writtenValues; i++) {
				sum += values[i];
			}
			return sum / writtenValues;
		}
	};
}// namespace lilray

#endif