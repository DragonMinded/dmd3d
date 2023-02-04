#include <cstdio>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "matrix.h"
#include "raster.h"
#include "common.h"

int main (int argc, char *argv[]) {
    printf("Running texture tests...\n");

    Screen *screen = new Screen(SIGN_WIDTH, SIGN_HEIGHT);
    int count = 0;

    // Load the texture.
    Texture *testTex = new Texture("testtex.png");

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up a textured square and then rotate it in place.
        Point *a2dCoords[] = {
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
        rotMatrix->translateX(-32);
        rotMatrix->rotateOriginZ(origin, count * -2.0);
        rotMatrix->multiplyPoints(a2dCoords, sizeof(a2dCoords) / sizeof(a2dCoords[0]));
        delete rotMatrix;
        delete origin;

        // Draw the quad to the screen itself.
        screen->drawTexturedQuad(a2dCoords[0], a2dCoords[1], a2dCoords[2], a2dCoords[3], uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3], testTex);

        // Now, set up the view matrix for the 3D one.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Set up a 3D textured quad and then rotate it all over.
        Point *a3dCoords[] = {
            new Point(-1.5,  1.5, 0),
            new Point( 1.5,  1.5, 0),
            new Point( 1.5, -1.5, 0),
            new Point(-1.5, -1.5, 0),
        };

        rotMatrix = new Matrix();
        rotMatrix->translateZ(4.0);
        rotMatrix->translateX(2.0);
        rotMatrix->rotateY(count * 2.75);
        rotMatrix->rotateX(count * 1.25);
        rotMatrix->multiplyPoints(a3dCoords, sizeof(a3dCoords) / sizeof(a3dCoords[0]));

        // Move the texture to where it should go.
        viewMatrix->projectPoints(a3dCoords, sizeof(a3dCoords) / sizeof(a3dCoords[0]));

        // Draw the cube, now.
        screen->drawTexturedQuad(a3dCoords[0], a3dCoords[1], a3dCoords[2], a3dCoords[3], uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3], testTex);

        // Write out the render to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        for (int i = 0; i < sizeof(a2dCoords) / sizeof(a2dCoords[0]); i++) {
            delete a2dCoords[i];
        }
        for (int i = 0; i < sizeof(a3dCoords) / sizeof(a3dCoords[0]); i++) {
            delete a3dCoords[i];
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
