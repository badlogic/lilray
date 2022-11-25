#include <stdint.h>
#include <MiniFB.h>
#include <lilray.h>

using namespace lilray;

int main(int argc, char **argv) {
    Texture textures("./wolftextures.png");
    Texture *texture = textures.getRegion(0, 0, 64, 64);
    Frame frame(800, 600);
    int32_t cells[] = {
            1, 1, 1, 1, 1,
            1, 0, 0, 0, 1,
            1, 0, 0, 0, 1,
            1, 0, 0, 1, 1,
            1, 1, 1, 1, 1
    };
    Map map(5, 5, cells);
    Camera camera(2, 2, 0, 66);
    float rotationSpeed = 90;
    float movementSpeed = 2;

    mfb_window *window = mfb_open_ex("lilray", 800, 600, WF_RESIZABLE);
    if (!window) return 0;
    struct mfb_timer *timer = mfb_timer_create();
    do {
        float delta = mfb_timer_delta(timer);
        if (mfb_get_key_buffer(window)[KB_KEY_A]) camera.rotate(-rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_D]) camera.rotate(rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_W]) camera.move(movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_S]) camera.move(-movementSpeed * delta);

        frame.clear(0);
        render(frame, camera, map, *texture);
        if (mfb_update_ex(window, frame.pixels, 800, 600) < 0) break;
    } while (mfb_wait_sync(window));
}