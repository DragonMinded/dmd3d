#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include "raster.h"

void Screen::clear() {
    memset(pixBuf, 0, SIGN_WIDTH * SIGN_HEIGHT);

    for (int i = 0; i < SIGN_WIDTH * SIGN_HEIGHT; i++) {
        zBuf[i] = -std::numeric_limits<double>::infinity();
    }
}

void Screen::waitForVBlank() {
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

void Screen::renderFrame() {
    FILE *fp = fopen("/sign/frame.bin", "wb");
    if (fp != NULL) {
        (void)!fwrite(pixBuf, 1, SIGN_WIDTH * SIGN_HEIGHT, fp);
        fclose(fp);
    }
}

void Screen::drawPixel(int x, int y, double z, bool on) {
    if (x < 0 || x >= SIGN_WIDTH || y < 0 || y >= SIGN_HEIGHT) {
        return;
    }
    if (z > 0.0) {
        return;
    }
    if (z < zBuf[x + (y * SIGN_WIDTH)]) {
        return;
    }

    pixBuf[x + (y * SIGN_WIDTH)] = on ? 1 : 0;
    zBuf[x + (y * SIGN_WIDTH)] = z;
}

void Screen::drawLine(int x0, int y0, double z0, int x1, int y1, double z1, bool on) {
    int dx =  abs (x1 - x0);
    int dy = -abs (y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    // First, calculate how many steps we must make.
    int steps = -1;
    int origX = x0;
    int origY = y0;

    for (;;){
        steps++;
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }

    // Given those steps, figure out dz.
    double dz = (steps <= 0) ? 0.0 : ((z1 - z0) / (double)steps);

    // Reset our lines.
    x0 = origX;
    y0 = origY;
    err = dx + dy;

    // Now, draw it.
    for (;;){
        drawPixel(x0, y0, z0, on);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
        z0 += dz;
    }
}

void Screen::drawLine(Point *first, Point *second, bool on) {
    drawLine((int)first->x, (int)first->y, first->z, (int)second->x, (int)second->y, second->z, on);
}

void Screen::drawQuad(Point *first, Point *second, Point *third, Point *fourth, bool on) {
    drawLine(first, second, on);
    drawLine(second, third, on);
    drawLine(third, fourth, on);
    drawLine(fourth, first, on);
}
