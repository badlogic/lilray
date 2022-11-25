#include <stdint.h>
#include <MiniFB.h>
#include <lilray-c.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int32_t frameWidth = 320;
    int32_t frameHeight = 240;

    lilray_image image = lilray_image_create_from_file("./wolftextures.png");
    lilray_image textures[] = {
            lilray_image_get_region(image, 0, 0, 64, 64),
            lilray_image_get_region(image, 64, 0, 64, 64),
            lilray_image_get_region(image, 128, 0, 64, 64),
            lilray_image_get_region(image, 192, 0, 64, 64),
            lilray_image_get_region(image, 256, 0, 64, 64)
    };
    lilray_image frame = lilray_image_create(frameWidth, frameHeight, NULL);
    int32_t cells[] = {
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 2, 2, 2, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    lilray_map map = lilray_map_create(21, 21, cells);
    lilray_camera camera = lilray_camera_create(2.5f, 2.5f, 90, 66);
    float rotationSpeed = 90;
    float movementSpeed = 2;

    struct mfb_window *window = mfb_open_ex("lilray", frameWidth * 2, frameHeight * 2, WF_RESIZABLE);
    if (!window) return 0;
    struct mfb_timer *deltaTimer = mfb_timer_create();
    do {
        float delta = mfb_timer_delta(deltaTimer);
        if (mfb_get_key_buffer(window)[KB_KEY_A]) lilray_camera_rotate(camera, -rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_D]) lilray_camera_rotate(camera, rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_W]) lilray_camera_move(camera, map, movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_S]) lilray_camera_move(camera, map, -movementSpeed * delta);

        lilray_image_clear(frame, 0);
        lilray_render(frame, camera, map, textures);
        if (mfb_update_ex(window, lilray_image_get_pixels(frame), frameWidth, frameHeight) < 0) break;
    } while (-1);
}
