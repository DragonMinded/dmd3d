#include <cstdio>
#include <cmath>
#include <cstring>
#include "matrix.h"
#include "raster.h"
#include "common.h"

int main (int argc, char *argv[]) {
    printf("Running textured cube tests...\n");

    Screen *screen = new Screen(SIGN_WIDTH, SIGN_HEIGHT);
    int count = 0;

    // Load the textures.
    Texture *testTex = new Texture("testtex.png");
    Texture *suite = new Texture("suite.png");

    // Set up a second texture that tiles.
    Texture *testTex2 = testTex->clone();
    testTex2->setClampMode(CLAMP_MODE_TILE);

    // Set up a third texture that mirrors.
    Texture *suite2 = suite->clone();
    suite2->setClampMode(CLAMP_MODE_MIRROR);

    while ( 1 ) {
        // Set up our pixel buffer.
        screen->clear();

        // Set up the view matrix.
        Matrix *viewMatrix = new Matrix(SIGN_WIDTH, SIGN_HEIGHT, 90.0, 1.0, 1000.0);

        // Set up our cube.
        Point *cubeCoords[] = {
            new Point(-1.0,  1.0, -1.0),
            new Point( 1.0,  1.0, -1.0),
            new Point( 1.0, -1.0, -1.0),
            new Point(-1.0, -1.0, -1.0),
            new Point(-1.0,  1.0,  1.0),
            new Point( 1.0,  1.0,  1.0),
            new Point( 1.0, -1.0,  1.0),
            new Point(-1.0, -1.0,  1.0),
        };
        UV *uvCoords[] = {
            // All textured cubes have the same UV coords.
            new UV(0, 0),
            new UV(1, 0),
            new UV(1, 1),
            new UV(0, 1),

            // Except the one that doesn't ;)
            new UV(0, 0),
            new UV(2, 0),
            new UV(2, 2),
            new UV(0, 2),
        };

        // Manipulate location of object in world.
        Matrix *effectsMatrix = new Matrix();

        // Move it back into the screen so it's visible.
        effectsMatrix->translateZ(2.5);

        // Rotate it about its origin randomly.
        effectsMatrix->rotateX((count * 0.2));
        effectsMatrix->rotateY((count * 2.5));
        effectsMatrix->rotateX(45);

        // Throb it by scaling the cube by a sinusoidal.
        double val = (0.55 + (sin((count / 30.0) * M_PI) / 15.0));
        effectsMatrix->scale(val, val, val);

        // Transform the full cube based on our effects above (in reverse order).
        effectsMatrix->multiplyPoints(cubeCoords, sizeof(cubeCoords) / sizeof(cubeCoords[0]));
        delete effectsMatrix;

        // Move the cube to where it should go.
        viewMatrix->projectPoints(cubeCoords, sizeof(cubeCoords) / sizeof(cubeCoords[0]));

        // Draw the cube four sides are textured and the other two are simple wireframe occlusion.
        // Demonstrate that we can intermix between these modes seamlessly.
        screen->drawTexturedOccludedQuad(
            cubeCoords[0], cubeCoords[1], cubeCoords[2], cubeCoords[3],
            uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3],
            testTex
        );
        screen->drawTexturedOccludedQuad(
            cubeCoords[5], cubeCoords[4], cubeCoords[7], cubeCoords[6],
            uvCoords[4], uvCoords[5], uvCoords[6], uvCoords[7],
            testTex2
        );
        screen->drawTexturedOccludedQuad(
            cubeCoords[0], cubeCoords[4], cubeCoords[5], cubeCoords[1],
            uvCoords[0], uvCoords[1], uvCoords[2], uvCoords[3],
            suite
        );
        screen->drawOccludedQuad(
            cubeCoords[1], cubeCoords[5], cubeCoords[6], cubeCoords[2]
        );
        screen->drawTexturedOccludedQuad(
            cubeCoords[2], cubeCoords[6], cubeCoords[7], cubeCoords[3],
            uvCoords[4], uvCoords[5], uvCoords[6], uvCoords[7],
            suite2
        );
        screen->drawOccludedQuad(
            cubeCoords[0], cubeCoords[3], cubeCoords[7], cubeCoords[4]
        );

        // Render it to the screen.
        screen->waitForVBlank();
        screen->renderFrame();

        // Clean up.
        delete viewMatrix;
        for (int i = 0; i < sizeof(cubeCoords) / sizeof(cubeCoords[0]); i++) {
            delete cubeCoords[i];
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
