#include <cstdio>
#include <cmath>
#include <cstring>
#include "matrix.h"
#include "raster.h"
#include "common.h"

int main (int argc, char *argv[]) {
    printf("Running view port tests...\n");

    Screen *screen = new Screen(SIGN_WIDTH, SIGN_HEIGHT);
    int count = 0;

    while ( 1 ) {
        // Set up our inner renderer.
        Screen *viewPort = new Screen(64, 64);
        viewPort->clear();

        // Set up the viewport view matrix.
        Matrix *viewportMatrix = new Matrix(64, 64, 60.0, 1.0, 1000.0);

        // Set up our gemstone.
        Point *gemCoords[] = {
            // Top part of gemstone.
            new Point(-3.3, 0,    0),
            new Point(-1.5, 3.0,  0),
            new Point(1.5,  3.0,  0),
            new Point(3.3,  0,    0),
            new Point(1.5,  -3.0, 0),
            new Point(-1.5, -3.0, 0),

            // Bottom part of gemstone.
            new Point(-1.5, -3.0, 3),
            new Point(1.5,  -3.0, 3),
            new Point(3.3,  0,    3),
            new Point(1.5,  3.0,  3),
            new Point(-1.5, 3.0,  3),
            new Point(-3.3, 0,    3),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();
        effectsMatrix->translateZ(10.0);
        effectsMatrix->rotateX(60 + (count * 1.0));
        effectsMatrix->rotateY(30 + (count * 1.1));
        effectsMatrix->rotateZ(count * 3.0);
        effectsMatrix->multiplyPoints(gemCoords, sizeof(gemCoords) / sizeof(gemCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewportMatrix->projectPoints(gemCoords, sizeof(gemCoords) / sizeof(gemCoords[0]));

        // Draw the polygons.
        viewPort->drawOccludedPolygon(&gemCoords[0], 6);
        viewPort->drawOccludedPolygon(&gemCoords[6], 6);

        // Draw the squares between each bit, which is "fun" to work out.
        viewPort->drawOccludedQuad(gemCoords[0], gemCoords[11], gemCoords[10], gemCoords[1]);
        viewPort->drawOccludedQuad(gemCoords[1], gemCoords[10], gemCoords[9], gemCoords[2]);
        viewPort->drawOccludedQuad(gemCoords[2], gemCoords[9], gemCoords[8], gemCoords[3]);
        viewPort->drawOccludedQuad(gemCoords[3], gemCoords[8], gemCoords[7], gemCoords[4]);
        viewPort->drawOccludedQuad(gemCoords[4], gemCoords[7], gemCoords[6], gemCoords[5]);
        viewPort->drawOccludedQuad(gemCoords[5], gemCoords[6], gemCoords[11], gemCoords[0]);

        // Draw a border on the screen.
        Point *boxCoords[] = {
            new Point(0, 0, 0),
            new Point(viewPort->width - 1, 0, 0),
            new Point(viewPort->width - 1, viewPort->height - 1, 0),
            new Point(0, viewPort->height - 1, 0),
            new Point(1, 1, 0),
            new Point(viewPort->width - 2, 1, 0),
            new Point(viewPort->width - 2, viewPort->height - 2, 0),
            new Point(1, viewPort->height - 2, 0),
        };
        viewPort->drawQuad(boxCoords[0], boxCoords[1], boxCoords[2], boxCoords[3], true);
        viewPort->drawQuad(boxCoords[4], boxCoords[5], boxCoords[6], boxCoords[7], true);

        // Grab a texture of this.
        Texture *viewPortTexture = viewPort->renderTexture();
        delete viewPort;
        delete viewportMatrix;
        for (int i = 0; i < sizeof(gemCoords) / sizeof(gemCoords[0]); i++) {
            delete gemCoords[i];
        }
        for (int i = 0; i < sizeof(boxCoords) / sizeof(boxCoords[0]); i++) {
            delete boxCoords[i];
        }

        // Now, set up the screen itself, render this as a polygon to the screen for funsies.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);
        screen->clear();

        // Set up a 3D textured quad and then rotate it all over.
        float size = 3.5;
        Point *a3dCoords[] = {
            new Point(-size,  size, 0),
            new Point( size,  size, 0),
            new Point( size, -size, 0),
            new Point(-size, -size, 0),
        };
        UV *uvCoords[] = {
            new UV(0, 0),
            new UV(1, 0),
            new UV(1, 1),
            new UV(0, 1),
        };

        Matrix *rotMatrix = new Matrix();
        rotMatrix->rotateY(sin(count / 80.0) * 20);
        rotMatrix->translateZ(5.0);
        rotMatrix->multiplyPoints(a3dCoords, sizeof(a3dCoords) / sizeof(a3dCoords[0]));
        delete rotMatrix;

        // Move the texture to where it should go.
        viewMatrix->projectPoints(a3dCoords, sizeof(a3dCoords) / sizeof(a3dCoords[0]));

        // Draw the cube, now.
        screen->drawTexturedQuad(
            a3dCoords[0], a3dCoords[1], a3dCoords[2], a3dCoords[3],
            uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3],
            viewPortTexture
        );

        // Render it to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        delete viewPortTexture;
        delete viewMatrix;
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
