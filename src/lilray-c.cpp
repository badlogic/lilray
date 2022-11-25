#include "lilray-c.h"
#include "lilray.h"

using namespace lilray;

lilray_image lilray_image_create(int32_t width, int32_t height, uint32_t *pixels) {
    return (lilray_image) new Image(width, height, pixels);
}

lilray_image lilray_image_create_from_file(const char *file) {
    return (lilray_image) new Image(file);
}

void lilray_image_dispose(lilray_image texture) {
    if (!texture) return;
    delete texture;
}

int32_t lilray_image_get_width(lilray_image texture) {
    if (!texture) return 0;
    return ((Image *) texture)->width;
}

int32_t lilray_image_get_height(lilray_image texture) {
    if (!texture) return 0;
    return ((Image *) texture)->height;
}

uint32_t *lilray_image_get_pixels(lilray_image texture) {
    if (!texture) return nullptr;
    return ((Image *) texture)->pixels;
}

lilray_image lilray_image_get_region(lilray_image texture, int32_t x, int32_t y, int32_t width, int32_t height) {
    if (!texture) return nullptr;
    return (lilray_image) ((Image *) texture)->getRegion(x, y, width, height);
}

void lilray_image_clear(lilray_image image, uint32_t argb_color) {
    if (!image) return;
    ((Image *) image)->clear(argb_color);
}

void
lilray_image_draw_vertical_line(lilray_image image, int32_t x, int32_t y_start, int32_t y_end, uint32_t argb_color) {
    if (!image) return;
    ((Image *) image)->drawVerticalLine(x, y_start, y_end, argb_color);
}

lilray_map lilray_map_create(int32_t width, int32_t height, int32_t *cells) {
    return (lilray_map) new Map(width, height, cells);
}

void lilray_map_dispose(lilray_map map) {
    delete (Map *) map;
}

int32_t lilray_map_get_width(lilray_map map) {
    if (!map) return 0;
    return ((Map *) map)->width;
}

int32_t lilray_map_get_height(lilray_map map) {
    if (!map) return 0;
    return ((Map *) map)->height;
}

int32_t *lilray_map_get_cells(lilray_map map) {
    if (!map) return nullptr;
    return ((Map *) map)->cells;
}

void lilray_map_set_cell(lilray_map map, int32_t x, int32_t y, int32_t value) {
    if (!map) return;
    ((Map *) map)->setCell(x, y, value);
}

int32_t lilray_map_get_cell(lilray_map map, int32_t x, int32_t y) {
    if (!map) return 0;
    return ((Map *) map)->getCell(x, y);
}

lilray_camera lilray_camera_create(float x, float y, float angle, float field_of_view) {
    return (lilray_camera) new Camera(x, y, angle, field_of_view);
}

void lilray_camera_dispose(lilray_camera camera) {
    if (!camera) return;
    delete ((Camera *) camera);
}

float lilray_camera_get_x(lilray_camera camera) {
    if (!camera) return 0;
    return ((Camera *) camera)->x;
}

void lilray_camera_set_x(lilray_camera camera, float x) {
    if (!camera) return;
    ((Camera *) camera)->x = x;
}

float lilray_camera_get_y(lilray_camera camera) {
    if (!camera) return 0;
    return ((Camera *) camera)->y;
}

void lilray_camera_set_y(lilray_camera camera, float y) {
    if (!camera) return;
    ((Camera *) camera)->y = y;
}

float lilray_camera_get_angle(lilray_camera camera) {
    if (!camera) return 0;
    return ((Camera *) camera)->angle;
}

void lilray_camera_set_angle(lilray_camera camera, float angle) {
    if (!camera) return;
    ((Camera *) camera)->angle = angle;
}

float lilray_camera_get_field_of_view(lilray_camera camera) {
    if (!camera) return 0;
    return ((Camera *) camera)->fieldOfView;
}

void lilray_camera_set_field_of_view(lilray_camera camera, float fieldOfView) {
    if (!camera) return;
    ((Camera *) camera)->fieldOfView = fieldOfView;
}

void lilray_camera_move(lilray_camera camera, lilray_map map, float distance) {
    if (!camera) return;
    ((Camera *) camera)->move(*(Map *) map, distance);
}

void lilray_camera_rotate(lilray_camera camera, float degrees) {
    if (!camera) return;
    ((Camera *) camera)->rotate(degrees);
}

void lilray_render(lilray_image frame, lilray_camera camera, lilray_map map, lilray_image *walls, lilray_image floor,
                   lilray_image ceiling, float lightDistance) {
    if (!frame) return;
    if (!camera) return;
    if (!map) return;
    render(*(Image *) frame, *(Camera *) camera, *(Map *) map, (Image **) walls, (Image *) floor, (Image *) ceiling,
           lightDistance);
}

void lilray_argb_to_rgba(uint32_t *argb, uint32_t *rgba, int32_t numPixels) {
    lilray::argb_to_rgba(argb, rgba, numPixels);
}
