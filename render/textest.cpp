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

    // Load the texture.
    Texture *testTex = new Texture("testtex.png");

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up a textured square and then rotate it in place.
        Point *coords[] = {
            new Point(48, 16, 0),
            new Point(80, 16, 0),
            new Point(80, 48, 0),
            new Point(48, 48, 0),
        };
        UV *uvCoords[] = {
            new UV(0, 0),
            new UV(1, 0),
            new UV(1, 1),
            new UV(0, 1),
        };

        Matrix *rotMatrix = new Matrix();
        Point *origin = new Point(64, 32, 0);
        rotMatrix->rotateOriginZ(origin, count * -2.0);
        rotMatrix->multiplyPoints(coords, sizeof(coords) / sizeof(coords[0]));

        // Draw the quad to the screen itself.
        screen->drawTexturedQuad(coords[0], coords[1], coords[2], coords[3], uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3], testTex);

        // Write out the render to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        for (int i = 0; i < sizeof(coords) / sizeof(coords[0]); i++) {
            delete coords[i];
        }
        for (int i = 0; i < sizeof(uvCoords) / sizeof(uvCoords[0]); i++) {
            delete uvCoords[i];
        }

        // Keep track of location.
        count++;
    }

    delete screen;
    printf("Done!\n");

    return 0;
}
