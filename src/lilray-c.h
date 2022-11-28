#ifndef LILRAY_C_H
#define LILRAY_C_H

#include <stdint.h>

#ifdef __cplusplus
#if _WIN32
#define FFI_EXPORT extern "C" __declspec(dllexport)
#else
#ifdef __EMSCRIPTEN__
#define FFI_EXPORT extern "C" __attribute__((used))
#else
#define FFI_EXPORT extern "C"
#endif
#endif
#else
#if _WIN32
#define FFI_EXPORT __declspec(dllexport)
#else
#ifdef __EMSCRIPTEN__
#define FFI_EXPORT __attribute__((used))
#else
#define FFI_EXPORT
#endif
#endif
#endif

#define FFI_OPAQUE_TYPE(name)     \
    typedef struct name##_wrapper { \
    } name##_wrapper;               \
    typedef name##_wrapper *name;

FFI_OPAQUE_TYPE(lilray_image)
FFI_EXPORT lilray_image lilray_image_create(int32_t width, int32_t height, uint32_t *pixels);
FFI_EXPORT lilray_image lilray_image_create_from_file(const char *file);
FFI_EXPORT lilray_image lilray_image_create_from_memory(uint8_t *data, int32_t num_bytes);
FFI_EXPORT void lilray_image_dispose(lilray_image image);
FFI_EXPORT int32_t lilray_image_get_width(lilray_image image);
FFI_EXPORT int32_t lilray_image_get_height(lilray_image image);
FFI_EXPORT uint32_t *lilray_image_get_pixels(lilray_image image);
FFI_EXPORT lilray_image lilray_image_get_region(lilray_image image, int32_t x, int32_t y, int32_t width,
                                                int32_t height);
FFI_EXPORT void lilray_image_clear(lilray_image image, uint32_t argb_color);
FFI_EXPORT void lilray_image_vertical_line(lilray_image image, int32_t x, int32_t y_start, int32_t y_end,
                                           uint32_t argb_color);
FFI_EXPORT void lilray_image_to_rgba(lilray_image image);

FFI_OPAQUE_TYPE(lilray_map)
FFI_EXPORT lilray_map lilray_map_create(int32_t width, int32_t height, int32_t *cells);
FFI_EXPORT void lilray_map_dispose(lilray_map map);
FFI_EXPORT int32_t lilray_map_get_width(lilray_map map);
FFI_EXPORT int32_t lilray_map_get_height(lilray_map map);
FFI_EXPORT int32_t *lilray_map_get_cells(lilray_map map);
FFI_EXPORT void lilray_map_set_cell(lilray_map map, int32_t x, int32_t y, int32_t value);
FFI_EXPORT int32_t lilray_map_get_cell(lilray_map map, int32_t x, int32_t y);

FFI_OPAQUE_TYPE(lilray_camera)
FFI_EXPORT lilray_camera lilray_camera_create(float x, float y, float angle, float field_of_view);
FFI_EXPORT void lilray_camera_dispose(lilray_camera camera);
FFI_EXPORT float lilray_camera_get_x(lilray_camera camera);
FFI_EXPORT void lilray_camera_set_x(lilray_camera camera, float x);
FFI_EXPORT float lilray_camera_get_y(lilray_camera camera);
FFI_EXPORT void lilray_camera_set_y(lilray_camera camera, float y);
FFI_EXPORT float lilray_camera_get_angle(lilray_camera camera);
FFI_EXPORT void lilray_camera_set_angle(lilray_camera camera, float angle);
FFI_EXPORT float lilray_camera_get_field_of_view(lilray_camera camera);
FFI_EXPORT void lilray_camera_set_field_of_view(lilray_camera camera, float field_of_view);
FFI_EXPORT void lilray_camera_move(lilray_camera camera, lilray_map map, float distance);
FFI_EXPORT void lilray_camera_rotate(lilray_camera camera, float degrees);

FFI_OPAQUE_TYPE(lilray_sprite)
FFI_EXPORT lilray_sprite lilray_sprite_create(float x, float y, float height, lilray_image image);
FFI_EXPORT void lilray_sprite_dispose(lilray_sprite sprite);
FFI_EXPORT float lilray_sprite_get_x(lilray_sprite sprite);
FFI_EXPORT void lilray_sprite_set_x(lilray_sprite sprite, float x);
FFI_EXPORT float lilray_sprite_get_y(lilray_sprite sprite);
FFI_EXPORT void lilray_sprite_set_y(lilray_sprite sprite, float y);
FFI_EXPORT float lilray_sprite_get_height(lilray_sprite sprite);
FFI_EXPORT void lilray_sprite_set_height(lilray_sprite sprite, float height);
FFI_EXPORT lilray_image lilray_sprite_get_image(lilray_sprite sprite);
FFI_EXPORT void lilray_sprite_set_image(lilray_sprite sprite, lilray_image image);

FFI_OPAQUE_TYPE(lilray_renderer)
FFI_EXPORT lilray_renderer lilray_renderer_create(int32_t width, int32_t height, lilray_image *wall_textures,
                                                  int32_t num_wall_textures, lilray_image floor_texture,
                                                  lilray_image ceiling_texture);
FFI_EXPORT void lilray_renderer_dispose(lilray_renderer renderer);
FFI_EXPORT lilray_image lilray_renderer_get_frame(lilray_renderer renderer);
FFI_EXPORT void
lilray_renderer_render(lilray_renderer renderer, lilray_camera camera, lilray_map map, lilray_sprite *sprites,
                       int num_sprites, float light_distance);
#endif
