#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include <unistd.h>
#include "raster.h"
#include "common.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

UV::UV(double reqU, double reqV) : u(reqU), v(reqV) {
    // Basically a struct with read-only members.
}

Texture::Texture(int width, int height, unsigned char data[]) {
    this->width = width;
    this->height = height;
    this->mode = CLAMP_MODE_NORMAL;

    this->managed = 1;
    this->data = (unsigned char *)malloc(width * height);
    memcpy(this->data, data, width * height);
}

Texture::Texture(const char * const filename) {
    // First, assume failure loading the texture.
    this->width = 0;
    this->height = 0;
    this->managed = 0;
    this->data = 0;
    this->mode = CLAMP_MODE_NORMAL;

    // Now, actually load it.
    char cmd[1024];
    sprintf(cmd, "python3 texload.py \"%s\"", filename);

    FILE *pipe = popen(cmd, "r");
    if (pipe != NULL) {
        // First read the dimensions.
        short width = 0;
        short height = 0;

        (void)!fread(&width, 1, sizeof(width), pipe);
        (void)!fread(&height, 1, sizeof(height), pipe);

        if (width > 0 && height > 0) {
            this->width = width;
            this->height = height;
            this->managed = 1;
            this->data = (unsigned char *)malloc(width * height);

            (void)!fread(this->data, 1, width * height, pipe);
        }

        pclose(pipe);
    }
}

Texture::~Texture() {
    if (this->managed) {
        this->managed = 0;
        free(this->data);
    }

    this->data = 0;
}

Texture *Texture::clone() {
    Texture *newTex = new Texture(width, height, data);
    newTex->setClampMode(mode);
    return newTex;
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

bool Texture::valueAt(double u, double v) {
    if (data == 0) { return false; }

    switch (mode) {
        case CLAMP_MODE_NORMAL:
            if (u < 0.0) { u = 0.0; }
            if (u > 1.0) { u = 1.0; }
            if (v < 0.0) { v = 0.0; }
            if (v > 1.0) { v = 1.0; }
            break;
        case CLAMP_MODE_MIRROR:
            // First, fold along axis, because mirroring works
            // at the UV zero crossing axis.
            u = fabs(u);
            v = fabs(v);

            // Now work out the fractional part and integer part.
            double uInt;
            double vInt;
            u = modf(u, &uInt);
            v = modf(v, &vInt);

            // Now, if the integer part is odd, we flip the direction.
            if (((int)uInt) & 1) {
                u = 1.0 - u;
            }
            if (((int)vInt) & 1) {
                v = 1.0 - v;
            }
            break;
        case CLAMP_MODE_TILE:
            double intPart;

            u = modf(u, &intPart);
            v = modf(v, &intPart);

            if (u < 0.0) { u += 1.0; }
            if (v < 0.0) { v += 1.0; }
            break;
    }

    int x = MIN((int)(u * (double)width), width - 1);
    int y = MIN((int)(v * (double)height), height - 1);

    return data[x + (y * width)] != 0;
}

Screen::Screen(int reqWidth, int reqHeight) : width(reqWidth), height(reqHeight) {
    this->pixBuf = (unsigned char *)malloc(width * height * sizeof(pixBuf[0]));
    this->zBuf = (double *)malloc(width * height * sizeof(zBuf[0]));
}

Screen::~Screen() {
    free(this->pixBuf);
    free(this->zBuf);
}

void Screen::clear() {
    memset(pixBuf, 0, width * height);

    for (int i = 0; i < width * height; i++) {
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
    // This only makes sense if we are the right size. Otherwise we may be used for textures.
    if (width != SIGN_WIDTH || height != SIGN_HEIGHT) { return; }

    FILE *fp = fopen("/sign/frame.bin", "wb");
    if (fp != NULL) {
        (void)!fwrite(pixBuf, 1, SIGN_WIDTH * SIGN_HEIGHT, fp);
        fclose(fp);
    }
}

Texture *Screen::renderTexture() {
    return new Texture(width, height, pixBuf);
}

bool Screen::_getPixel(int x, int y) {
    return pixBuf[x + (y * width)] != 0;
}

void Screen::drawPixel(int x, int y, double z, bool on) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // Z is technically 1/W, where W is -Z, so invert it.
    if (z != 0.0) {
        z = -1 / z;
    }

    if (z < 0.0) {
        return;
    }
    if (z > zBuf[x + (y * width)]) {
        return;
    }

    pixBuf[x + (y * width)] = on ? 1 : 0;
    zBuf[x + (y * width)] = z;
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

void Screen::drawPolygon(Point *points[], int length, bool on) {
    // Don't draw this if it isn't at least a 3-poly.
    if (length < 3) { return; }
    if (length == 3) {
        drawTri(points[0], points[1], points[2], on);
        return;
    }
    if (length == 4) {
        drawQuad(points[0], points[1], points[2], points[3], on);
        return;
    }

    // Draw each individual line outlining the polygon.
    for (int i = 0; i < length - 1; i++) {
        drawLine(points[i], points[i + 1], on);
    }

    drawLine(points[length - 1], points[0], on);
}

void Screen::drawTexturedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex) {
    // Calculate the bounds.
    int minX = (int)MIN(MIN(first->x, second->x), third->x);
    int minY = (int)MIN(MIN(first->y, second->y), third->y);
    int maxX = (int)MAX(MAX(first->x, second->x), third->x);
    int maxY = (int)MAX(MAX(first->y, second->y), third->y);

    if (minX >= width || maxX < 0) { return; }
    if (minY >= height || maxY < 0) { return; }

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

    // Heuristic/hack to support affine tranformation rendering using the same function.
    double firstW = first->z;
    double secondW = second->z;
    double thirdW = third->z;
    bool isAffine = false;
    if (firstW == 0.0 && secondW == 0.0 && thirdW == 0.0)
    {
        firstW = 1.0;
        secondW = 1.0;
        thirdW = 1.0;
        isAffine = true;
    }

    Matrix *uvwMatrix = new Matrix();
    uvwMatrix->a11 = (secondTex->u * secondW) - (firstTex->u * firstW);
    uvwMatrix->a12 = (secondTex->v * secondW) - (firstTex->v * firstW);
    uvwMatrix->a13 = secondW - firstW;
    uvwMatrix->a21 = (thirdTex->u * thirdW) - (firstTex->u * firstW);
    uvwMatrix->a22 = (thirdTex->v * thirdW) - (firstTex->v * firstW);
    uvwMatrix->a23 = thirdW - firstW;
    uvwMatrix->a41 = firstTex->u * firstW;
    uvwMatrix->a42 = firstTex->v * firstW;
    uvwMatrix->a43 = firstW;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            Point *curPoint = new Point(x + 0.5, y + 0.5, 0.0);
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

            // Figure out the 1/W UV coordinates for this pixel.
            Point *uvInvPoint = uvwMatrix->multiplyPoint(triPoint);
            double u = uvInvPoint->x / uvInvPoint->z;
            double v = uvInvPoint->y / uvInvPoint->z;

            drawPixel(x, y, isAffine ? 0.0 : uvInvPoint->z, tex->valueAt(u, v));
            delete uvInvPoint;
            delete triPoint;
            delete curPoint;
        }
    }

    delete uvwMatrix;
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

void Screen::drawTexturedPolygon(Point *points[], UV *uv[], int length, Texture *tex) {
    if (length < 3) { return; }
    if (length == 3) {
        drawTexturedTri(points[0], points[1], points[2], uv[0], uv[1], uv[2], tex);
        return;
    }
    if (length == 4) {
        drawTexturedQuad(points[0], points[1], points[2], points[3], uv[0], uv[1], uv[2], uv[3], tex);
        return;
    }

    // Draw the textured polygon. in length-2 triangles.
    for (int i = 0; i < length - 2; i++) {
        drawTexturedTri(points[i], points[i + 1], points[length - 1], uv[i], uv[i + 1], uv[length - 1], tex);
    }
}

void Screen::_drawOccludedTri(Point *first, Point *second, Point *third, Screen *screen) {
    // Calculate the bounds.
    int minX = (int)MIN(MIN(first->x, second->x), third->x);
    int minY = (int)MIN(MIN(first->y, second->y), third->y);
    int maxX = (int)MAX(MAX(first->x, second->x), third->x);
    int maxY = (int)MAX(MAX(first->y, second->y), third->y);

    if (minX >= width || maxX < 0) { return; }
    if (minY >= height || maxY < 0) { return; }

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
            Point *curPoint = new Point(x + 0.5, y + 0.5, 0.0);
            Point *triPoint = xyMatrix->multiplyPoint(curPoint);

            // Cheeky hack to make sure we always draw the bounding box itself.
            // We know where it should be, so rounding errors where the inverse
            // matrix falls slightly outside of the box can be avoided if we just
            // assume every "lit" pixel in the texture "mask" is within bounds.
            bool isSet = screen->_getPixel(x, y);
            if (!isSet) {
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
            }

            // Figure out the 1/W coordinate for this pixel.
            Point *actualPoint = xywMatrix->multiplyPoint(triPoint);
            drawPixel(x, y, actualPoint->z, isSet);
            delete actualPoint;
            delete triPoint;
            delete curPoint;
        }
    }

    delete xywMatrix;
    delete xyMatrix;
}

bool Screen::_isBackFacing(Point *first, Point *second, Point *third) {
    // We are a CW system, not a CCW system, so the first vector is first->third.
    double ax = third->x - first->x;
    double ay = third->y - first->y;

    double bx = second->x - first->x;
    double by = second->y - first->y;

    // We just need the Z axis from the cross product.
    return ((ax * by) - (ay * bx)) > 0.0;
}

void Screen::drawOccludedTri(Point *first, Point *second, Point *third) {
    // Don't draw this if it is back-facing.
    if (_isBackFacing(first, second, third)) { return; }

    // First, we draw the border, so that we have the "texture" to pull from when we want to outline the triangle.
    Screen *tmpScreen = new Screen(width, height);
    tmpScreen->clear();
    tmpScreen->drawTri(first, second, third, true);

    // Now, draw the "texture".
    _drawOccludedTri(first, second, third, tmpScreen);
    delete tmpScreen;
}

void Screen::drawOccludedQuad(Point *first, Point *second, Point *third, Point *fourth) {
    // Don't draw this if it is back-facing.
    if (_isBackFacing(first, second, fourth)) { return; }

    // First, we draw the border, so that we have the "texture" to pull from when we want to outline the quad.
    Screen *tmpScreen = new Screen(width, height);
    tmpScreen->clear();
    tmpScreen->drawQuad(first, second, third, fourth, true);

    // Now, draw the "texture" in two quads.
    _drawOccludedTri(first, second, fourth, tmpScreen);
    _drawOccludedTri(second, third, fourth, tmpScreen);
    delete tmpScreen;
}

void Screen::drawOccludedPolygon(Point *points[], int length) {
    // Don't draw this if it isn't at least a 3-poly.
    if (length < 3) { return; }
    if (length == 3) {
        drawOccludedTri(points[0], points[1], points[2]);
        return;
    }
    if (length == 4) {
        drawOccludedQuad(points[0], points[1], points[2], points[3]);
        return;
    }

    // Don't draw this if it is back-facing.
    if (_isBackFacing(points[0], points[1], points[length - 1])) { return; }

    // First, we draw the border, so that we have the "texture" to pull from when we want to outline the shape.
    Screen *tmpScreen = new Screen(width, height);
    tmpScreen->clear();

    for (int i = 0; i < length - 1; i++) {
        tmpScreen->drawLine(points[i], points[i + 1], true);
    }
    tmpScreen->drawLine(points[length - 1], points[0], true);

    // Now, draw the "texture" in length-2 triangles.
    for (int i = 0; i < length - 2; i++) {
        _drawOccludedTri(points[i], points[i + 1], points[length - 1], tmpScreen);
    }

    delete tmpScreen;
}

void Screen::drawTexturedOccludedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex) {
    // Don't draw this if it is back-facing.
    if (_isBackFacing(first, second, third)) { return; }

    // Now, draw the texture to the screen.
    drawTexturedTri(first, second, third, firstTex, secondTex, thirdTex, tex);
}

void Screen::drawTexturedOccludedQuad(
    Point *first, Point *second, Point *third, Point *fourth,
    UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
    Texture *tex
) {
    // Don't draw this if it is back-facing.
    if (_isBackFacing(first, second, fourth)) { return; }

    // Now, draw the texture to the screen.
    drawTexturedQuad(first, second, third, fourth, firstTex, secondTex, thirdTex, fourthTex, tex);
}

void Screen::drawTexturedOccludedPolygon(Point *points[], UV *uv[], int length, Texture *tex) {
    // Don't draw this if it isn't at least a 3-poly.
    if (length < 3) { return; }
    if (length == 3) {
        drawTexturedOccludedTri(points[0], points[1], points[2], uv[0], uv[1], uv[2], tex);
        return;
    }
    if (length == 4) {
        drawTexturedOccludedQuad(points[0], points[1], points[2], points[3], uv[0], uv[1], uv[2], uv[3], tex);
        return;
    }

    // Don't draw this if it is back-facing.
    if (_isBackFacing(points[0], points[1], points[length - 1])) { return; }

    // Now, draw the texture to the screen.
    drawTexturedPolygon(points, uv, length, tex);
}
