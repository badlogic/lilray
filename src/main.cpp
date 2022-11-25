#include <stdint.h>
#include <MiniFB.h>
#include <lilray.h>

using namespace lilray;

int main(int argc, char **argv) {
    int resX = 640, resY = 480;

    Image frame(resX, resY);
    float *zbuffer = new float[resX];

    Image *textures[] = {
            new Image("STARG2.png"),
            new Image("STARG3.png"),
            new Image("STARGR2.png"),
            new Image("TEKWALL1.png"),
            new Image("TEKWALL2.png"),
            new Image("TEKWALL3.png"),
            new Image("TEKWALL4.png"),
    };
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
    Image grunt("grunt.png");
    Sprite *sprites[] = {
            new Sprite(3.5f, 1.5f, 0.5f, &grunt),
            new Sprite(7.5f, 1.5f, 0.2, &grunt),
            new Sprite(1.5f, 1.5f, 0.7, &grunt)
    };
    Camera camera(2.5, 2.5, 0, 66);
    float rotationSpeed = 40;
    float movementSpeed = 2;

    mfb_window *window = mfb_open_ex("lilray", resX * 2, resY * 2, WF_RESIZABLE);
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
        render(frame, zbuffer, camera, map, sprites, sizeof(sprites) / sizeof(Sprite *), textures, textures[1],
               textures[1], 6);
        printf("frame time: %f, pos: %f, %f\n", mfb_timer_now(frameTimer) - start, camera.x, camera.y);
        if (mfb_update_ex(window, frame.pixels, resX, resY) < 0) break;
    } while (true);
}