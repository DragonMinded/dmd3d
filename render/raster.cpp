#include <cstdio>
#include <cstring>
#include <cmath>
#include "raster.h"

void Screen::clear() {
    memset(pixBuf, 0, SIGN_WIDTH * SIGN_HEIGHT);
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

void Screen::drawPixel(int x, int y) {
    if (x < 0 || x >= SIGN_WIDTH || y < 0 || y >= SIGN_HEIGHT) {
        return;
    }

    pixBuf[x + (y * SIGN_WIDTH)] = 1;
}

void Screen::drawLine(int x0, int y0, int x1, int y1) {
    int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;){  /* loop */
        drawPixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void Screen::drawLine(Point *first, Point *second) {
    drawLine((int)first->x, (int)first->y, (int)second->x, (int)second->y);
}

void Screen::drawQuad(Point *first, Point *second, Point *third, Point *fourth) {
    drawLine(first, second);
    drawLine(second, third);
    drawLine(third, fourth);
    drawLine(fourth, first);
}
