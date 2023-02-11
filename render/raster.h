#ifndef RASTER_H
#define RASTER_H

#include "matrix.h"

#define CLAMP_MODE_NORMAL 0
#define CLAMP_MODE_MIRROR 1
#define CLAMP_MODE_TILE 2

#define NORMAL_ORDER_CW 0
#define NORMAL_ORDER_CCW 1

class UV {
    public:
        UV(double u, double v);

        const double u;
        const double v;
};

class Texture {
    public:
        Texture(int width, int height, unsigned char data[]);
        Texture(const char * const filename);
        ~Texture();

        Texture *clone();

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
        Screen(int width, int height);
        ~Screen();
        
        // Sets the normal order for backface culling. Defaults to counter-clockwise (CCW) which matches
        // the normal order for STL triangles.
        void setNormalOrder(int normalOrder);

        // Wipe the screen and the Z-buffer, setting all pixels to unlit and the Z-depth for each pixel to infinity.
        void clear();

        // Wait until the physical screen attached to this device has gone into vblank, where it is safe to draw
        // the next frame.
        void waitForVBlank();

        // Render the pixels represented by this screen to the physical screen attached to this device.
        void renderFrame();

        // Returns a texture representation of this screen, useful for rendering this screen onto a polygon
        // in another scene.
        Texture *renderTexture();

        // Draw a pixel to x,y coordinate on this screen, with final Z-depth specified and a boolean
        // for whether the pixel should be drawn lit or unlit. Respects Z-depth, so pixels drawn at
        // the same location but further back than an existing pixel will be skipped. Note that the
        // Z-depth is represented as W here, which is 1/Z.
        void drawPixel(int x, int y, double w, bool on);

        // Draw a line from x0,y0 coordinate on this screen, to x1,y1 coordinate on this screen. Respects
        // the Z-depth at each interpolated point on the line, so that lines drawn have correct Z-buffering.
        // Note that the Z-depth is represented as W here, which is 1/Z.
        void drawLine(int x0, int y0, double w0, int x1, int y1, double w1, bool on);

        // Draw a line between the first and second point, whose x and y coordinates represent screen coordinates
        // and whose z coordinate represents W which is 1/Z.
        void drawLine(Point *first, Point *second, bool on);

        // Draw triangles, quads and arbitrary convex polygons with no edge intersection and respecting the Z-depth
        // as represented on the points by W which is 1/Z.
        void drawTri(Point *first, Point *second, Point *third, bool on);
        void drawQuad(Point *first, Point *second, Point *third, Point *fourth, bool on);
        void drawPolygon(Point *points[], int length, bool on);

        // Identical to the above, but instead of drawing see-through wireframe, this "fills in" the center of the
        // polygons made by the wireframe edges with unlit pixels so that the wireframe polygons behind this one are
        // occluded. Similar to drawing with a texture of all unlit pixels except for this guarantees a one pixel
        // highlighted border around the polygon. Note that if this polygon is facing away from the camera it will
        // not be drawn.
        void drawOccludedTri(Point *first, Point *second, Point *third);
        void drawOccludedQuad(Point *first, Point *second, Point *third, Point *fourth);
        void drawOccludedPolygon(Point *points[], int length);

        // Overloaded versions of the above, which allow you to control whether a particular edge is highlighted or not.
        // Useful for frustum culling occluded polygons where some edges should no longer be highlighted. Note that the
        // drawn/not drawn flag for a given edge is attached to the beginning point for that edge.
        void drawOccludedTri(Point *first, Point *second, Point *third, bool drawFirst, bool drawSecond, bool drawThird);
        void drawOccludedQuad(Point *first, Point *second, Point *third, Point *fourth, bool drawFirst, bool drawSecond, bool drawThird, bool drawFourth);
        void drawOccludedPolygon(Point *points[], bool draws[], int length);

        // Draw a textured triangle, quad or arbitrary convex polygon with no edge intersection, using a supplied texture
        // and UV coordinates for the various points of the polygons, and respecting the Z-depth as represented on the points
        // by W which is 1/Z.
        void drawTexturedTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex);
        void drawTexturedQuad(
            Point *first, Point *second, Point *third, Point *fourth,
            UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
            Texture *tex
        );
        void drawTexturedPolygon(Point *points[], UV *uv[], int length, Texture *tex);

        // Identical to the above textured drawing functions with backface culling. That means that if the polygon faces away
        // from the camera it will not be drawn at all. So, if your camera is inside a polygon it will never be rendered.
        void drawTexturedCulledTri(Point *first, Point *second, Point *third, UV *firstTex, UV *secondTex, UV *thirdTex, Texture *tex);
        void drawTexturedCulledQuad(
            Point *first, Point *second, Point *third, Point *fourth,
            UV *firstTex, UV *secondTex, UV *thirdTex, UV *fourthTex,
            Texture *tex
        );
        void drawTexturedCulledPolygon(Point *points[], UV *uv[], int length, Texture *tex);

        // The width and height of this screen in pixels.
        const int width;
        const int height;
    private:
        Screen *_getMaskScreen();
        bool _getPixel(int x, int y);
        bool _isBackFacing(Point *first, Point *second, Point *third);
        void _drawOccludedTri(Point *first, Point *second, Point *third, Screen *outline);

        int normalOrder;
        unsigned char *pixBuf;
        double *zBuf;
        Screen *maskScreen;
};

#endif
