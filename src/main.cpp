#include <stdint.h>
#include <MiniFB.h>
#include <lilray.h>

using namespace lilray;

int main(int argc, char **argv) {
    int resX = 1600;
    int resY = 1200;

    Image image("./wolftextures.png");
    Image *textures[] = {
            image.getRegion(0, 0, 64, 64),
            image.getRegion(64, 0, 64, 64),
            image.getRegion(128, 0, 64, 64),
            image.getRegion(192, 0, 64, 64),
            image.getRegion(256, 0, 64, 64)
    };
    Image frame(resX, resY);
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
    Map map(21, 21, cells);
    Camera camera(2.5, 2.5, 0, 60);
    float rotationSpeed = 90;
    float movementSpeed = 2;

    mfb_window *window = mfb_open_ex("lilray", resX, resY, WF_RESIZABLE);
    if (!window) return 0;
    mfb_timer *deltaTimer = mfb_timer_create();
    mfb_timer *frameTimer = mfb_timer_create();
    do {
        float delta = mfb_timer_delta(deltaTimer);
        if (mfb_get_key_buffer(window)[KB_KEY_A]) camera.rotate(-rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_D]) camera.rotate(rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_W]) camera.move(map, movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_S]) camera.move(map, -movementSpeed * delta);

        double start = mfb_timer_now(frameTimer);
        frame.clear(0);
        render(frame, camera, map, textures);
        printf("frame time: %f, pos: %f, %f\n", mfb_timer_now(frameTimer) - start, camera.x, camera.y);
        if (mfb_update_ex(window, frame.pixels, resX, resY) < 0) break;
    } while (true);
}