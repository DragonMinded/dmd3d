#include <cstdio>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "matrix.h"
#include "raster.h"

int main (int argc, char *argv[]) {
    printf("Running rectangle tests...\n");

    Screen *screen = new Screen();
    int count = 0;

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up a square and then rotate it in place.
        Point *coords[4] = {
            new Point(42, 10, 0),
            new Point(86, 10, 0),
            new Point(86, 54, 0),
            new Point(42, 54, 0),
        };

        Matrix *rotMatrix = new Matrix();
        Point *origin = new Point(64, 32, 0);
        rotMatrix->rotateOriginZ(origin, count * 3);
        rotMatrix->multiplyPoints(coords, sizeof(coords) / sizeof(coords[0]));

        // Draw the quad to the screen itself.
        screen->drawQuad(coords[0], coords[1], coords[2], coords[3]);

        // Write out the render to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
            delete coords[i];
        }

        // Keep track of location.
        count++;
    }

    delete screen;
    printf("Done!\n");

    return 0;
}
