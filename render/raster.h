#ifndef RASTER_H
#define RASTER_H

#include "matrix.h"

#define SIGN_WIDTH 128
#define SIGN_HEIGHT 64

class Screen {
    public:
        void clear();
        void waitForVBlank();
        void renderFrame();

        void drawPixel(int x, int y, double z, bool on);
        void drawLine(int x0, int y0, double z0, int x1, int y1, double z1, bool on);
        void drawLine(Point *first, Point *second, bool on);
        void drawTri(Point *first, Point *second, Point *third, bool on);
        void drawQuad(Point *first, Point *second, Point *third, Point *fourth, bool on);

    private:
        unsigned char pixBuf[SIGN_WIDTH * SIGN_HEIGHT];
        double zBuf[SIGN_WIDTH * SIGN_HEIGHT];
};

#endif
