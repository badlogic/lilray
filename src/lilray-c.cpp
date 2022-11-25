#include "lilray-c.h"
#include "lilray.h"

using namespace lilray;

lilray_texture
lilray_texture_create(int32_t width, int32_t height, uint8_t *pixels, uint32_t *palette, int32_t num_colors) {
    return (lilray_texture) new Texture(width, height, pixels, palette, num_colors);
}

void lilray_texture_dispose(lilray_texture texture) {
    if (!texture) return;
    delete texture;
}

int32_t lilray_texture_get_width(lilray_texture texture) {
    if (!texture) return 0;
    return ((Texture *) texture)->width;
}

int32_t lilray_texture_get_height(lilray_texture texture) {
    if (!texture) return 0;
    return ((Texture *) texture)->height;
}

uint8_t *lilray_texture_get_pixels(lilray_texture texture) {
    if (!texture) return nullptr;
    return ((Texture *) texture)->pixels;
}

uint32_t *lilray_texture_get_palette(lilray_texture texture) {
    if (!texture) return nullptr;
    return ((Texture *) texture)->palette;
}

int32_t lilray_texture_get_num_colors(lilray_texture texture) {
    if (!texture) return 0;
    return ((Texture *) texture)->numColors;
}

lilray_frame lilray_frame_create(int32_t width, int32_t height) {
    return (lilray_frame) new Frame(width, height);
}

void lilray_frame_dispose(lilray_frame frame) {
    if (!frame) return;
    delete (Frame *) frame;
}

int32_t lilray_frame_get_width(lilray_frame frame) {
    if (!frame) return 0;
    return ((Frame *) frame)->width;
}

int32_t lilray_frame_get_height(lilray_frame frame) {
    if (!frame) return 0;
    return ((Frame *) frame)->height;
}

uint32_t *lilray_frame_get_pixels(lilray_frame frame) {
    if (!frame) return nullptr;
    return ((Frame *) frame)->pixels;
}

void lilray_frame_clear(lilray_frame frame, uint32_t argb_color) {
    if (!frame) return;
    ((Frame *) frame)->clear(argb_color);
}

void lilray_draw_vertical_line(lilray_frame frame, int32_t x, int32_t y_start, int32_t y_end, uint32_t argb_color) {
    if (!frame) return;
    ((Frame *) frame)->drawVerticalLine(x, y_start, y_end, argb_color);
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

void lilray_camera_move(lilray_camera camera, float distance) {
    if (!camera) return;
    ((Camera *) camera)->move(distance);
}

void lilray_camera_rotate(lilray_camera camera, float degrees) {
    if (!camera) return;
    ((Camera *) camera)->rotate(degrees);
}

void lilray_render(lilray_frame frame, lilray_camera camera, lilray_map map, lilray_texture texture) {
    if (!frame) return;
    if (!camera) return;
    if (!map) return;
    render(*(Frame *) frame, *(Camera *) camera, *(Map *) map, *(Texture *) texture);
}
