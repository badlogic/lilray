#include <stdint.h>
#include <MiniFB.h>
#include <lilray.h>

using namespace lilray;

static Renderer *renderer;

int main(int argc, char **argv) {
    const int resX = 640, resY = 480, resScale = 2;
    const float rotationSpeed = 70;
    const float movementSpeed = 2.5;

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
    Font font("font.png", 6, 12);
    Map map(21, 21, cells);
    Image grunt("grunt.png");
    Sprite *sprites[] = {
            new Sprite(3.5f, 2.5f, 0.7f, &grunt),
            new Sprite(4.5f, 1.5f, 0.7f, &grunt),
            new Sprite(5.5f, 2.0f, 0.7f, &grunt),
    };
    Camera camera(2.5f, 2.5f, 0, 66);
    renderer = new Renderer(resX, resY, textures, sizeof(textures) / sizeof(Image *), textures[1], textures[2]);

    mfb_window *window = mfb_open_ex("lilray", resX * resScale, resY * resScale, WF_RESIZABLE);
    if (!window) return 0;
    mfb_timer *deltaTimer = mfb_timer_create();
    mfb_timer *frameTimer = mfb_timer_create();
    mfb_set_char_input_callback(window, [](mfb_window *window, unsigned int character) {
        if (character == '0') renderer->useFixedPoint = !renderer->useFixedPoint;
        if (character == '1') renderer->drawWalls = !renderer->drawWalls;
        if (character == '2') renderer->drawFloorAndCeiling = !renderer->drawFloorAndCeiling;
        if (character == '3') renderer->drawSprites = !renderer->drawSprites;
    });
    Average avgFrameTime(50);
    do {
        float delta = mfb_timer_delta(deltaTimer);
        if (mfb_get_key_buffer(window)[KB_KEY_A]) camera.rotate(-rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_D]) camera.rotate(rotationSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_W]) camera.move(map, movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_S]) camera.move(map, -movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_Q]) camera.strafe(map, movementSpeed * delta);
        if (mfb_get_key_buffer(window)[KB_KEY_E]) camera.strafe(map, -movementSpeed * delta);

        double start = mfb_timer_now(frameTimer);
        renderer->render(camera, map, sprites, sizeof(sprites) / sizeof(Sprite *), 6);
        avgFrameTime.addValue(mfb_timer_now(frameTimer) - start);

        char text[255];
        snprintf(text, 255,
                 "Frame time: %f\n(0) Use Fixed point:    %s\n(1) Draw walls:         %s\n(2) Draw floor/ceiling: %s\n(3) Draw sprites:       %s",
                 avgFrameTime.getAverage(),
                 renderer->useFixedPoint ? "true" : "false",
                 renderer->drawWalls ? "true" : "false",
                 renderer->drawFloorAndCeiling ? "true" : "false",
                 renderer->drawSprites ? "true" : "false");
        int32_t textWidth, textHeight;
        font.getBounds(textWidth, textHeight, text);
        renderer->frame.drawRectangle(0, 0, textWidth, textHeight, 0xff222222);
        renderer->frame.drawText(font, 0, 1, 0xffcccccc, text);
        if (mfb_update_ex(window, renderer->frame.pixels, resX, resY) < 0) break;
    } while (true);
}