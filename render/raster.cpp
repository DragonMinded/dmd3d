#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include <unistd.h>
#include "raster.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

UV::UV(double u, double v) {
    this->u = u;
    this->v = v;
}

Texture::Texture(int width, int height) {
    this->width = width;
    this->height = height;
    this->data = (unsigned char *)malloc(width * height);
    this->managed = 1;
    this->mode = CLAMP_MODE_NORMAL;
}

Texture::Texture(int width, int height, unsigned char data[]) {
    this->width = width;
    this->height = height;
    this->data = data;
    this->managed = 0;
    this->mode = CLAMP_MODE_NORMAL;
}

Texture::~Texture() {
    if (this->managed) {
        this->managed = 0;
        free(this->data);
    }

    this->data = 0;
}

void Texture::setClampMode(int mode) {
    switch (mode) {
        case CLAMP_MODE_NORMAL:
        case CLAMP_MODE_MIRROR:
        case CLAMP_MODE_TILE:
            this->mode = mode;
            break;
    }
}

unsigned char Texture::valueAt(double u, double v) {
    switch (mode) {
        case CLAMP_MODE_NORMAL:
            if (u < 0.0) { u = 0.0; }
            if (u > 1.0) { u = 1.0; }
            if (v < 0.0) { v = 0.0; }
            if (v > 1.0) { v = 1.0; }
            break;
        case CLAMP_MODE_MIRROR:
        case CLAMP_MODE_TILE:
            // TODO: Implement these!
            return 0;
    }

    int x = (int)(u * (double)(width - 1));
    int y = (int)(v * (double)(height - 1));

    return data[x + (y * width)];
}

void Screen::clear() {
    memset(pixBuf, 0, SIGN_WIDTH * SIGN_HEIGHT);

    for (int i = 0; i < SIGN_WIDTH * SIGN_HEIGHT; i++) {
        zBuf[i] = std::numeric_limits<double>::infinity();
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

        // Give it a rest.
        usleep(1000);
    }
}

void Screen::renderFrame() {
    FILE *fp = fopen("/sign/frame.bin", "wb");
    if (fp != NULL) {
        (void)!fwrite(pixBuf, 1, SIGN_WIDTH * SIGN_HEIGHT, fp);
        fclose(fp);
    }
}

bool Screen::_getPixel(int x, int y) {
    return pixBuf[x + (y * SIGN_WIDTH)] != 0;
}

void Screen::drawPixel(int x, int y, double z, bool on) {
    if (x < 0 || x >= SIGN_WIDTH || y < 0 || y >= SIGN_HEIGHT) {
        return;
    }

    // Z is technically 1/W, where W is -Z, so invert it.
    z = -1 / z;

    if (z < 0.0) {
        return;
    }
    if (z > zBuf[x + (y * SIGN_WIDTH)]) {
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

void Screen::drawTri(Point *first, Point *second, Point *third, bool on) {
    drawLine(first, second, on);
    drawLine(second, third, on);
    drawLine(third, first, on);
}

void Screen::drawQuad(Point *first, Point *second, Point *third, Point *fourth, bool on) {
    drawLine(first, second, on);
    drawLine(second, third, on);
    drawLine(third, fourth, on);
    drawLine(fourth, first, on);
}

void Screen::drawTexturedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex) {
    // Calculate the bounds.
    int minX = (int)MIN(MIN(first->x, second->x), third->x);
    int minY = (int)MIN(MIN(first->y, second->y), third->y);
    int maxX = (int)MAX(MAX(first->x, second->x), third->x);
    int maxY = (int)MAX(MAX(first->y, second->y), third->y);

    if (minX >= SIGN_WIDTH || maxX < 0) { return; }
    if (minY >= SIGN_HEIGHT || maxY < 0) { return; }

    // Due to the way projectPoint works, each point is already in the form of X/W, Y/W, 1/W, so we can interpolate
    // UV coordinates based on the fact that perspective projects are linear in this coordinate system. So, we need
    // to construct a matrix that will, given a screen X/Y coordinate will work us back to the virtual U/V coordinate
    // of where we fall on the triangle.
    Matrix *xyMatrix = new Matrix();
    xyMatrix->a11 = second->x - first->x;
    xyMatrix->a12 = second->y - first->y;
    xyMatrix->a21 = third->x - first->x;
    xyMatrix->a22 = third->y - first->y;
    xyMatrix->a41 = first->x;
    xyMatrix->a42 = first->y;
    xyMatrix->invert();

    Matrix *xywMatrix = new Matrix();
    xywMatrix->a11 = second->x - first->x;
    xywMatrix->a12 = second->y - first->y;
    xywMatrix->a13 = second->z - first->z;
    xywMatrix->a21 = third->x - first->x;
    xywMatrix->a22 = third->y - first->y;
    xywMatrix->a23 = third->z - first->z;
    xywMatrix->a41 = first->x;
    xywMatrix->a42 = first->y;
    xywMatrix->a43 = first->z;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Point *curPoint = new Point(x, y, 0.0);
            Point *triPoint = xyMatrix->multiplyPoint(curPoint);

            // Make sure that we stay within bounds of the triangle.
            if (triPoint->x < 0.0 || triPoint->x > 1.0) {
                delete curPoint;
                delete triPoint;
                continue;
            }
            if (triPoint->y < 0.0 || triPoint->y > 1.0) {
                delete curPoint;
                delete triPoint;
                continue;
            }
            if (triPoint->y > (1.0 - triPoint->x)) {
                delete curPoint;
                delete triPoint;
                continue;
            }

            // TODO: Figure out the 1/W coordinate for this pixel.
            Point *actualPoint = xywMatrix->multiplyPoint(triPoint);
            drawPixel(x, y, actualPoint->z, false);
            delete actualPoint;
            delete triPoint;
            delete curPoint;
        }
    }

    delete xywMatrix;
    delete xyMatrix;
}

void Screen::drawTexturedQuad(
    Point *first, Point *second, Point *third, Point *fourth,
    UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
    Texture *tex
) {
    drawTexturedTri(first, second, fourth, firstTex, secondTex, fourthTex, tex);
    drawTexturedTri(second, third, fourth, secondTex, thirdTex, fourthTex, tex);
}

void Screen::_drawOccludedTri(Point *first, Point *second, Point *third, Screen *screen) {
    // Calculate the bounds.
    int minX = (int)MIN(MIN(first->x, second->x), third->x);
    int minY = (int)MIN(MIN(first->y, second->y), third->y);
    int maxX = (int)MAX(MAX(first->x, second->x), third->x);
    int maxY = (int)MAX(MAX(first->y, second->y), third->y);

    if (minX >= SIGN_WIDTH || maxX < 0) { return; }
    if (minY >= SIGN_HEIGHT || maxY < 0) { return; }

    // Due to the way projectPoint works, each point is already in the form of X/W, Y/W, 1/W, so we can interpolate
    // UV coordinates based on the fact that perspective projects are linear in this coordinate system. So, we need
    // to construct a matrix that will, given a screen X/Y coordinate will work us back to the virtual U/V coordinate
    // of where we fall on the triangle.
    Matrix *xyMatrix = new Matrix();
    xyMatrix->a11 = second->x - first->x;
    xyMatrix->a12 = second->y - first->y;
    xyMatrix->a21 = third->x - first->x;
    xyMatrix->a22 = third->y - first->y;
    xyMatrix->a41 = first->x;
    xyMatrix->a42 = first->y;
    xyMatrix->invert();

    Matrix *xywMatrix = new Matrix();
    xywMatrix->a11 = second->x - first->x;
    xywMatrix->a12 = second->y - first->y;
    xywMatrix->a13 = second->z - first->z;
    xywMatrix->a21 = third->x - first->x;
    xywMatrix->a22 = third->y - first->y;
    xywMatrix->a23 = third->z - first->z;
    xywMatrix->a41 = first->x;
    xywMatrix->a42 = first->y;
    xywMatrix->a43 = first->z;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Point *curPoint = new Point(x, y, 0.0);
            Point *triPoint = xyMatrix->multiplyPoint(curPoint);

            // Make sure that we stay within bounds of the triangle.
            if (triPoint->x < 0.0 || triPoint->x > 1.0) {
                delete curPoint;
                delete triPoint;
                continue;
            }
            if (triPoint->y < 0.0 || triPoint->y > 1.0) {
                delete curPoint;
                delete triPoint;
                continue;
            }
            if (triPoint->y > (1.0 - triPoint->x)) {
                delete curPoint;
                delete triPoint;
                continue;
            }

            // Figure out the 1/W coordinate for this pixel.
            Point *actualPoint = xywMatrix->multiplyPoint(triPoint);
            drawPixel(x, y, actualPoint->z, screen->_getPixel(x, y));
            delete actualPoint;
            delete triPoint;
            delete curPoint;
        }
    }

    delete xywMatrix;
    delete xyMatrix;
}

void Screen::drawOccludedTri(Point *first, Point *second, Point *third) {
    // First, we draw the border, so that we have the "texture" to pull from when we want to outline the triangle.
    Screen *tmpScreen = new Screen();
    tmpScreen->clear();
    tmpScreen->drawTri(first, second, third, true);

    // Now, draw the "texture".
    _drawOccludedTri(first, second, third, tmpScreen);
    delete tmpScreen;
}

void Screen::drawOccludedQuad(Point *first, Point *second, Point *third, Point *fourth) {
    // First, we draw the border, so that we have the "texture" to pull from when we want to outline the quad.
    Screen *tmpScreen = new Screen();
    tmpScreen->clear();
    tmpScreen->drawQuad(first, second, third, fourth, true);

    // Now, draw the "texture" in two quads.
    _drawOccludedTri(first, second, fourth, tmpScreen);
    _drawOccludedTri(second, third, fourth, tmpScreen);
    delete tmpScreen;
}
