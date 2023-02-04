#ifndef RASTER_H
#define RASTER_H

#include "matrix.h"

#define SIGN_WIDTH 128
#define SIGN_HEIGHT 64

#define CLAMP_MODE_NORMAL 0
#define CLAMP_MODE_MIRROR 1
#define CLAMP_MODE_TILE 2

class UV {
    public:
        UV(double u, double v);

        double u;
        double v;
};

class Texture {
    public:
        Texture(int width, int height, unsigned char data[]);
        Texture(const char * const filename);
        ~Texture();

        void setClampMode(int mode);

        bool valueAt(double u, double v);

    private:
        int width;
        int height;
        int managed;
        int mode;
        unsigned char *data;
};

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
        void drawPolygon(Point *points[], int length, bool on);

        void drawTexturedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex);
        void drawTexturedQuad(
            Point *first, Point *second, Point *third, Point *fourth,
            UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
            Texture *tex
        );
        void drawTexturedPolygon(Point *points[], UV *uv[], int length, Texture *tex);

        void drawOccludedTri(Point *first, Point *second, Point *third);
        void drawOccludedQuad(Point *first, Point *second, Point *third, Point *fourth);
        void drawOccludedPolygon(Point *points[], int length);

        void drawTexturedOccludedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex);
        void drawTexturedOccludedQuad(
            Point *first, Point *second, Point *third, Point *fourth,
            UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
            Texture *tex
        );
        void drawTexturedOccludedPolygon(Point *points[], UV *uv[], int length, Texture *tex);


    private:
        bool _getPixel(int x, int y);
        bool _isBackFacing(Point *first, Point *second, Point *third);
        void _drawOccludedTri(Point *first, Point *second, Point *third, Screen *outline);

        unsigned char pixBuf[SIGN_WIDTH * SIGN_HEIGHT];
        double zBuf[SIGN_WIDTH * SIGN_HEIGHT];
};

#endif
