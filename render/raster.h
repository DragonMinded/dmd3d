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

        void drawPixel(int x, int y);
        void drawLine(int x0, int y0, int x1, int y1);
        void drawLine(Point *first, Point *second);
        void drawQuad(Point *first, Point *second, Point *third, Point *fourth);

    private:
        unsigned char pixBuf[SIGN_WIDTH * SIGN_HEIGHT];
};

#endif
