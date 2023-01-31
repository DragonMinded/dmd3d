#include <cstdio>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "matrix.h"

#define SIGN_WIDTH 128
#define SIGN_HEIGHT 64
#define RENDER_CUBE 1

void waitForVBlank() {
    static unsigned long lastFrame = 0xFFFFFFFFFFFFFFFFL;

    while( 1 ) {
        unsigned long curFrame = lastFrame;

        FILE *fp = fopen("/sign/lastframe", "rb");
        if (fp != NULL) {
            (void)!fread(&curFrame, 1, sizeof(curFrame), fp);
            fclose(fp);
        }

        if (curFrame != lastFrame) {
            lastFrame = curFrame;
            return;
        }
    }
}

void renderFrame(uint8_t pixBuf[]) {
    FILE *fp = fopen("/sign/frame.bin", "wb");
    if (fp != NULL) {
        (void)!fwrite(pixBuf, 1, SIGN_WIDTH * SIGN_HEIGHT, fp);
        fclose(fp);
    }
}

void multiplyPoints(Matrix *matrix, Point *coords[], int length) {
    for (int i = 0; i < length; i++) {
        Point *newPoint = matrix->multiplyPoint(coords[i]);
        delete coords[i];
        coords[i] = newPoint;
    }
}

void drawPixel(uint8_t pixBuf[], int x, int y) {
    if (x < 0 || x >= SIGN_WIDTH || y < 0 || y >= SIGN_HEIGHT) {
        return;
    }

    pixBuf[x + (y * SIGN_WIDTH)] = 1;
}

void drawLine(uint8_t pixBuf[], int x0, int y0, int x1, int y1) {
    int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;){  /* loop */
        drawPixel(pixBuf, x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void drawLine(uint8_t pixBuf[], Point *first, Point *second) {
    drawLine(pixBuf, (int)first->x, (int)first->y, (int)second->x, (int)second->y);
}

void drawQuad(uint8_t pixBuf[], Point *first, Point *second, Point *third, Point *fourth) {
    drawLine(pixBuf, first, second);
    drawLine(pixBuf, second, third);
    drawLine(pixBuf, third, fourth);
    drawLine(pixBuf, fourth, first);
}

int main (int argc, char *argv[]) {
    printf("Running cube tests...\n");

    int count = 0;
    while ( 1 ) {
        // Set up our pixel buffer.
        uint8_t pixBuf[SIGN_WIDTH * SIGN_HEIGHT];
        memset(pixBuf, 0, SIGN_WIDTH * SIGN_HEIGHT);

#if RENDER_CUBE
        // Set up our throbbing cube.
        double val = (0.5 + (sin((count / 30.0) * M_PI) / 16.0));
        Point *coords[8] = {
            new Point(-val, -val, -val),
            new Point( val, -val, -val),
            new Point( val,  val, -val),
            new Point(-val,  val, -val),
            new Point(-val, -val,  val),
            new Point( val, -val,  val),
            new Point( val,  val,  val),
            new Point(-val,  val,  val),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();
        effectsMatrix->translateZ(2.5);
        effectsMatrix->rotateX(60 + count);
        effectsMatrix->rotateY(30 + count);
        multiplyPoints(effectsMatrix, coords, sizeof(coords) / sizeof(coords[0]));

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Move the cube to where it should go.
        multiplyPoints(viewMatrix, coords, sizeof(coords) / sizeof(coords[0]));

        // Draw the cube.
        drawQuad(pixBuf, coords[0], coords[1], coords[2], coords[3]);
        drawQuad(pixBuf, coords[4], coords[5], coords[6], coords[7]);
        drawLine(pixBuf, coords[0], coords[4]);
        drawLine(pixBuf, coords[1], coords[5]);
        drawLine(pixBuf, coords[2], coords[6]);
        drawLine(pixBuf, coords[3], coords[7]);
#else
        Point *coords[4] = {
            new Point(42, 10, 0),
            new Point(86, 10, 0),
            new Point(86, 54, 0),
            new Point(42, 54, 0),
        };

        Matrix *rotMatrix = new Matrix();
        Point *origin = new Point(64, 32, 0);
        rotMatrix->rotateOriginZ(origin, count * 3);
        multiplyPoints(rotMatrix, coords, sizeof(coords) / sizeof(coords[0]));

        drawQuad(pixBuf, coords[0], coords[1], coords[2], coords[3]);
#endif

        // Render it to the screen.
        waitForVBlank();
        renderFrame(pixBuf);

        // Clean up.
        for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
            delete coords[i];
        }

        // Keep track of location.
        count++;
    }

    printf("Done!\n");

    return 0;
}
