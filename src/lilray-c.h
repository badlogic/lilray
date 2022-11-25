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

#define LILRAY_ARGB(r, g, b, a) (uint32_t)(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))

FFI_OPAQUE_TYPE(lilray_texture)
FFI_EXPORT lilray_texture
lilray_texture_create(int32_t width, int32_t height, uint32_t *pixels);
FFI_EXPORT lilray_texture
lilray_texture_create_image(const char *file);
FFI_EXPORT lilray_texture
lilray_texture_create_palette(int32_t width, int32_t height, uint8_t *pixels, uint32_t *palette, int32_t num_colors);
FFI_EXPORT void lilray_texture_dispose(lilray_texture texture);
FFI_EXPORT int32_t lilray_texture_get_width(lilray_texture texture);
FFI_EXPORT int32_t lilray_texture_get_height(lilray_texture texture);
FFI_EXPORT uint32_t *lilray_texture_get_pixels(lilray_texture texture);
FFI_EXPORT lilray_texture lilray_texture_get_region(lilray_texture texture, int32_t x, int32_t y, int32_t width, int32_t height);

FFI_OPAQUE_TYPE(lilray_frame)
FFI_EXPORT lilray_frame lilray_frame_create(int32_t width, int32_t height);
FFI_EXPORT void lilray_frame_dispose(lilray_frame frame);
FFI_EXPORT int32_t lilray_frame_get_width(lilray_frame frame);
FFI_EXPORT int32_t lilray_frame_get_height(lilray_frame frame);
FFI_EXPORT uint32_t *lilray_frame_get_pixels(lilray_frame frame);
FFI_EXPORT void lilray_frame_clear(lilray_frame frame, uint32_t argb_color);
FFI_EXPORT void
lilray_draw_vertical_line(lilray_frame frame, int32_t x, int32_t y_start, int32_t y_end, uint32_t argb_color);

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
FFI_EXPORT void lilray_camera_move(lilray_camera camera, float distance);
FFI_EXPORT void lilray_camera_rotate(lilray_camera camera, float degrees);

FFI_EXPORT void lilray_render(lilray_frame frame, lilray_camera camera, lilray_map map, lilray_texture *textures);
FFI_EXPORT void lilray_argb_to_rgba(uint8_t *argb, uint8_t *rgba, int32_t numPixels);
#endif
