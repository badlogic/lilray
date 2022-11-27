#include "lilray-c.h"
#include "lilray.h"

using namespace lilray;

lilray_image lilray_image_create(int32_t width, int32_t height, uint32_t *pixels) {
    return (lilray_image) new Image(width, height, pixels);
}

lilray_image lilray_image_create_from_file(const char *file) {
    return (lilray_image) new Image(file);
}

FFI_EXPORT lilray_image lilray_image_create_from_memory(uint8_t *data, int32_t num_bytes) {
    return (lilray_image) new Image(data, num_bytes);
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

void lilray_image_to_rgba(lilray_image image) {
    if (!image) return;
    ((Image *) image)->toRgba();
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

lilray_sprite lilray_sprite_create(float x, float y, float height, lilray_image image) {
    Sprite *sprite = new Sprite(x, y, height, (Image *) image);
    return (lilray_sprite) sprite;
}

void lilray_sprite_dispose(lilray_sprite sprite) {
    if (!sprite) return;
    delete sprite;
}

float lilray_sprite_get_x(lilray_sprite sprite) {
    if (!sprite) return 0;
    return ((Sprite *) sprite)->x;
}

void lilray_sprite_set_x(lilray_sprite sprite, float x) {
    if (!sprite) return;
    ((Sprite *) sprite)->x = x;
}

float lilray_sprite_get_y(lilray_sprite sprite) {
    if (!sprite) return 0;
    return ((Sprite *) sprite)->y;
}

void lilray_sprite_set_y(lilray_sprite sprite, float y) {
    if (!sprite) return;
    ((Sprite *) sprite)->y = y;
}

float lilray_sprite_get_height(lilray_sprite sprite) {
    if (!sprite) return 0;
    return ((Sprite *) sprite)->height;
}

void lilray_sprite_set_height(lilray_sprite sprite, float height) {
    if (!sprite) return;
    ((Sprite *) sprite)->height = height;
}

lilray_image lilray_sprite_get_image(lilray_sprite sprite) {
    if (!sprite) return nullptr;
    return (lilray_image) ((Sprite *) sprite)->image;
}

void lilray_sprite_set_image(lilray_sprite sprite, lilray_image *image) {
    if (!sprite) return;
    ((Sprite *) sprite)->image = (Image *) image;
}

lilray_renderer lilray_renderer_create(int32_t width, int32_t height, lilray_image *wall_textures,
                                       int32_t num_wall_textures, lilray_image floor_texture,
                                       lilray_image ceiling_texture) {
    Renderer *renderer = new Renderer(width, height, (Image **) wall_textures, num_wall_textures,
                                      (Image *) floor_texture, (Image *) ceiling_texture);
    return (lilray_renderer) renderer;
}

void lilray_renderer_dispose(lilray_renderer renderer) {
    if (!renderer) return;
    delete renderer;
}

lilray_image lilray_renderer_get_frame(lilray_renderer renderer) {
    if (!renderer) return nullptr;
    Image *frame = &((Renderer *) renderer)->frame;
    return (lilray_image) frame;
}

void lilray_renderer_render(lilray_renderer renderer, lilray_camera camera, lilray_map map, lilray_sprite *sprites,
                            int num_sprites, float light_distance) {
    if (!renderer) return;
    ((Renderer *) renderer)->render(*(Camera *) camera, *(Map *) map, (Sprite **) sprites, num_sprites, light_distance);
}
