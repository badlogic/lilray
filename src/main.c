#include <stdint.h>
#include <MiniFB.h>
#include <lilray-c.h>

int main(int argc, char **argv) {
    lilray_frame frame = lilray_frame_create(800, 600);
    int32_t cells[] = {
            1, 1, 1, 1, 1,
            1, 0, 0, 0, 1,
            1, 0, 0, 0, 1,
            1, 0, 0, 1, 1,
            1, 1, 1, 1, 1
    };
    lilray_map map = lilray_map_create(5, 5, cells);
    lilray_camera camera = lilray_camera_create(2, 2, 0, 60);

    struct mfb_window *window = mfb_open_ex("lilray", 800, 600, WF_RESIZABLE);
    if (!window) return 0;

    do {
        if (mfb_get_key_buffer(window)[KB_KEY_A]) lilray_camera_rotate(camera, -60 / 60);
        if (mfb_get_key_buffer(window)[KB_KEY_D]) lilray_camera_rotate(camera, 60 / 60);
        if (mfb_get_key_buffer(window)[KB_KEY_W]) lilray_camera_move(camera, 0.5 / 60);
        if (mfb_get_key_buffer(window)[KB_KEY_S]) lilray_camera_move(camera, -0.5 / 60);

        lilray_frame_clear(frame, 0);
        lilray_render(frame, camera, map);
        if (mfb_update_ex(window, lilray_frame_get_pixels(frame), 800, 600) < 0) break;
    } while (mfb_wait_sync(window));
}
